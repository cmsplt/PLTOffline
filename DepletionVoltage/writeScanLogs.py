#!/usr/bin/env python3

# reads vMon vs time data from pickle files (produced by identifyScans.py)
# and merges with per-channel rates from histogram files and weighted by HCAL rates from HCAL.
# writes the above to csv files in same format as auto-scan logs

import pandas, numpy
from typing import Dict, List, Tuple # [https://towardsdatascience.com/static-typing-in-python-55aa6dfe61b4]

def TimerDecorator( func ):
    # print the execution time for a function
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
def FindHistogramFile( scanStart:pandas.Timestamp, scanEnd:pandas.Timestamp ) -> List[str]:
    # return list of histogram file(s) corresponding to a given timestamp
    import glob
    path        = f'/localdata/{scanStart.year}/WORKLOOP'
    hFiles      = sorted( glob.glob( f'{path}/Data_Histograms_*_V2.dat.gz' ) )
    hFileTS     = pandas.Index( [ pandas.to_datetime( hFile.split('_')[2], format = "%Y%m%d.%H%M%S" ) for hFile in hFiles ] ).sort_values()
    hFileStart  = hFileTS.get_loc( scanStart, method='ffill' )
    hFileEnd    = hFileTS.get_loc( scanEnd,   method='bfill' )
    histFiles   = hFileTS[ hFileStart:hFileEnd ]
    return [ f'{path}/Data_Histograms_{ ts.strftime( "%Y%m%d.%H%M%S" ) }_V2.dat.gz' for ts in histFiles ]

def HistFileDate( histFilename:str ): # -> datetime.date:
    # return date object from filename string
    import re
    return pandas.to_datetime( re.search( r'\d{8}\.\d{6}', histFilename )[0], format="%Y%m%d.%H%M%S" ).date()

def HistFileDtype() -> numpy.dtype:
    # create structured dtype [https://docs.scipy.org/doc/numpy/user/basics.rec.html#structured-datatype-creation]
    nBX         = 3564
    names       = [ 'timestamp', 'orbit', 'nibble', 'lumisection', 'run', 'fill', 'channel', 'dataPerBX' ]
    formats     = [ numpy.uint32 ] * len( names )
    itemsizes   = [ '' ] * ( len( names ) - 1 ) + [ (nBX,) ] # (size,) [https://stackoverflow.com/a/57488298/13019084]
    histDtype   = numpy.dtype( list( zip( names, formats, itemsizes ) ) )
    return histDtype

@TimerDecorator
def LoadHistogramFile( histFilename:str, seekOffset:int = 0, numNibbles:int = 1 ) -> numpy.ndarray:
    # load compressed histogram log file into a numpy array
    import os, gzip, datetime
    histDtype = HistFileDtype()
    with gzip.open( histFilename, mode = 'rb' ) as hFile:
        if seekOffset: print( f'skipping first {seekOffset} lines and opening {numNibbles} line(s) in "{histFilename.split("/")[-1]}"' )
        hFile.seek( seekOffset * 16 * histDtype.itemsize, os.SEEK_SET ) # seek [https://stackoverflow.com/a/30126751/13019084]
        histData = numpy.frombuffer( hFile.read( numNibbles * 16 * histDtype.itemsize ), dtype = histDtype ) # read first line
        # histData = numpy.fromfile( hFile, dtype = numpy.uint32 ) # Infuriantingly, this yields garbage data!
            # "[gzip module] returns a file handle to the compressed stream" [https://github.com/numpy/numpy/issues/10866] [https://stackoverflow.com/a/58549964/13019084]
    return histData

def CalculateFileSeek( histFilename:str, scanStart:pandas.Timestamp, scanEnd:pandas.Timestamp ) -> Tuple[float,float]:
    # based on the first timestamp in the file, determine the amount of bytes to seek into the file and how many bytes to read
    # in order to get the per-channel rates for the length of the scanCandidate
    nibble = 2**12 * 3564 * 25*10**(-9) # 2^12 LHC orbits = 2^{12} * nBX * 25ns*( 10^{-9}s/ns )
    hFilenameDate = HistFileDate( histFilename )
    fileStartTS = pandas.to_datetime( set( LoadHistogramFile( histFilename, seekOffset=0, numNibbles=1 )['timestamp'] ).pop(), unit='ms' )
    fileStartTS = pandas.Timestamp.combine( hFilenameDate, fileStartTS.time() ).tz_localize('GMT').tz_convert('UTC')
        # histogram file contains no date info, so borrow it from 'histFilename'
    seekOffset = ( scanStart - fileStartTS ).total_seconds() / nibble if (scanStart-fileStartTS) > pandas.Timedelta(0) else 0
        # read from start of histogram file if histo filename timestamp is before the start of scan (indicating multiple histo files with midnight rollover)
    numNibbles = ( scanEnd - scanStart ).total_seconds() / nibble
    return ( seekOffset, numNibbles )

