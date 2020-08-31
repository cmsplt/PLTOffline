#!/usr/bin/env python3

# read auto scan logs into a pandas dataframe, merge with HF data from brilcalc, group together and cleanup scan data
# (i.e. drop non-uniform values of per-channel-iMon and per-channel-rates),
# calculate depletion voltage (plateau of rate vs HV curve) and plot

import pandas
from typing import Dict, List, Tuple # [https://towardsdatascience.com/static-typing-in-python-55aa6dfe61b4]

def TimerDecorator( func ):
    # print the processing time for a function
    # [https://realpython.com/primer-on-python-decorators/]
    import functools, timeit
    @functools.wraps( func )
    def Wrapper( *args, **kwargs ):
        t0      = timeit.default_timer()
        value   = func( *args, **kwargs )
        t1      = timeit.default_timer()
        print( f'\t{func.__name__}(): { t1 - t0 }s' )
        return value
    return Wrapper

@TimerDecorator
def ParseLogFile( logFile:str ) -> pandas.DataFrame:
    # Define column headers and load log file into a pandas dataframe (only keep lines matching "#M")
    def PerChList(prefix): return [ f'{prefix}{i}' for i in [12,13,14,15,8,9,10,11,4,5,6,7,0,1,2,3] ]
        # channel order hard-coded to match auto-HV scan log files:
        # $ grep 'HpFT0,HpFT1,HpFT2,HpFT3,HpNT0,HpNT1,HpNT2,HpNT3,HmFT0,HmFT1,HmFT2,HmFT3,HmNT0,HmNT1,HmNT2,HmNT3' AutoScanLogs/*.txt
    cols = [ 'timestamp', 'ignore' ] + PerChList('vMon') + PerChList('iMon') + PerChList('rate') + [ 'avgRate' ]
        # [ timestamp, #M, vMon[0:15], iMon[0:15], rate[0:15], avgRate ]
    with open( f'{logFile}', "r" ) as log:
        data = [ line.rstrip( '\n' ).split(',') for num, line in enumerate( log ) if '#M' in line ]
    logData = pandas.DataFrame( data, columns = cols )
    logData.timestamp = pandas.DatetimeIndex( pandas.to_datetime( logData.timestamp, format = '%Y.%m.%d %H:%M:%S.%f' ) ).tz_localize('Europe/Amsterdam').tz_convert('UTC')
        # convert timestamps from local CERN time to UTC
        # pandas.tz_localize and pandas.tz_convert only operate on pandas.DatetimeIndex because of reasons [https://stackoverflow.com/a/26090113/13019084]
    floatCol = logData.columns.to_list()[2:]
    logData[floatCol] = logData[ floatCol ].apply( lambda x: x.astype( float ) )
    return logData

@TimerDecorator
def Brilcalc( beginTS:str, endTS:str, lmeter:str ) -> pandas.DataFrame:
    # load brilcalc hfet data for a given fill and load into a pandas dataframe
    # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalc]
    import subprocess, shlex
    command              = f'brilcalc lumi --begin "{beginTS}" --end "{endTS}" --byls --tssec --type {lmeter} --output-style csv'
    brilcalcData         = subprocess.check_output( shlex.split( command ), encoding = 'utf-8' ).splitlines()
        # [https://janakiev.com/blog/python-shell-commands/]
        # shlex.split() splits string by spaces into list of strings, but preserves quoted strings [https://stackoverflow.com/a/79985/13019084]
        # 'utf-8' returns string instead of binary output
        # splitlines() splits strings by line break [https://note.nkmk.me/en/python-split-rsplit-splitlines-re/]
    cols                 = brilcalcData[1].split(',')
    brilcalcData         = [ line.split(',') for line in brilcalcData[2:-4] ]
        # skip line 0 (norm tag version), line 1 (column headers), and the last 4 lines (summary)
    brilcalcDF           = pandas.DataFrame( brilcalcData, columns = cols )
    floatCol             = [ 'delivered(/ub)', 'recorded(/ub)', 'avgpu' ]
    brilcalcDF[floatCol] = brilcalcDF[ floatCol ].apply( lambda x: x.astype( float ) )
    brilcalcDF['dt']     = pandas.to_datetime( brilcalcDF.time, unit = 's', utc = True)
    return brilcalcDF

