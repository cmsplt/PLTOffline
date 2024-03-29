#!/usr/bin/env python3

'''
read HV bias scan logs into a pandas dataframe, merge with HF or BCM1f data from brilcalc,
group together and cleanup scan data (i.e. drop non-uniform values of per-channel-iMon and per-channel-rates),
and calculate depletion voltage (plateau of rate vs HV curve) and plot as a function of time/intLumi
'''

import functools
import io
import pathlib
import socket
import subprocess
import timeit
import typing

import matplotlib.pyplot
import numpy
import pandas
import tables

if socket.gethostname() == 'scx5-c2f06-36':
    PATH = pathlib.Path('/localdata/SSHFS.BRILDATA')

def timer(func: typing.Callable):
    '''Print the processing time for a function.'''
    # [https://realpython.com/primer-on-python-decorators/]
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        t0 = timeit.default_timer()
        value = func(*args, **kwargs)
        t1 = timeit.default_timer()
        print(f"\t{func.__name__}(): {t1 - t0}s")
        return value
    return wrapper

@timer
def parseLogFile(log_file: pathlib.Path) -> pandas.DataFrame:
    '''Parse automatic scan log files into a `pandas.DataFrame`.'''
    log = pandas.read_csv(log_file, sep='\n', header=None).squeeze()
    meta_idx = log[log.str.contains('#channels')].index
    meta = pandas.Series(index=log[meta_idx].squeeze().split(',')[1:], data=log[meta_idx+1].squeeze().split(',')[1:])
    nCh = int(meta.pop('#channels'))
    fields = meta[meta.astype(int).astype(bool)].index
    active_ch = log[log.str.contains('H[pm][FN]T[0-3]')].iloc[0].split(',')[1:]
    assert nCh == len(active_ch) 
    channel_map = dict(zip([f'H{q}T{ch}' for q in ['mN','mF','pN','pF'] for ch in [0,2,1,3]], range(16))) # https://twiki.cern.ch/twiki/bin/viewauth/CMS/PLT#PLT_Channel_Map
    ch = pandas.Series(channel_map.get(ch) for ch in active_ch).astype(str)
    program_cols = ['timestamp', 'step'] + pandas.concat([field+'_'+ch for field in fields[:-1]]).to_list() + list('rate_'+ch) + ['rate_avg']
    program = pandas.DataFrame(data=log[log.str.contains('#[0-9]+')].str.split(',').to_list(), columns=program_cols)
    program['step'] = program.step.str[1:]
    program[program.columns[program.columns != 'timestamp']] = program[program.columns[program.columns != 'timestamp']].apply(pandas.to_numeric, errors='coerce')
    program['timestamp'] = pandas.to_datetime(program.timestamp).dt.tz_localize('Europe/Amsterdam').dt.tz_convert('UTC')
    data_header = ['timestamp', 'ignore'] + list(fields[0]+'_'+ch) + list(fields[1]+'_'+ch) + list('rate_'+ch) + ['rate_avg']
    data = pandas.DataFrame(data=log[log.str.contains('#M')].str.split(',').to_list(), columns=data_header)
    data = data.apply(pandas.to_numeric, errors='ignore')
    data['timestamp'] = pandas.to_datetime(data.timestamp).dt.tz_localize('Europe/Amsterdam').dt.tz_convert('UTC')
    return (program, data)

@timer
def brilcalc(luminometer: str, begin: pandas.Timestamp, end: pandas.Timestamp) -> pandas.DataFrame:
    '''load brilcalc hfet data for a given fill and load into a `pandas.DataFrame`.''' # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalc]
    cmd = ['brilcalc', 'lumi', '--type', luminometer, '--begin', begin.strftime('%m/%d/%y %H:%M:%S'), '--end', end.strftime('%m/%d/%y %H:%M:%S'), '--byls', '--tssec', '--output-style', 'csv']
    data = subprocess.run(cmd, capture_output=True).stdout.decode()
    data = pandas.read_csv(io.StringIO(data), header=None, names=data.splitlines()[1].split(','), comment='#')
    data[['run', 'fill']] = data['#run:fill'].str.split(':', expand=True)
    data[['ls', 'cms_ls']] = data['ls'].str.split(':', expand=True)
    data = data.apply(pandas.to_numeric, errors='ignore')
    data['datetime'] = pandas.to_datetime(data.time, unit='s', utc=True)
    return data

