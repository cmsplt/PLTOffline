#!/usr/bin/env python3

# parse CAEN vMon logs and identify HV scans
# crop vMon vs time data into pickle files and plot each scan candidate
# plots should be inspected by hand and "bad" scans specified in removeBadScans

import pandas
import typing as t # [https://docs.python.org/3/library/typing.html]

def timerDecorator( func ):
    # print the execution time for a function
    # [https://realpython.com/primer-on-python-decorators/]
    import functools, timeit
    @functools.wraps( func )
    def wrapper( *args, **kwargs ):
        t0      = timeit.default_timer()
        value   = func( *args, **kwargs )
        t1      = timeit.default_timer()
        print( f'\t{func.__name__}(): { t1 - t0 }s' )
        return value
    return wrapper

@timerDecorator
def mapLogFile( logfile:str ) -> pandas.DataFrame:
    # parse exported CAEN log files using newlines as delimiters
    import itertools
    def preprocessLine( line:str ) -> str: return line.rstrip( '\n' ).replace('"','').replace(',','',1)
        # remove trailing newline, remove all double-quotes, and remove the first comma (MM DD, YYYY) [https://docs.python.org/3/library/stdtypes.html]
    with open( logfile, 'r', encoding='utf-8-sig' ) as logFile:
        # utf-8-sig encoding removes byte order mark ('\ufeff') [https://stackoverflow.com/a/49150749/13019084]
        lines:t.List[str] = list( map( preprocessLine, logFile.readlines() ) )
    logData:t.List[t.List[str]] = [ list(channelData) for delim,channelData in itertools.groupby( lines, lambda z: z=='' ) if not delim ]
        # group logfile data into a sublist for each channel ( len(logData)==16 ) using an empty string ('') as delimiter
            # newlines are stripped to empty strings '' by line.rstrip( '\n' ) in preprocessLine()
            # split list by delimiter word [https://stackoverflow.com/a/15358422/13019084]
            # itertools.groupby() [https://docs.python.org/2/library/itertools.html#itertools.groupby] [https://realpython.com/python-itertools/#building-relay-teams-from-swimmer-data]
    channels:t.List[str] = [ dataCh[0].split('/')[-1] for dataCh in logData ]
        # list channels headers (sub-string after last '/' for the first entry in each sub-list)
    logDict:t.Dict[str,t.Dict[str,float]] = { channels[ch] : { entry.split(',')[0] : float( entry.split(',')[1] ) for entry in dataCh[2:] } for ch,dataCh in enumerate(logData) }
        # filter data into nested dictionary comprehension; parent keys correspond to channel headers (logDict.keys()) and child dictionaries contains timestamp keys and vMon values
    logDF = pandas.DataFrame.from_dict( logDict )
    logDF.index = pandas.to_datetime( logDF.index, format='%b %d %Y %H:%M:%S' ).tz_localize('GMT').tz_convert('UTC')
    return logDF

@timerDecorator
def filterLogFile( logfile:str ) -> pandas.DataFrame:
    # parse exported CAEN log files using channel headers as delimiters
    with open( logfile , 'r' ) as logFile:
        lines:t.List[str] = [ line.lstrip('\ufeff').rstrip('\n').replace('"','') for line in logFile.readlines() if line.strip() ]
            # 'if line.strip()' removes empty lines [https://docs.python.org/3/library/stdtypes.html#str.strip] [https://stackoverflow.com/a/40647977/13019084]
    logData:t.Dict[str,t.Dict[str,float]] = {}
    for line in lines:
        if 'CMS_PLT' in line:
            ch:str = line.split('/')[-1]
            logData[ch]:t.Dict[str,float] = {}
        elif 'Date,ACTUAL_VMON' in line:
            pass
        else:
            dt:str = str.join( '', line.split(',')[:2] )
            vmon:float = float( line.split(',')[-1] )
            logData[ch][dt] = vmon
    logDF = pandas.DataFrame( logData )
    logDF.index = pandas.to_datetime( logDF.index, format='%b %d %Y %H:%M:%S' ).tz_localize('GMT').tz_convert('UTC')
    return logDF