@TimerDecorator
def MergeDF( logData:pandas.DataFrame ) -> pandas.DataFrame:
    # merge logData and brilcalcDF based on closest timestamps and add new PLT/HF rate column to logData
    # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalc]
    beginTS     = logData.timestamp.iloc[0].strftime( "%m/%d/%y %H:%M:%S" )
    endTS       = logData.timestamp.iloc[-1].strftime( "%m/%d/%y %H:%M:%S" )
    brilcalcDF  = Brilcalc( beginTS, endTS, 'hfet' )
    if len(brilcalcDF) == 0:
        brilcalcDF  = Brilcalc( beginTS, endTS, 'bcm1f' )
    mergedDF    = pandas.merge_asof( left=logData, right=brilcalcDF, left_on='timestamp', right_on='dt' )
        # lol this worked? join dataframes using nearest timestamp
        # [https://www.reddit.com/r/learnpython/comments/6wbc7t/help_joining_two_pandas_dataframes/]
        # [https://towardsdatascience.com/5-lesser-known-pandas-tricks-e8ab1dd21431]
    for ch in range(16):
        logData[f'rateN{ch}'] = mergedDF[f'rate{ch}'] / mergedDF['avgpu']
    return logData.dropna()

def ProcessChannel( logData:pandas.DataFrame, ch:int ) -> pandas.DataFrame:
    # determine scanpoints for ch, filter logData for each ch and scanpoint based on uniformity of vMon and rateN values, and return median/std for rateCh & rateNCh
    vMonGroup = logData[ f'vMon{ch}' ].groupby( logData[ f'vMon{ch}' ].round( decimals = 0 ) ).count() > 4
        # group all integer-rounded vMon values and mark 'true' if vMon group size > 2
    scanSteps   = vMonGroup[ vMonGroup.values ].index.to_list()
    logDataCh   = logData[ [ 'timestamp', f'vMon{ch}', f'rate{ch}', f'rateN{ch}' ] ]
    medianRate, medianRateN, stdevRate, stdevRateN = [],[],[],[]
    for i, _ in enumerate( scanSteps ):
        # filter per-channel data into each HV scanstep \pm 2, select values within 5% of the last element in rateN{ch},
        # and calculate median and stdev
        logDataChStep = logDataCh[ (logDataCh[f'vMon{ch}']<=scanSteps[i]+2) & (logDataCh[f'vMon{ch}']>=scanSteps[i]-2) ]
        logDataChStep = logDataChStep[ logDataChStep[f'rateN{ch}'].apply( lambda x: 1 - x/logDataChStep[f'rateN{ch}'].iloc[-1] ).abs() < 0.05 ]
        medianRate.append(  logDataChStep[f'rate{ch}'].median()  )
        stdevRate.append(   logDataChStep[f'rate{ch}'].std()     )
        medianRateN.append( logDataChStep[f'rateN{ch}'].median() )
        stdevRateN.append(  logDataChStep[f'rateN{ch}'].std()    )
    data        = list( zip( medianRate, stdevRate, medianRateN, stdevRateN ) )
    cols        = [ f'medianRate{ch}', f'stdevRate{ch}', f'medianRateN{ch}', f'stdevRateN{ch}' ]
    scanDataCh  = pandas.DataFrame( data, index = scanSteps, columns = cols )
    return scanDataCh

def CalculateDeplVolt( scanDataCh:pandas.DataFrame, ch:int, thr1:float, thr2:float ) -> int:
    # calculate depletion voltage based on percent change:
    # pick first value (highest to lowest HV values) which is within 'thr1' of the previous value and within 'thr2' of next-to-previous value
    scanDataCh  = scanDataCh[::-1] # reverse order of scan points (from highest to lowest)
    def PctCh(per): return scanDataCh[f'medianRateN{ch}'].pct_change( periods = per ).abs()
    pctP1       = PctCh(1)
    pctP2       = PctCh(2)
    deplVoltCh  = scanDataCh[ (pctP1>thr1) & (pctP2>thr2) ]
    if len( deplVoltCh ):
        deplVoltCh    = scanDataCh[ (pctP1>thr1) & (pctP2>thr2) ].index[0]
        deplVoltStDev = scanDataCh[f'medianRateN{ch}'][:deplVoltCh].std()
    else:
        print(f'\tUnable to estimate depletion voltage for Ch{ch}') #:\n{scanDataCh}')
        deplVoltCh = 0
    return deplVoltCh