@TimerDecorator
def AggHistData( scanHistData:numpy.ndarray ) -> pandas.DataFrame:
    # sum per-bx data for each line and load into a pandas dataframe
    print( f'summing per-BX data...' )
    dataPerBXSum = ( [ entry[name] for name in scanHistData.dtype.names[0:-1] ] + [ ( entry['dataPerBX']&0x1fff ).sum() ] for entry in scanHistData )
        # generator comprehension [https://www.pythonlikeyoumeanit.com/Module2_EssentialsOfPython/Generators_and_Comprehensions.html#Creating-your-own-generator:-generator-comprehensions]
        # &0x1fff [https://gitlab.cern.ch/bril/cmsplt/-/blob/master/interface/src/histfile_check.cpp#L99]
    return pandas.DataFrame( dataPerBXSum, columns=scanHistData.dtype.names )

def ScanStartDate( scanHist:pandas.DataFrame, histFilename:str ): # -> datetime.date:
    # return the date for start of scan. increment a day if midnight has rolled over from the time the hist file was created until the start of the scan.
    # hopefully, no scans begin more than 24h after the creation of the histogram file!
    scanStartDate = HistFileDate( histFilename ) # date from filename
    histFileStartTime = pandas.to_datetime( set( LoadHistogramFile( histFilename, seekOffset=0, numNibbles=1 )['timestamp'] ).pop(), unit='ms' ).time() # first timestamp in histogram file
    scanStartTime = pandas.to_datetime( scanHist.timestamp, unit='ms' ).dt.time[0] # first timestamp from scan dataframe
    if scanStartTime < histFileStartTime: scanStartDate += pandas.Timedelta(days=1)
        # increment a day to date from filename if the first timestamp from the scan dataframe is smaller than the first timestamp in the histogram file
    return scanStartDate

def HistDateTimeCombine( scanHist:pandas.DataFrame, histFilename:str ) -> pandas.Series:
    # combine current date with time from scanHist.timestamp. handle midnight rollover during scan if needed.
    currentDate = ScanStartDate( scanHist, histFilename )
    histTime = pandas.to_datetime( scanHist.timestamp, unit='ms' ).dt.time
    histDT = pandas.Series( dtype='datetime64[ns]' )
    def CombineDT(row): return pandas.Timestamp.combine( currentDate, row )
    dayRollover = scanHist.timestamp[ scanHist.timestamp.diff() < 0 ] # find row(s) where scanHist.timestamp is smaller than the previous row
    if len(dayRollover) == 0:
        histDT = histDT.append( histTime.apply( CombineDT ) )
    if len(dayRollover) == 1:
        histDT = histDT.append( histTime[:dayRollover.index[0]].apply( CombineDT ) )
        currentDate += pandas.Timedelta(days=1)
        histDT = histDT.append( histTime[dayRollover.index[0]:].apply( CombineDT ) )
    histDT = pandas.DatetimeIndex( histDT ).tz_localize('GMT').tz_convert('UTC')
        # pandas.tz_localize and pandas.tz_convert only operate on pandas.DatetimeIndex because of reasons [https://stackoverflow.com/a/26090113/13019084]
    return histDT

def ProcessScanHist( histFilename:str, histSeek:Tuple[float,float], scanStart:pandas.Timestamp ) -> pandas.DataFrame:
    # read the per-channel data from histogram files and return a dataframe with pandas.Timestamp as the index
    scanHistData = LoadHistogramFile( histFilename, int(histSeek[0]), int(histSeek[1]) )
    scanHist = AggHistData( scanHistData )
    scanHist['dt'] = HistDateTimeCombine( scanHist, histFilename )
    return scanHist

def MergeDF( scanVmon:pandas.DataFrame, scanHist:pandas.DataFrame, ch:int ) -> pandas.DataFrame:
    # for channel 'ch', merge histogram data (per-channel rate) with CAEN HV data (vMon per-channel)
    def ChHV(q, o): return { i+o: f'PLTHV_H{q}T{i}' for i in range(4) }
    chVmon:Dict[int,str] = { **ChHV('mN',0), **ChHV('mF',4), **ChHV('pN',8), **ChHV('pF',12) } # [https://twiki.cern.ch/twiki/bin/viewauth/CMS/PLT#PLT_Channel_Map]
    scanHist = pandas.merge_asof( left=scanVmon[chVmon[ch]], right=scanHist[scanHist.channel==ch], left_on=scanVmon.index, right_on='dt' )[ ['dt',chVmon[ch],'dataPerBX'] ]
    return scanHist.rename( columns={ 'dt':'timestamp', f'{chVmon[ch]}':f'vMon{ch}', 'dataPerBX':f'rate{ch}' } )