@timer
def hd5(filepath: pathlib.Path, begin: pandas.Timestamp, end: pandas.Timestamp, node: str = '/pltaggzero') -> pandas.DataFrame:
    condition = f'(timestampsec>={begin.timestamp()}) & (timestampsec<={end.timestamp()})'
    with tables.open_file(filepath, 'r') as table:
        return pandas.DataFrame(data=[row[:] for row in table.get_node(node).where(condition)], columns=table.get_node(node).colnames)

def pileup(bx_data: pandas.Series, k: float = 11245/300) -> numpy.ndarray:
    '''Calculates pileup based on `k`. Note that `bx_data` values correspond to a bunch crossing with zero hits: 4*nibble == 4*(2**12) == 16384'''
    bx_data = numpy.stack(bx_data) # [how to convert a Series of arrays into a single matrix in pandas/numpy?](https://stackoverflow.com/a/48793939/13019084)
    bx_data[bx_data > 2**14] = 2**14 # set values > 2**14 to zero hits (this usually only happens at the beginning of a file)
    return k * -numpy.log(bx_data/2**14)

def merge(log_file: pathlib.Path):
    program, scan_data = parseLogFile(log_file)
    begin, end = program.timestamp.iloc[0], program.timestamp.iloc[-1]
    brilcalc_data = brilcalc(luminometer='hfet', begin=begin, end=end)
    fill = brilcalc_data.fill.unique()[0]
    runs = brilcalc_data.run.unique()
    hd5_files = [[*pathlib.Path(f"{PATH}/{program.timestamp.iloc[0].strftime('%y')}/{fill}").glob(f'{fill}_{run}*')][0] for run in runs]
    data = pandas.concat(hd5(filepath=filepath, begin=begin, end=end, node='/pltaggzero') for filepath in hd5_files)
    data['datetime'] = pandas.to_datetime(data.timestampsec*1000 + data.timestampmsec, unit='ms', utc=True)
    data['pileup'] = pileup(bx_data=data.data).sum(axis=1)
    data = pandas.merge_asof(left=data, right=brilcalc_data, left_on='datetime', right_on='datetime')
    data = pandas.merge_asof(left=data, right=scan_data, left_on='datetime', right_on='timestamp')
    return program, data.dropna()

def plot_raw(data: pandas.DataFrame):
    matplotlib.pyplot.scatter(x=data[data.channelid==10].datetime, y=data[data.channelid==10].pileup)
    # label HV setpoints
    matplotlib.pyplot.show()

def vmonVsPileup(v_mon: pandas.Series, pileup: pandas.Series, v_set: pandas.Series):
    v_bins = pandas.concat([v_set - 2, v_set + 2]).sort_values().drop_duplicates().reset_index(drop=True)
    v_mon_vs_pileup  = pileup.groupby(pandas.cut(x=v_mon, bins=v_bins)).median().dropna() # [Pandas Groupby Range of Values](https://stackoverflow.com/a/21441621)
    return pandas.Series({x.mid: v_mon_vs_pileup[x] for x in v_mon_vs_pileup.index if (x.length == 4)})

def main():
    log_file = pathlib.Path('AutoScanLogs/Scan_2023_6_3_11_4_41.txt')
    program, data = merge(log_file=log_file)
    for ch in range(16):
        if ch == 13:
            continue
        ch_data = data[data.channelid == ch].reset_index(drop=True)
        v_mon_vs_pileup = vmonVsPileup(v_mon=ch_data[f'v0_{ch}'], pileup=ch_data.pileup/ch_data.avgpu, v_set=program[f'v0_{ch}'])
        matplotlib.pyplot.scatter(x=v_mon_vs_pileup.index, y=v_mon_vs_pileup)
        matplotlib.pyplot.savefig(f'AutoScanLogs/2023/{log_file.stem}_ch{ch:02d}.png', dpi=400)
        matplotlib.pyplot.close()


##########################################