def PlotChannel( dateTime, ch, scanDataCh, deplVoltCh, dataDir ):
    # plot rate (raw and normalized) vs HV
    import os, pandas, matplotlib.pyplot
    scale   = scanDataCh[f'medianRateN{ch}'].iloc[-1] / scanDataCh[f'medianRate{ch}'].iloc[-1]
    fig, ax = matplotlib.pyplot.subplots()
    matplotlib.pyplot.style.use('seaborn-white')
    ax.set( title=f'Ch{ch} rate vs HV ({dateTime})', xlabel='', ylabel='' )
    ax.errorbar( scanDataCh.index, scanDataCh[f'medianRateN{ch}'], yerr=scanDataCh[f'stdevRateN{ch}'], ls="", marker="o", markersize='4', label='PLT/HF' )
    ax.errorbar( scanDataCh.index, scanDataCh[f'medianRate{ch}']*scale, yerr=scanDataCh[f'stdevRate{ch}']*scale, ls="", marker="o", markersize='4', label='PLT' )
    ax.legend( loc='lower right', borderpad=0.1, labelspacing=0.1, fancybox=True, framealpha=0.4)
    ax.axvline( deplVoltCh, color = 'red' )
    fig.tight_layout()
    os.makedirs( f'{dataDir}/{dateTime[:4]}/{dateTime}/', exist_ok = True )
    fig.savefig( f'{dataDir}/{dateTime[:4]}/{dateTime}/{dateTime}.ch{ch}.png', dpi = 300 )
    matplotlib.pyplot.close('all')

def PlotDeplVolt( ch, dataDir ):
    import glob, json, pandas, matplotlib.pyplot
    deplVolt = {}
    # for deplVoltFile in sorted( glob.glob( 'depletionVoltage.{dataDir}.json' ) ):
    #     with open( deplVoltFile, 'r' ) as deplVoltFile:
    #         deplVolt.update( json.load( deplVoltFile ) )
    with open( f'depletionVoltage.{dataDir}.json', 'r' ) as deplVoltFile:
        deplVolt = json.load( deplVoltFile )
    deplVolt = pandas.DataFrame.from_dict( deplVolt ).T
    deplVolt.index = [ fname.split('/')[1].lstrip('Scan_').rstrip('.txt') for fname in deplVolt.index.to_list() ]
    deplVolt.index = pandas.to_datetime( deplVolt.index, format = '%Y_%m_%d_%H_%M_%S' )
    plot = deplVolt[ch][ deplVolt[ch] != 0 ].plot( marker = "o", ls = '' )
    plot.xaxis.set_major_formatter( matplotlib.dates.DateFormatter( '%Y.%m.%d' ) )
    plot.axes.set_xlim( pandas.to_datetime('2016-04-01'), pandas.to_datetime('2018-11-01') )
    plot.axes.set_ylim( 0.0, 800.0 )
    matplotlib.pyplot.axvline( pandas.to_datetime('2018-08-18'), color='grey', linestyle='--', label='VcThr') # [http://cmsonline.cern.ch/cms-elog/1058918]
    plot.set_title( f'Ch{ch} Depletion Voltage vs Time', fontsize=14)
    matplotlib.pyplot.tight_layout()
    matplotlib.pyplot.savefig( f'{dataDir}/ch{ch}DepletionVoltage.png', dpi = 300 )
    matplotlib.pyplot.close('all')