@TimerDecorator
def WriteLogFile( scanVmon:pandas.DataFrame, scanHist:pandas.DataFrame ) -> pandas.DataFrame:
    # write logData (timestamps, per-channel rates, and per-channel vMon) to a csv file in the same format as the AutoScanLogs
    chOrder = [12,13,14,15,8,9,10,11,4,5,6,7,0,1,2,3]
    def PerChList(prefix): return [ f'{prefix}{i}' for i in chOrder ]
        # channel order hard-coded to match auto-HV scan log files
        # $ grep 'HpFT0,HpFT1,HpFT2,HpFT3,HpNT0,HpNT1,HpNT2,HpNT3,HmFT0,HmFT1,HmFT2,HmFT3,HmNT0,HmNT1,HmNT2,HmNT3' AutoScanLogs/*.txt
    logData = MergeDF( scanVmon, scanHist, ch=chOrder[0] ).timestamp # create "empty" DF (borrow datetime index from pandas.merge_asof(scanVmon.index,scanHist.dt) )
    for ch in chOrder: logData = pandas.merge_asof( left=logData, right=MergeDF( scanVmon, scanHist, ch ), left_on='timestamp', right_on='timestamp' )
    fnameTS = scanVmon.index[0].strftime("%Y_%m_%d_%H_%M_%S")
    logData['timestamp'] = logData.timestamp.dt.tz_convert( 'Europe/Amsterdam' )
    logData['timestamp'] = logData.timestamp.dt.strftime("%Y.%m.%d %H:%M:%S.%f")
    logData['ignore'] = '#M'
    for ch in chOrder: logData[f'iMon{ch}'] = 0
    logData['avgRate'] = logData[ PerChList('rate') ].mean(axis=1)
    logData = logData[ ['timestamp','ignore'] + PerChList('vMon') + PerChList('iMon') + PerChList('rate') + ['avgRate'] ].drop_duplicates().fillna(0)
    emptyRateRows = [ ( logData[ PerChList('rate') ] != 0 ).loc[row].any() for row in logData.index ]
    logData = logData[ emptyRateRows ]
    logData.to_csv( f'ManualScanLogs/Scan_{fnameTS}.txt', sep=',', header=False, index=False )
    return logData

def ProcessScan( scan:str, scanBoundaries:List ) -> pandas.DataFrame:
    # read vMon from pickle file and slice scan boundaries specified in 'scans' (provided by hand)
    import glob, sys
    from identifyScans import PlotScanCandidate
    def ScanBoundary( idx ):
        if scanBoundaries[idx]:
            return pandas.to_datetime( scanBoundaries[idx], format="%Y%m%d.%H%M" ).tz_localize('GMT').tz_convert('UTC')
        else:
            return scanVmon.index[-idx]
    scanVmon    = pandas.read_pickle( glob.glob( f'ManualScanLogs/*/{scan}*.pkl' )[0] )
    scanStart   = ScanBoundary( 0 )
    scanEnd     = ScanBoundary( 1 )
    scanVmon    = scanVmon[ (scanVmon.index >= scanStart) & (scanVmon.index <= scanEnd) ]
    PlotScanCandidate( scanVmon, year='' )
    print( f'\nProcessing scan candidate: {scanStart.strftime( "%Y%m%d.%H%M%S" )}\n' )
    histFiles   = FindHistogramFile( scanStart, scanEnd )
    scanHist    = pandas.DataFrame()
    for histFilename in histFiles:
        histSeek = CalculateFileSeek( histFilename, scanStart, scanEnd )
        scanHist = scanHist.append( ProcessScanHist( histFilename, histSeek, scanStart ) )
        if scanHist.dt.iloc[0] > scanVmon.index[-1] or scanHist.dt.iloc[-1] < scanVmon.index[0]:
            # exit if CalculateFileSeek somehow fails, e.g. logData = ProcessScan( '20161023', ['',''] )
            sys.exit( 'histogram timestamps do not overlap with vMon timestamps :(' )
    scanHist = scanHist.sort_values( by=['dt','channel'] )
    logData = WriteLogFile( scanVmon, scanHist )
    return logData