@timer
def mergeDF(logData: pandas.DataFrame, luminometer: str = "hfet") -> pandas.DataFrame:
    # merge logData and brilcalcDF based on closest timestamps and add new PLT/HF rate column to logData
    # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalc]
    beginTS = logData.timestamp.iloc[0].strftime("%m/%d/%y %H:%M:%S")
    endTS = logData.timestamp.iloc[-1].strftime("%m/%d/%y %H:%M:%S")
    brilcalcDF = brilcalc(beginTS, endTS, luminometer)
    if brilcalcDF.empty:
        raise f"no data from brilcalc!"
        # brilcalcDF = brilcalc(beginTS, endTS, "bcm1f")
    if (brilcalcDF.avgpu == 0).all():
        raise f"data from brilcalc is all zeros!"
    mergedDF = pandas.merge_asof(left=logData, right=brilcalcDF, left_on="timestamp", right_on="dt")
    # lol this worked? join dataframes using nearest timestamp
    # [https://www.reddit.com/r/learnpython/comments/6wbc7t/help_joining_two_pandas_dataframes/]
    # [https://towardsdatascience.com/5-lesser-known-pandas-tricks-e8ab1dd21431]
    for ch in range(16):
        logData[f"rateN{ch}"] = mergedDF[f"rate{ch}"] / mergedDF["avgpu"]
        # logData[f"rateN{ch}"] = mergedDF[f"rate{ch}"] / mergedDF["delivered(/ub)"]
    return logData.dropna()

def processChannel(logData: pandas.DataFrame, ch: int) -> pandas.DataFrame:
    # determine scanpoints for ch, filter logData for each ch and scanpoint based on uniformity of vMon and rateN values, and return median/std for rateCh & rateNCh
    vMonGroup = (logData[f"vMon{ch}"].groupby(logData[f"vMon{ch}"].round(decimals=0)).count() > 4)
    # group all integer-rounded vMon values and mark 'true' if vMon group size > 4
    scanSteps = vMonGroup[vMonGroup.values].index.to_list()
    logDataCh = logData[["timestamp", f"vMon{ch}", f"rate{ch}", f"rateN{ch}"]]
    medianRate, medianRateN, stdevRate, stdevRateN = [], [], [], []
    for i, _ in enumerate(scanSteps):
        # filter per-channel data into each HV scanstep \pm 2, select values within 5% of the last element in rateN{ch},
        # and calculate median and stdev
        logDataChStep = logDataCh[(logDataCh[f"vMon{ch}"] <= scanSteps[i] + 2) & (logDataCh[f"vMon{ch}"] >= scanSteps[i] - 2)]
        logDataChStep = logDataChStep[logDataChStep[f"rateN{ch}"].apply(lambda x: 1 - x / logDataChStep[f"rateN{ch}"].iloc[-1]).abs() < 0.05]
        medianRate.append(logDataChStep[f"rate{ch}"].median())
        stdevRate.append(logDataChStep[f"rate{ch}"].std())
        medianRateN.append(logDataChStep[f"rateN{ch}"].median())
        stdevRateN.append(logDataChStep[f"rateN{ch}"].std())
    data = list(zip(medianRate, stdevRate, medianRateN, stdevRateN))
    cols = [f"medianRate{ch}", f"stdevRate{ch}", f"medianRateN{ch}", f"stdevRateN{ch}"]
    scanDataCh = pandas.DataFrame(data, index=scanSteps, columns=cols)
    return scanDataCh


def calculateDeplVolt(scanDataCh: pandas.DataFrame, ch: int, thr1: float, thr2: float) -> int:
    # calculate depletion voltage based on percent change:
    # pick first value (highest to lowest HV setpoints) which is within 'thr1' of the previous value and within 'thr2' of next-to-previous value
    scanDataCh = scanDataCh[::-1]  # reverse order of scan points (from highest to lowest)
    def pctCh(per):
        return scanDataCh[f"medianRateN{ch}"].pct_change(periods=per).abs()
    pctP1 = pctCh(1)
    pctP2 = pctCh(2)
    deplVoltCh = scanDataCh[(pctP1 > thr1) & (pctP2 > thr2)]
    if len(deplVoltCh):
        deplVoltCh = scanDataCh[(pctP1 > thr1) & (pctP2 > thr2)].index[0]
        deplVoltStDev = scanDataCh[f"medianRateN{ch}"][:deplVoltCh].std()
    else:
        print(f"\tUnable to estimate depletion voltage for Ch{ch}")  #:\n{scanDataCh}')
        deplVoltCh = 0
    return deplVoltCh