def main():
    import glob, json
    dataDir = 'ManualScanLogs' # 'AutoScanLogs'
    deplVolt = {}
    # for year in [2016,2017,2018]:
    for logFile in sorted( glob.glob( f'{dataDir}/Scan_*.txt' ) ):
        print( f'\nprocessing {logFile}...' )
        deplVolt[logFile]   = []
        logData             = ParseLogFile( logFile )
        logData             = MergeDF( logData )
        dateTime            = logData.timestamp.iloc[0].strftime( "%Y%m%d.%H%M%S" )
        for ch in range(16):
            scanDataCh  = ProcessChannel( logData, ch )
            deplVoltCh  = CalculateDeplVolt( scanDataCh, ch, 0.01, 0.02 )
            deplVolt[logFile].append( deplVoltCh )
            PlotChannel( dateTime, ch, scanDataCh, deplVoltCh, dataDir )
    with open( f'depletionVoltage.{dataDir}.json', 'w' ) as deplVoltFile:
        json.dump( deplVolt, deplVoltFile )
    _ = [ PlotDeplVolt( ch, dataDir ) for ch in range(16) ]
    return deplVolt

if __name__ == "__main__":
    main()

###########################################################################

# Fitting function
def sigmoid( x, L ,x0, k, b ):
    y = L / ( 1 + np.exp( -k*( x - x0 ) ) ) + b
    return y


#Get plateau values and the corresponding vMon
def xVSy(ch, xy_axes, x_ax, y_ax, vMon_values1, vMon_values2):
    # Remove the rows that correspond to the initial decrease of voltage from the initial voltage
    bad_rows = 0
    for i in range(len(xy_axes[x_ax][:-2])):
        if abs(xy_axes[x_ax][i+1]-xy_axes[x_ax][i])/xy_axes[x_ax][i]>0.025 and abs(xy_axes[x_ax][i+2]-xy_axes[x_ax][i+1])/xy_axes[x_ax][i+1]<=0.025:
            bad_rows = i
            break
    bad_rows = bad_rows+1
    xy_axes = xy_axes[bad_rows:] #Remove bad_rows from initial rows
    xy_axes = xy_axes.sort_values(by = x_ax) #Sort them in increasing order
    xy_axes = xy_axes.reset_index(drop=True) #Rearrange the indexin from 0 to n-1
    #print(xy_axes)

    #get the bin edges within which you want to group values 
    delimiting_values = [xy_axes[x_ax][i] for i in range(len(xy_axes[x_ax][:-1])) if (abs(xy_axes[x_ax][i+1]-xy_axes[x_ax][i])/xy_axes[x_ax][i]>0.025)]
    delimiting_values.insert(0,0)
    delimiting_values.insert(len(delimiting_values),10*xy_axes[x_ax][len(xy_axes[x_ax][:-1])]) #10* is needed as you have to make sure the last edge of the bin has the max value of the last group
    bins = pd.cut(xy_axes[x_ax], delimiting_values)

    #groupby and get mean,std
    #print(xy_axes.groupby(bins)[x_ax].agg(['count', 'mean', 'std']))
    #print(xy_axes.groupby(bins)[y_ax].agg(['count', 'mean', 'std']))
    x_val = xy_axes.groupby(bins)[x_ax].agg(['mean']).values
    skip_n_values = 0 
    x_val = [x_val[i][0] for i in range(len(x_val)) if i>skip_n_values]
    x_err = xy_axes.groupby(bins)[x_ax].agg(['std']).values
    x_err = [x_err[i][0] for i in range(len(x_err)) if i>skip_n_values]
    y_val = xy_axes.groupby(bins)[y_ax].agg(['mean']).values
    y_val = [y_val[i][0] for i in range(len(y_val)) if i>skip_n_values]
    y_err = xy_axes.groupby(bins)[y_ax].agg(['std']).values
    y_err = [y_err[i][0] for i in range(len(y_err)) if i>skip_n_values]

    #fit
    p0 = [max(y_val), np.median(x_val)*0.75, 1, min(y_val)] # reasonable initial values, or the fit will not converge
    popt, pcov = curve_fit(sigmoid, x_val, y_val, p0, sigma=y_err, method='dogbox')
    continue_fit = True
    continue_fit_iterator = 0
    while continue_fit and continue_fit_iterator<20:
     popt_, pcov_ = curve_fit(sigmoid, x_val, y_val, popt, sigma=y_err, method='dogbox')
     if(max(abs(popt_-popt)/popt))<0.001:
      continue_fit = False
     else:
      popt = popt_
    plateau_value = popt_[0]+popt_[3]
    plateau_value_1 = plateau_value*0.95
    plateau_value_2 = plateau_value*0.995
    x_val_model = []
    y_val_model = []
    for x in range(int(min(x_val)), int(max(x_val)), 1): #int((max(x_val)-min(x_val))/100)):
     x_val_model.append(x)
     y_val_model.append(sigmoid(x, *popt_))
    x_plateau_value1 = np.interp(plateau_value_1, y_val_model, x_val_model)
    x_plateau_value2 = np.interp(plateau_value_2, y_val_model, x_val_model)
    vMon_values1.append(x_plateau_value1)
    vMon_values2.append(x_plateau_value2)
    #print(x_plateau_value)
    #plot
    fig = plt.figure(ch)
    fig.set_size_inches((40, 20), forward=False) #Number are W and H respectively
    plt.title("Channel %s" % (ch), size=50)
    plt.tick_params(labelsize=25)
    plt.xlabel('vMon (V)',size=50)
    plt.ylabel('rate',size=50)
    plt.errorbar(x_val, y_val, xerr=x_err, yerr=y_err, c='green', label='Measured points')
    plt.plot(x_val_model, sigmoid(x_val_model, *popt_), 'b-', label='Sigmoid fit')# Function:\n $y = %0.2f e^{%0.2f t} + %0.2f$' % ())
    plt.plot((min(x_val),x_plateau_value1), (plateau_value_1, plateau_value_1), 'r--', label='95% of sigmoid plateau')
    plt.plot((x_plateau_value1,x_plateau_value1), (min(y_val), plateau_value_1), 'r--')
    plt.plot((min(x_val),x_plateau_value2), (plateau_value_2, plateau_value_2), 'r-', label='99.5% of sigmoid plateau')
    plt.plot((x_plateau_value2,x_plateau_value2), (min(y_val), plateau_value_2), 'r-')
    handles,labels = plt.gca().get_legend_handles_labels()
    handles = [handles[3], handles[0], handles[1], handles[2]]
    labels = [labels[3], labels[0], labels[1], labels[2]] 
    plt.legend(handles, labels, loc='center left',prop={'size': 35})
    #plt.legend(loc='center left',prop={'size': 35})
    #mng = plt.get_current_fig_manager()
    #mng.resize(*mng.window.maxsize())
    fig.savefig("rate_vMon_"+str(ch)+".png")#, bbox_inches='tight')    