@timerDecorator
def dataframeDiff( mapLogFileDF:pandas.DataFrame, filterLogFileDF:pandas.DataFrame ) -> pandas.DataFrame:
    # check whether dataframes are identical
    print( f'mapLogFileDF.equals(filterLogFileDF): {mapLogFileDF.equals( filterLogFileDF )}' )
    print( f'len(mapLogFileDF) ?= len(filterLogFileDF): {len(mapLogFileDF) == len(filterLogFileDF)}' )
    comparison_df = mapLogFileDF.merge( filterLogFileDF, indicator=True, how='outer' )
        # pandas.DataFrame.merge [https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.DataFrame.merge.html] [https://hackersandslackers.com/compare-rows-pandas-dataframes/]
    return comparison_df[ comparison_df['_merge'] != 'both' ]

@timerDecorator
def fillLogDF( logDF:pandas.DataFrame ) -> pandas.DataFrame:
    # merge logDF with an empty 10-second-frequency dataframe and fill NaN gaps in logDF using ffill
    emptyDF = pandas.DataFrame( columns=logDF.columns, index=pandas.date_range( start=logDF.index[0], end=logDF.index[-1], freq='10s') )
        # create empty dataframe matching the duration of logDF with rows of 10-second frequency
    logDF = pandas.concat( [ emptyDF, logDF ] ).sort_index()
        # concatenate logDF to emptyDF making sure not to overwrite non-NaN values [https://stackoverflow.com/a/52422104/13019084]
    logDF = logDF.fillna(method='ffill').dropna()
        # fillna(method='ffill') propagates last valid observation forward to next valid. also drop NaN (right at the beginning of the log)
    return logDF

@timerDecorator
def identifyScanCandidates( logDF:pandas.DataFrame ) -> pandas.Series:
    # group together rows where vMon changes by at least 10V within 30 minute intervals ( where 30 min corresponds to longest "step" duration for scanCandidates)
    vmonDelta10 = logDF[ logDF[ logDF.diff().abs() >= 10 ].any(1) ] # any(1) [https://stackoverflow.com/a/46207540/13019084]
    identifier = ( vmonDelta10.index.to_series().diff() >= pandas.Timedelta(minutes=30)  ).cumsum()
        # cumsum() increments by one once rows in vmonDelta10 are separated by >=30min [https://stackoverflow.com/a/54403641/13019084]
        # to_series() required to apply diff() to a DatetimeIndex [https://stackoverflow.com/a/49277956/13019084]
    return identifier

def brilcalcTimestamps( beginTS:str, endTS:str ) -> pandas.DataFrame:
    # load brilcalc data for provided begin and end timestamps and return result as pandas dataframe
        # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalc]
    import subprocess, shlex
    command = f'brilcalc lumi --begin "{beginTS}" --end "{endTS}" --byls --tssec --output-style csv'
    try: output:t.List[str] = subprocess.check_output( shlex.split(command), encoding='utf-8' ).splitlines()
        # [https://janakiev.com/blog/python-shell-commands/] [https://docs.python.org/3/library/subprocess.html]
            # shlex.split() splits string by spaces into list of strings, but preserves quoted strings [https://stackoverflow.com/a/79985/13019084]
            # 'utf-8' returns string instead of binary output
            # splitlines() split string by line break [https://note.nkmk.me/en/python-split-rsplit-splitlines-re/]
    except subprocess.CalledProcessError: output:t.List[str] = ['','#run:fill,ls,time,beamstatus,E(GeV),delivered(/ub),recorded(/ub),avgpu,source']
    header:t.List[str] = output[1].split(',')
    lsData:t.List[str] = [ line.split(',') for line in output[2:-4] ]
        # skip line 0 (norm tag version), line 1 (column headers), and the last 4 lines (summary)
    df = pandas.DataFrame( lsData, columns=header )
    flCol = ['delivered(/ub)', 'recorded(/ub)', 'avgpu']
    df[flCol] = df[flCol].astype(float)
    df['dt'] = pandas.to_datetime( df.time, unit='s' )
    return df