def plotChannel(dateTime: str, ch: int, scanDataCh: pandas.DataFrame, deplVoltCh: float, dataDir: str,):
    # plot rate (raw and normalized) vs HV
    import os, matplotlib.pyplot
    scale = (scanDataCh[f"medianRateN{ch}"].iloc[-1] / scanDataCh[f"medianRate{ch}"].iloc[-1])
    fig, ax = matplotlib.pyplot.subplots()
    matplotlib.pyplot.style.use("seaborn-white")
    ax.set(title=f"Channel {ch} rate vs HV ({dateTime})", xlabel="", ylabel="")
    ax.set_ylabel("Raw inst. lumi rate", fontsize=12)
    ax.set_xlabel("HV setpoint (V)", fontsize=12)
    ax.errorbar(scanDataCh.index, scanDataCh[f"medianRateN{ch}"], yerr=scanDataCh[f"stdevRateN{ch}"], ls="", marker="o", markersize="4", label="PLT normalized",)
    ax.errorbar(scanDataCh.index, scanDataCh[f"medianRate{ch}"] * scale, yerr=scanDataCh[f"stdevRate{ch}"] * scale, ls="", marker="o", markersize="4", label="PLT",)
    ax.legend(loc="lower right", borderpad=0.1, labelspacing=0.1, fancybox=True, framealpha=0.4,)
    ax.axvline(deplVoltCh, color="red")
    fig.tight_layout()
    os.makedirs(f"{dataDir}/{dateTime[:4]}/{dateTime}/", exist_ok=True)
    fig.savefig(f"{dataDir}/{dateTime[:4]}/{dateTime}/{dateTime}.ch{ch}.png", dpi=300)
    matplotlib.pyplot.close("all")


def loadDepletionVoltageJSON(dataDir: str) -> pandas.DataFrame:
    import json
    with open(f"depletionVoltage.{dataDir}.json", "r") as deplVoltFile:
        deplVolt = json.load(deplVoltFile)
    deplVolt = pandas.DataFrame.from_dict(deplVolt).T
    deplVolt.index = [fname.split("/")[1].lstrip("Scan_").rstrip(".txt") for fname in deplVolt.index.to_list()]
    deplVolt.index = pandas.to_datetime(deplVolt.index, format="%Y_%m_%d_%H_%M_%S")
    return deplVolt


def plotAxVline(intLumi: str, label: str, position: str):
    # add vertical line at the specified intLumi (given by lumiByDay)
    import matplotlib.pyplot
    matplotlib.pyplot.axvline(x=intLumi, color="grey", linestyle="--", alpha=0.4, label=label)
    matplotlib.pyplot.text(x=intLumi + 1, y=10, s=label, rotation=90, verticalalignment=position)


def lumiByDay() -> pandas.DataFrame:
    # return dataframe Run2 cumulative integrated lumi (in 1/fb) with the date as index
    import os
    file = ("lumiByDay.csv" if os.path.exists("lumiByDay.csv") else "https://cern.ch/cmslumi/publicplots/lumiByDay.csv")
    lumiByDay = pandas.read_csv(file)
    lumiByDayRun2 = lumiByDay[lumiByDay.Date >= "2015"]  # LHC Run2
    date = pandas.to_datetime(lumiByDayRun2.Date, format="%Y-%m-%d")
    cumulativeIntLumi = (lumiByDayRun2["Delivered(/ub)"].cumsum().divide(10**9).rename("Delivered(/fb)"))
    return pandas.concat([date, cumulativeIntLumi], axis=1).set_index("Date")["Delivered(/fb)"]


