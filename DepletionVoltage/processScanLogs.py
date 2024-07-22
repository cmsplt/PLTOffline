#!/cvmfs/cms-bril.cern.ch/brilconda310/bin/python3

'''
read HV bias scan logs into a pandas dataframe, merge with HF or BCM1f data from brilcalc,
group together and cleanup scan data (i.e. drop non-uniform values of per-channel-iMon and per-channel-rates),
and plot per-channel normalized rates vs HV
'''

import functools
import pathlib
import subprocess
import timeit
import typing

import matplotlib.pyplot
import numpy
import pandas
import tables

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
    log = pandas.Series(log_file.open(mode='r').read().splitlines()) # pandas.read_csv(log_file, sep='\n', header=None).squeeze()
    meta_idx = log[log.str.contains('#channels')].index
    meta = pandas.Series(index=log[meta_idx].squeeze().split(',')[1:], data=log[meta_idx+1].squeeze().split(',')[1:])
    nCh = int(meta.pop('#channels'))
    fields = meta[meta.astype(int).astype(bool)].index
    active_ch = log[log.str.contains('H[pm][FN]T[0-3]')].iloc[0].split(',')[1:]
    assert nCh == len(active_ch) 
    channel_map = dict(zip([f'H{q}T{ch}' for q in ['mN','mF','pN','pF'] for ch in [0,2,1,3]], range(16))) # https://twiki.cern.ch/twiki/bin/viewauth/CMS/PLT#PLT_Channel_Map
    ch = pandas.Series(channel_map.get(ch) for ch in active_ch).astype(str)
    program_cols = ['timestamp', 'step'] + pandas.concat([field+'_'+ch for field in fields[:-1]]).to_list() + list('rate_'+ch) + ['rate_avg']
    if log.str.contains('NOT CORRECTLY RECEIVED').any():
        program_cols = ['timestamp', 'step'] + pandas.concat([field+'_'+ch for field in fields[:-1]]).to_list() + ['ignore', 'rate_avg']
    program = pandas.DataFrame(data=log[log.str.contains('#[0-9]+')].str.split(',').to_list(), columns=program_cols)
    program['step'] = program.step.str[1:]
    program[program.columns[program.columns != 'timestamp']] = program[program.columns[program.columns != 'timestamp']].apply(pandas.to_numeric, errors='coerce')
    program['timestamp'] = pandas.to_datetime(program.timestamp).dt.tz_localize('Europe/Amsterdam').dt.tz_convert('UTC')
    data_header = ['timestamp', 'ignore'] + list(fields[2]+'_'+ch) + list(fields[3]+'_'+ch) + list('rate_'+ch) + ['rate_avg']
    if log.str.contains('NOT CORRECTLY RECEIVED').any():
        data_header = ['timestamp', 'ignore'] + list(fields[2]+'_'+ch) + list(fields[3]+'_'+ch) + ['ignore', 'rate_avg']
    data = pandas.DataFrame(data=log[log.str.contains('#M')].str.split(',').to_list(), columns=data_header)
    data = data.apply(pandas.to_numeric, errors='ignore')
    data['timestamp'] = pandas.to_datetime(data.timestamp).dt.tz_localize('Europe/Amsterdam').dt.tz_convert('UTC')
    return (program, data)

@timer
def brilcalcFillRuns(begin: pandas.Timestamp, end: pandas.Timestamp):
    brilcalc_query = ['brilcalc', 'beam', '--begin', begin.strftime('%m/%d/%y %H:%M:%S'), '--end', end.strftime('%m/%d/%y %H:%M:%S'), '--tssec', '--output-style', 'csv']
    brilcalc_response = subprocess.run(brilcalc_query, capture_output=True).stdout.decode('utf-8').splitlines()
    brilcalc_response = pandas.DataFrame(data=[row.split(',') for row in brilcalc_response[2:]], columns=brilcalc_response[1].lstrip('#').split(',')).apply(pandas.to_numeric, errors='ignore')
    return brilcalc_response.fill.unique().item(), brilcalc_response.run.unique()