def plotScanCandidate( scanCandidate:pandas.DataFrame, year:int ):
    # plot HV vs time for the scan candidate
    import matplotlib.pyplot, matplotlib.dates
    print( f'Plotting HV scan candidate {scanCandidate.index[0].strftime("%Y%m%d.%H%M%S")}...' )
    plot = scanCandidate.plot()
    plot.set_title( 'HV vs time', fontsize=14)
    matplotlib.pyplot.rc( 'legend', fontsize=8  )
    # matplotlib.pyplot.legend( [ f'Ch{ch:02d}' for ch in [*range(16)] ], loc='upper left', borderpad=0.1, labelspacing=0.1, framealpha=0.4 )
    matplotlib.pyplot.legend( ['Ch 0'] + 3*['_nolegend_'] + ['Ch 4'] + 10*['_nolegend_'] + ['All other Ch'], loc='upper left', borderpad=0.1, labelspacing=0.1, framealpha=0.4 )
    plot.xaxis.set_major_formatter( matplotlib.dates.DateFormatter('%Y.%m.%d-%H:%M') )
    filename = f'{scanCandidate.index[0].strftime("%Y%m%d.%H%M%S")}.png'
    matplotlib.pyplot.tight_layout()
    matplotlib.pyplot.savefig( f'ManualScanLogs/{year}/{filename}', dpi=300 )
    matplotlib.pyplot.close('all')

def removeBadScans():
    # inspect plots and tag bad scan candidates *by hand*
    import os
    badScans  = []
    badScans += ['20150612','20150916','20150921','20151027','20151101','20151102','20151124']
    badScans += ['20160429','20160527','20160627','20160630','20160706','20160707','20160908','20160909']
    badScans += ['20170904','20171120']
    badScans += ['20180413','20180512','20180912','20181005','20181020','20181021']
    _ = [ os.system( f'mv -fv ManualScanLogs/*/{scan}* ManualScanLogs/trash/ 2>/dev/null' ) for scan in badScans ]

def main():
    import os
    os.makedirs( 'ManualScanLogs/trash/', exist_ok=True )
    for year in [2015,2016,2017,2018]:
        os.makedirs( f'ManualScanLogs/{year}/', exist_ok=True )
        logfile = f'/scratch/DepletionVoltage/CaenLogs/{year}-vmon.csv'
        logDF = filterLogFile( logfile )
        logDF = fillLogDF( logDF )
        identifier = identifyScanCandidates( logDF )
        for key,grp in logDF.groupby( identifier ):
            scanCandidate = logDF[ ( logDF.index >= grp.index[0] - pandas.Timedelta(minutes=10) ) & ( logDF.index <= grp.index[-1] + pandas.Timedelta(minutes=10) ) ]
                # slice logDF within first and last entry in grp +- 10min
            print( f'{scanCandidate.index[0].strftime( "%Y%m%d.%H%M%S" )}' )
            brilcalcDF = brilcalcTimestamps( scanCandidate.index[0].strftime("%m/%d/%y %H:%M:%S"), scanCandidate.index[-1].strftime("%m/%d/%y %H:%M:%S") )
            if ( brilcalcDF.beamstatus.str.contains( 'STABLE|ADJUST', regex=True ).any() ):
                print( 'got one!' ) # require STABLEBEAMS or ADJUST from brilcalc
                scanCandidate.to_pickle( f'ManualScanLogs/{year}/{scanCandidate.index[0].strftime("%Y%m%d.%H%M%S")}.pkl' )
                plotScanCandidate( scanCandidate, year )
    removeBadScans()

if __name__ == "__main__":
    main()

##### FUNCTION GRAVEYARD #####
##### All functionality below replaced by identifyScanCandidates() #####

@timerDecorator
def timeDerivative( logDF:pandas.DataFrame ) -> pandas.DataFrame:
    # calculate time derivative [https://stackoverflow.com/q/50766061/13019084]
        # pandas.DataFrame.diff             [https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.DataFrame.diff.html]
        # pandas.Index.to_series            [https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.Index.to_series.html]
        # pandas.Series.dt.total_seconds    [https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.Series.dt.total_seconds.html]
    dV = logDF.diff()
    dT = logDF.index.to_series().diff().dt.total_seconds() # /3600
    derivative = pandas.DataFrame()
    for channel in logDF.keys():
        derivative[channel] = dV[channel] / dT
    return derivative