def plotDeplVolt(ch: int, dataDir: str):
    # plot depletion voltage vs time for channel 'ch'
    # to do: plot deplVolt vs intLumi
    import matplotlib.pyplot
    deplVolt = loadDepletionVoltageJSON(dataDir)
    dataCh = deplVolt[ch][deplVolt[ch] != 0]  # depletion voltage dataframe (drop entries where depletion voltage == 0 )
    dataChDate = (dataCh.index.to_series().dt.strftime("%Y-%m-%d").to_list())  # to_series() required to do things to a DatetimeIndex [https://stackoverflow.com/a/49277956/13019084]
    lumiByDaySeries = lumiByDay()
    lumiPerScanDate = [lumiByDaySeries[pandas.to_datetime(scan.date())] for scan in dataCh.index.to_list()]  # Run2 cumulativeIntLumi for each scan date
    fig, intLumiAx = matplotlib.pyplot.subplots(figsize=(10, 6))
    intLumiAx.scatter(x=lumiPerScanDate, y=dataCh.to_list())
    intLumiAx.set_title(f"Ch{ch} Depletion Voltage vs Integrated Luminosity", fontsize=14)
    matplotlib.pyplot.xlabel("Integrated Luminosity (1/fb)")
    matplotlib.pyplot.ylabel("Depletion Voltage (V)")
    intLumiAx.axes.set_ylim(0.0, 800.0)
    intLumiAx.axes.set_xlim(0.0, 165.0)
    # Add secondary date axis [https://stackoverflow.com/a/33447004/13019084]
    dateAx = intLumiAx.twiny()
    dateAx.set_xlim(intLumiAx.get_xlim())  # set same range as intLumi axis
    dateAx.set_xticks(lumiPerScanDate)  # copy location of intLumi x-ticks
    dateAx.set_xticklabels(dataChDate, horizontalalignment="left")  # draw them as the date!
    dateAx.tick_params(axis="x", labelrotation=45, labelsize=8)
    [label.set_visible(False) for label in dateAx.get_xaxis().get_ticklabels()[1::2]]
    # print every second xticklabel starting with the first [https://stackoverflow.com/a/50034357/13019084]
    hvSetPoints = {"2016-09-09 16:10": "200V", "2017-08-10 01:10": "300V", "2017-10-18 21:00": "400V", "2018-03-22 17:45": "500V", "2018-06-10 04:50": "800V", "2018-08-18 04:35": "VcThr",}
    # 200:[http://cmsonline.cern.ch/cms-elog/948105]  250:[http://cmsonline.cern.ch/cms-elog/1002826] 300:[http://cmsonline.cern.ch/cms-elog/1003149]
    # 350:[http://cmsonline.cern.ch/cms-elog/1015071] 400:[http://cmsonline.cern.ch/cms-elog/1016344] 800:[http://cmsonline.cern.ch/cms-elog/1047254]
    # VcThr:[http://cmsonline.cern.ch/cms-elog/1058918]
    _ = [plotAxVline(lumiByDaySeries[pandas.to_datetime(date).date()], hv, "bottom") for date, hv in hvSetPoints.items()]
    matplotlib.pyplot.tight_layout()
    # matplotlib.pyplot.show()
    matplotlib.pyplot.savefig(f"{dataDir}/ch{ch}DepletionVoltage.png", dpi=300)
    matplotlib.pyplot.close("all")


def processScanLog(log_file: str, DataDir: str):
    dataDir = "AutoScanLogs"
    print(f"\nprocessing {log_file}...")
    deplVolt = []
    logData = parseLogFile(log_file)
    logData = mergeDF(logData, "bcm1f")
    dateTime = logData.timestamp.iloc[0].strftime("%Y%m%d.%H%M%S")
    for ch in range(16):
        scanDataCh = processChannel(logData, ch)
        deplVoltCh = calculateDeplVolt(scanDataCh, ch, 0.01, 0.02)
        deplVolt.append(deplVoltCh)
        plotChannel(dateTime, ch, scanDataCh, deplVoltCh, dataDir)
    return deplVolt

def _main():
    import json, pathlib
    dataDir = "AutoScanLogs"
    deplVolt = {}
    for log_file in sorted(pathlib.Path('AutoScanLogs/').glob('Scan_2022*.txt')):
        deplVolt[f'{log_file}'] = processScanLog(log_file, DataDir)
        # print(f"\nprocessing {log_file}...")
        # deplVolt[f'{log_file}'] = []
        # logData = parseLogFile(log_file)
        # logData = mergeDF(logData, "bcm1f")
        # dateTime = logData.timestamp.iloc[0].strftime("%Y%m%d.%H%M%S")
        # for ch in range(16):
        #     scanDataCh = processChannel(logData, ch)
        #     deplVoltCh = calculateDeplVolt(scanDataCh, ch, 0.01, 0.02)
        #     deplVolt[f'{log_file}'].append(deplVoltCh)
        #     plotChannel(dateTime, ch, scanDataCh, deplVoltCh, dataDir)
    with open(f"depletionVoltage.{dataDir}.json", "w") as deplVoltFile:
        json.dump(obj=deplVolt, fp=deplVoltFile)
    # _ = [ plotDeplVolt( ch, dataDir ) for ch in range(16) ]
    return deplVolt


if __name__ == "__main__":
    main()