@timer
def hd5_read(filepath: pathlib.Path, begin: pandas.Timestamp, end: pandas.Timestamp, node: str = '/pltaggzero') -> pandas.DataFrame:
    condition = f'(timestampsec>={begin.timestamp()}) & (timestampsec<={end.timestamp()})'
    with tables.open_file(filepath, 'r') as table:
        return pandas.DataFrame(data=(row[:] for row in table.get_node(node).where(condition)), columns=table.get_node(node).colnames)

@timer
def lumi(bx_data: pandas.Series, k: float = 11245/300) -> numpy.ndarray:
    '''Calculates pileup based on `k`. Note that `bx_data` values correspond to a bunch crossing with zero hits: 4*nibble == 4*(2**12) == 16384'''
    bx_data = numpy.stack(bx_data) # [how to convert a Series of arrays into a single matrix in pandas/numpy?](https://stackoverflow.com/a/48793939/13019084)
    bx_data[bx_data > 2**14] = 2**14 # set values > 2**14 to zero hits (this usually only happens at the beginning of a file)
    return k * -numpy.log(bx_data/2**14)

@timer
def hd5_lumi(begin: pandas.Timestamp, end: pandas.Timestamp, fill: int, runs: list[int], node: str = '/pltaggzero', hd5_path: pathlib.Path = pathlib.Path('/localdata/sshfs/brildata/')):
    hd5_files = [next(pathlib.Path(f"{hd5_path}/{begin.strftime('%y')}/{fill}").glob(f'{fill}_{run}*')) for run in runs]
    data = pandas.concat(hd5_read(filepath=filepath, begin=begin, end=end, node=node) for filepath in hd5_files)
    data['datetime'] = pandas.to_datetime(data.timestampsec*1000 + data.timestampmsec, unit='ms', utc=True)
    if 'data' in data:
        data['rate'] = lumi(bx_data=data.data).sum(axis=1)
    if 'bx' in data:
        data['rate'] = numpy.stack(data.bx).sum(axis=1)
    return data

@timer
def merge(log_file: pathlib.Path):
    program, scan_data = parseLogFile(log_file)
    begin, end = program.timestamp.iloc[0], program.timestamp.iloc[-1]
    fill, runs = brilcalcFillRuns(begin=begin, end=end)
    plt = hd5_lumi(begin=begin, end=end, fill=fill, runs=runs, node='/pltaggzero')
    hfet = hd5_lumi(begin=begin, end=end, fill=fill, runs=runs, node='/hfetlumi')
    data = pandas.merge_asof(left=plt, right=hfet, left_on='datetime', right_on='datetime', suffixes=('_plt','_hfet'))
    data = pandas.merge_asof(left=data, right=scan_data, left_on='datetime', right_on='timestamp')
    return program, data.dropna()
    # beam = pandas.concat(hd5(filepath=filepath, begin=begin, end=end, node='/beam') for filepath in hd5_files)
    # n_colliding_bunches = numpy.unique(numpy.stack(beam.collidable).sum(axis=1))[0]

def plot_raw(data: pandas.DataFrame):
    matplotlib.pyplot.scatter(x=data[data.channelid_plt==10].datetime, y=data[data.channelid_plt==10].rate_plt/data[data.channelid_plt==10].rate_hfet)
    # label HV setpoints
    matplotlib.pyplot.show()

def vmonVsRate(v_mon: pandas.Series, rate: pandas.Series, v_set: pandas.Series, thr: int = 2):
    v_bins = pandas.IntervalIndex(sorted({pandas.Interval(left=v-thr, right=v+thr, closed='both') for v in v_set}))
    vmon_vs_rate_median  = rate.groupby(pandas.cut(x=v_mon, bins=v_bins)).median().dropna() # [Pandas Groupby Range of Values](https://stackoverflow.com/a/21441621)
    vmon_vs_rate_std  = rate.groupby(pandas.cut(x=v_mon, bins=v_bins)).std().dropna()
    vmon_vs_rate = pandas.concat([vmon_vs_rate_median, vmon_vs_rate_std], axis=1).rename(columns={0: 'median', 1: 'std'})
    vmon_vs_rate.index = vmon_vs_rate.index.remove_unused_categories().categories.mid
    return vmon_vs_rate