def binScanCandidates( logDF:pandas.DataFrame, derivative:pandas.DataFrame, floorFrequency:int = 60*4 ) -> pandas.Series:
    # identifies scan candidates and returns a pandas.Series of their approximate/coarse timestamp intervals (index) and occurrence of "large derivatives" (values)
        # pandas.DataFrame.dropna               [https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.DataFrame.dropna.html]
        # pandas.DatetimeIndex.floor()          [https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.DatetimeIndex.floor.html] [https://stackoverflow.com/a/50360271/13019084]
            # offset-aliases                    [https://pandas.pydata.org/pandas-docs/stable/user_guide/timeseries.html#offset-aliases]
        # pandas.DatetimeIndex.value_counts()   [https://pandas.pydata.org/pandas-docs/version/0.22.0/generated/pandas.DatetimeIndex.value_counts.html]
        # pandas.Series.sort_index()            [https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.Series.sort_values.html]
    timestamps = abs(derivative)[ abs(derivative) > 1 ].dropna( thresh=1 ).index.floor( freq=f'{floorFrequency}min' ).value_counts().sort_index()
        # keep rows with abs(derivative)>1 values (in at least one channel), group the timestamps (indices) into 'floorFrequency'-minute intervals based on the earliest one (floor),
        # count instances of abs(derivative)>1 within 'floorFrequency'-minute interval, and sort by timestamp [https://stackoverflow.com/a/50360271/13019084]
    return timestamps

def filterWithMargin( df:pandas.DataFrame, iEdge:pandas.Timestamp, iMargin:int, fEdge:pandas.Timestamp, fMargin:int ) -> pandas.DataFrame:
    # slice dataframe at initial and final timestamps with some initial and final margin
    return df[ ( df.index >= iEdge - pandas.Timedelta(minutes=iMargin) ) & ( df.index <= fEdge + pandas.Timedelta(minutes=fMargin) ) ].sort_index()

def fillNanGaps( scanCandidate:pandas.DataFrame, logDF:pandas.DataFrame ) -> pandas.DataFrame:
    # Fill NaN gaps in scanCandidate using ffill and previously valid observation
    emptyDF = pandas.DataFrame( columns = scanCandidate.columns, index = pandas.date_range( start=scanCandidate.index[0], end=scanCandidate.index[-1], freq='10s') )
        # create empty dataframe matching the duration of scanCandidate with rows of 10-second frequency
    scanCandidate = pandas.concat( [ emptyDF, scanCandidate ] ).sort_index()
        # concatenate scanCandidate to emptyDF making sure not to overwrite non-NaN values # [https://stackoverflow.com/a/52422104/13019084]
    scanCandidate = scanCandidate.fillna(method='ffill')
        # fillna(method='ffill') propagates last valid observation forward to next valid
    for ch in scanCandidate.columns:
        try: firstValidVmon = scanCandidate[ch].dropna().index[0]
            # find previously valid observation for each channel (which should correspond to the per-channel setpoint before the scan)
        except IndexError: firstValidVmon = scanCandidate[ch].index[0]
            # catch execption where scanCandidate[ch] is all NaN
        previousValidVmon = logDF[ch][ logDF.index < firstValidVmon ].dropna()
            # all previous valid (non-NaN) observations before firstValidVmon
        iSetpoint = previousValidVmon[-1] if len(previousValidVmon) else 0.0
            # previous valid (non-NaN) observations before firstValidVmon (unless there isn't one lol, in which case make it zero)
        scanCandidate[ch].fillna( iSetpoint, inplace=True )
            # fill NaN (which should all occur at beginning of the dataframe) with previously valid observation for each channel
    return scanCandidate