def main():
    # determine the scan start and end timestamps *by hand* based on the vMon vs time plots produced by PlotScanCandidate()
    scans2016 = { '20160422':['',''], '20160925':['',''] }
    scans2017 = { '20170807.10':['',''], '20170807.15':['',''], '20170809.22':['',''], '20170926':['',''], '20171010':['',''], '20171011':['',''], '20171016':['20171016.2023',''], '20171024':['',''], '20171103':['20171103.1654',''], '20171110':['',''] }
    scans2018 = { '20180510':['',''], '20180519':['20180519.0902',''], '20180523':['20180523.1320','20180523.1338'], '20180523':['20180523.1340',''], '20180529':['20180529.0948',''], '20180604':['','20180604.1008'], '20180604':['20180604.1010',''], '20180626':['20180626.0810','20180626.0830'], '20180626':['20180626.0856',''], '20180719':['',''], 20180731:['',''], 20180801:['',''], 20180808:['',''], 20180823:['',''], 20180905:['',''], '20181004':['','20181004.0930'], '20181004':['20181004.0940',''] }
    # gzip: /localdata/2018/WORKLOOP/Data_Histograms_20180815.014325_V2.dat.gz: unexpected end of file
    for scan,scanBoundaries in { **scans2016, **scans2017, **scans2018 }.items():
        logData = ProcessScan( scan, scanBoundaries )

if __name__ == "__main__":
    main()

# from writeScanLogs import *
# year=2016; scan='20161023'; import glob; scanVmon = pandas.read_pickle( glob.glob( f'ManualScanLogs/{year}/{scan}*.pkl' )[0] ); scanStart = scanVmon.index[0]; scanEnd = scanVmon.index[-1];
# histFiles = FindHistogramFile( scanStart, scanEnd ); # histFile = histFiles[0]; histSeek = CalculateFileSeek( histFile, scanStart, scanEnd )
# histFilename = histFiles[0]
# scanHist = pandas.DataFrame()
# histSeek = CalculateFileSeek( histFilename, scanStart, scanEnd )
# scanHistData = LoadHistogramFile( histFilename, int(histSeek[0]), int(histSeek[1]) )
# scanHist = AggHistData( scanHistData )
# scanHist['dt'] = HistDateTimeCombine( scanHist, histFilename )

# for histFile in histFiles:
#     histSeek = CalculateFileSeek( histFile, scanStart, scanEnd )
#     scanHist = scanHist.append( ProcessScanHist( histFile, histSeek, scanStart ) )
# scanHist = scanHist.sort_values( by=['dt','channel'] )
# logData     = WriteLogFile( scanVmon, scanHist )

###################################################################################################

@TimerDecorator
def NumpyLoadHistogramFile( histFile ):
    # open 'histFile' and return decoded buffer as a numpy array
    import subprocess
    print( f'Opening {histFile}...' )
    if histFile.split('.')[-1] == 'gz':
        # with gzip.open( histFile, mode = 'rb' ) as hFile:
        #     histData = numpy.frombuffer( hFile.read(), dtype = numpy.uint32 )
        # histData = subprocess.check_output( f'zcat {histFile}'.split(), encoding = 'utf-8' ).splitlines()
        histData  = numpy.frombuffer( subprocess.check_output( f'zcat {histFile}'.split() ), dtype = numpy.uint32 )
    elif histFile.split('.')[-1] == 'dat':
        with open( hFile, mode = 'rb' ) as hFile:
            histData  = numpy.fromfile( hFile, dtype = numpy.uint32 )
    return histData

def StructLoadHistogramFile():
    import gzip, struct
    histFile        = '/localdata/2018/WORKLOOP/Data_Histograms_20180823.125634_V2.dat.gz'
    with gzip.open( histFile, mode = 'rb' ) as hFile:
        # [http://vislab-ccom.unh.edu/~schwehr/rt/python-binary-files.html#sec-2_4]
        fields      = ( 'ts', 'orbit', 'nibble', 'ls', 'run', 'fill', 'channel' )
        values      = struct.unpack('7i', hFile.read( len( fields )*4 ) )
        data        = dict( zip( fields, values ) )
    print( f"Time: {pandas.to_datetime( data['ts'], unit='ms' ).time()} Nibble: {data['nibble']:2d} LS:{data['ls']:4d} Run: {data['run']:6d} Fill: {data['fill']:4d} Channel: {data['channel']:2d}" )
    # 'Time: 12:56:35.309000 Nibble: 10 LS: 461 Run: 321707 Fill: 7078'