@timer
def plotVmonVsRate(vmon_vs_rate: pandas.Series, log_file: pathlib.Path, ch: int, v_thr: int = 2, v_relative: tuple[int, int] = (700, 800)):
    matplotlib.pyplot.errorbar(x=vmon_vs_rate.index, y=vmon_vs_rate['median'], xerr=v_thr, yerr=vmon_vs_rate['std'], fmt='o')
    relative_change = (vmon_vs_rate.loc[v_relative[1], 'median'] - vmon_vs_rate.loc[v_relative[0], 'median'])/vmon_vs_rate.loc[v_relative[0], 'median']*100
    year = log_file.stem[5:9]
    matplotlib.pyplot.title(f'PLT{ch:02}/HFET vs HV (Δ({v_relative[1]}, {v_relative[0]}) ≈ {relative_change:.2}%)')
    matplotlib.pyplot.xlabel('HV setpoint (V)')
    matplotlib.pyplot.ylabel(f'PLT ch{ch:02} rate/HFET rate')
    matplotlib.pyplot.savefig(f'AutoScanLogs/{year}/{log_file.stem}_ch{ch:02d}.png', dpi=400)
    matplotlib.pyplot.close()

def main(log_file: str):
    print(log_file)
    program, data = merge(log_file=pathlib.Path(log_file))
    for ch in range(16):
        if ch in (6, 8, 9, 13):
            continue
        print(f'ch{ch:02}')
        ch_data = data[data['channelid_plt'] == ch].reset_index(drop=True)
        vmon_vs_rate = vmonVsRate(v_mon=ch_data[f'vMon_{ch}'], rate=ch_data.rate_plt/ch_data.rate_hfet, v_set=program[f'v0_{ch}'])
        plotVmonVsRate(vmon_vs_rate=vmon_vs_rate, log_file=pathlib.Path(log_file), ch=ch)

def iv():
    iv_scans = (pathlib.Path('AutoScanLogs/Scan_2023_8_22_15_22_4.txt'), pathlib.Path('AutoScanLogs/Scan_2024_3_4_16_37_49.txt'))
    for log_file in iv_scans:
        program, scan_data = parseLogFile(log_file)
        for ch in range(16):
            print(f'{scan_data.timestamp.iloc[0].date()} Ch{ch:02}')
            data = pandas.DataFrame({'current (uA)': scan_data[f'iMon_{ch}'], 'voltage (V)': scan_data[f'vMon_{ch}']})
            data.plot(kind='scatter', x='voltage (V)', y='current (uA)', title=f'{scan_data.timestamp.iloc[0].date()} Ch{ch:02} current vs voltage', xlim=(0,850), ylim=(0,100))
            matplotlib.pyplot.savefig(f'iv_plots/{scan_data.timestamp.iloc[0].date()}-{ch:02d}.png', dpi=600)

if __name__ == "__main__":
    main(log_file='AutoScanLogs/Scan_2024_7_15_11_1_9.txt')

# ##########################################

# def calculateDeplVolt(scanDataCh: pandas.DataFrame, ch: int, thr1: float, thr2: float) -> int:
#     # calculate depletion voltage based on percent change:
#     # pick first value (highest to lowest HV setpoints) which is within 'thr1' of the previous value and within 'thr2' of next-to-previous value
#     scanDataCh = scanDataCh[::-1]  # reverse order of scan points (from highest to lowest)
#     def pctCh(per):
#         return scanDataCh[f"medianRateN{ch}"].pct_change(periods=per).abs()
#     pctP1 = pctCh(1)
#     pctP2 = pctCh(2)
#     deplVoltCh = scanDataCh[(pctP1 > thr1) & (pctP2 > thr2)]
#     if len(deplVoltCh):
#         deplVoltCh = scanDataCh[(pctP1 > thr1) & (pctP2 > thr2)].index[0]
#         deplVoltStDev = scanDataCh[f"medianRateN{ch}"][:deplVoltCh].std()
#     else:
#         print(f"\tUnable to estimate depletion voltage for Ch{ch}")  #:\n{scanDataCh}')
#         deplVoltCh = 0
#     return deplVoltCh