@timerDecorator
def filterScanCandidates( logDF:pandas.DataFrame, timestamps:pandas.Series ) -> t.List[pandas.DataFrame]:
    # determine and trim to "edges" of scan candidates and require STABLEBEAMS or ADJUST
    scanCandidates = []
    for ts in timestamps.index:
        scanCandidate = filterWithMargin( df=logDF, iEdge=ts, iMargin=30, fEdge=ts, fMargin=60*4+30 )
            # slice candidateScan vMon data into very coarse intervals (10hr around the scan candidate timestamp)
        scanCandidate = fillNanGaps( scanCandidate, logDF )
            # Fill NaN gaps in scanCandidate using ffill and previously valid observation
        scanEdges = scanCandidate[ scanCandidate.diff().abs() >= 10 ].dropna(thresh=1)
            # rows where vMon changes at least +-10V wrt to the previous row/sample
        if len( scanEdges ):
            scanCandidate = filterWithMargin( df=scanCandidate, iEdge=scanEdges.index[0], iMargin=10, fEdge=scanEdges.index[-1], fMargin=10 )
                # slice scanCandidate within +-10 min of the first and last timestamps in scanEdges
            print( f'{scanCandidate.index[0].strftime( "%Y%m%d.%H%M%S" )}' )
            brilcalcDF = brilcalcTimestamps( scanCandidate.index[0].strftime( "%m/%d/%y %H:%M:%S" ), scanCandidate.index[-1].strftime( "%m/%d/%y %H:%M:%S" ) )
            if ( brilcalcDF.beamstatus.str.contains( 'STABLE|ADJUST', regex=True ).any() ):
                # require STABLEBEAMS or ADJUST from brilcalc
                print( 'got one!' )
                scanCandidates.append( scanCandidate )
    return scanCandidates

def vmonScanCandidatesToPickle( logfile:str ):
    # identify scan candidates, apply stable beams requirement, and save to pickle file
    logDF           = filterLogFile( logfile )
    derivative      = timeDerivative( logDF )
    timestamps      = binScanCandidates( logDF, derivative )
    print( f'{len(timestamps)} scan candidate(s) identified. Filtering and checking for STABLEBEAMS or ADJUST...' ) #Querying brilcalc to require scans during stable beams' )
    scanCandidates  = filterScanCandidates( logDF, timestamps )
    for scan in scanCandidates:
        scan.to_pickle( f'ManualScanLogs/{scan.index[0].strftime( "%Y%m%d.%H%M%S" )}.vMon.pkl' )

def vmonScanCandidatePlots( year:int ):
    # plot each scan candidate from pickle file
    import glob
    for pklFile in sorted( glob.glob( f'ManualScanLogs/{year}*.pkl' ) ):
        scanCandidate = pandas.read_pickle( pklFile )
        plotScanCandidate( scanCandidate )

def mainOld():
    import os
    for year in [2017]: # [2015,2016,2017,2018]:
        logfile = f'/scratch/DepletionVoltage/CaenLogs/{year}-vmon.csv'
        vmonScanCandidatesToPickle( logfile )
        vmonScanCandidatePlots( year )
    # inspect plots and tag bad/redundant scan candidates *by hand*
    os.makedirs( 'ManualScanLogs/trash/', exist_ok = True )
    # badScans  = ['2015']
    # badScans += ['20160429','20160525','20160526','20160626','20160630','20160706','20160908']
    # badScans += ['20170809','20170904','20171010.142925','20171011.154429','20171120']
    #     # 20170809 # each _scanpoint_ has a corresponding histogram file LOL NOPE! [http://cmsonline.cern.ch/cms-elog/1003149] $ ll /localdata/2017/WORKLOOP/Data_Histograms_201708{09..10}*
    # badScans += ['20180512','20180523.131310','20180524','20180730.142359','']# ['20180413.07','20180512.15','20180525.07','20180626.03','20180701.23','20180728.03','20180912.07','20180925.11','20180930.150615','20181004.23','20181020.15','20181021.11']
    # _ = [ os.system( f'mv -fv ManualScanLogs/{scan}* ManualScanLogs/trash/ 2>/dev/null' ) for scan in badScans ]