# channels = []
# vMon_values2 = []
# vMon_values1 = []
# for i in range(0,16):
#     ch=i
#     channels.append(ch)
#     x_ax = 'vMon'+str(ch)
#     y_ax = 'rate'+str(ch)
#     xy_axes = logData[ [x_ax, y_ax] ]
#     xVSy(ch, xy_axes, x_ax, y_ax, vMon_values1, vMon_values2)
# print(vMon_values1)
# print(vMon_values2)

#Final plot
# def finalPlot():
#     fig = plt.figure("vMonPlateau vs channel")
#     fig.set_size_inches((40, 20), forward=False) #Number are W and H respectively
#     plt.tick_params(labelsize=25)
#     plt.xlabel('PLT channel',size=50)
#     plt.ylabel('vMon at the rate plateau (V)',size=50)
#     for ch in channels:
#         plt.plot((channels[ch],channels[ch]), (vMon_values1[ch],vMon_values2[ch]), 'b--')
#     #plt.plot(channels,vMon_values1, 'bo', label='95% of sigmoid plateau')
#     #plt.plot(channels,vMon_values2, 'ro', label='99.5% of sigmoid plateau')
#     #plt.legend(loc='lower left',prop={'size': 35})
#     fig.savefig("vMon_at_the_rate_plateau.png")
#     #plt.show()