# def plotDeplVolt(ch: int, dataDir: str):
#     # plot depletion voltage vs time for channel 'ch'
#     # to do: plot deplVolt vs intLumi
#     import matplotlib.pyplot
#     deplVolt = loadDepletionVoltageJSON(dataDir)
#     dataCh = deplVolt[ch][deplVolt[ch] != 0]  # depletion voltage dataframe (drop entries where depletion voltage == 0 )
#     dataChDate = (dataCh.index.to_series().dt.strftime("%Y-%m-%d").to_list())  # to_series() required to do things to a DatetimeIndex [https://stackoverflow.com/a/49277956/13019084]
#     lumiByDaySeries = lumiByDay()
#     lumiPerScanDate = [lumiByDaySeries[pandas.to_datetime(scan.date())] for scan in dataCh.index.to_list()]  # Run2 cumulativeIntLumi for each scan date
#     fig, intLumiAx = matplotlib.pyplot.subplots(figsize=(10, 6))
#     intLumiAx.scatter(x=lumiPerScanDate, y=dataCh.to_list())
#     intLumiAx.set_title(f"Ch{ch} Depletion Voltage vs Integrated Luminosity", fontsize=14)
#     matplotlib.pyplot.xlabel("Integrated Luminosity (1/fb)")
#     matplotlib.pyplot.ylabel("Depletion Voltage (V)")
#     intLumiAx.axes.set_ylim(0.0, 800.0)
#     intLumiAx.axes.set_xlim(0.0, 165.0)
#     # Add secondary date axis [https://stackoverflow.com/a/33447004/13019084]
#     dateAx = intLumiAx.twiny()
#     dateAx.set_xlim(intLumiAx.get_xlim())  # set same range as intLumi axis
#     dateAx.set_xticks(lumiPerScanDate)  # copy location of intLumi x-ticks
#     dateAx.set_xticklabels(dataChDate, horizontalalignment="left")  # draw them as the date!
#     dateAx.tick_params(axis="x", labelrotation=45, labelsize=8)
#     [label.set_visible(False) for label in dateAx.get_xaxis().get_ticklabels()[1::2]]
#     # print every second xticklabel starting with the first [https://stackoverflow.com/a/50034357/13019084]
#     hvSetPoints = {"2016-09-09 16:10": "200V", "2017-08-10 01:10": "300V", "2017-10-18 21:00": "400V", "2018-03-22 17:45": "500V", "2018-06-10 04:50": "800V", "2018-08-18 04:35": "VcThr",}
#     # 200:[http://cmsonline.cern.ch/cms-elog/948105]  250:[http://cmsonline.cern.ch/cms-elog/1002826] 300:[http://cmsonline.cern.ch/cms-elog/1003149]
#     # 350:[http://cmsonline.cern.ch/cms-elog/1015071] 400:[http://cmsonline.cern.ch/cms-elog/1016344] 800:[http://cmsonline.cern.ch/cms-elog/1047254]
#     # VcThr:[http://cmsonline.cern.ch/cms-elog/1058918]
#     _ = [plotAxVline(lumiByDaySeries[pandas.to_datetime(date).date()], hv, "bottom") for date, hv in hvSetPoints.items()]
#     matplotlib.pyplot.tight_layout()
#     # matplotlib.pyplot.show()
#     matplotlib.pyplot.savefig(f"{dataDir}/ch{ch}DepletionVoltage.png", dpi=300)
#     matplotlib.pyplot.close("all")
