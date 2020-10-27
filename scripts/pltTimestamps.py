#!/usr/bin/env python3

# This script will create a csv file containing stable beam fill numbers with their corresponding start and end timestamps.
# It fetches LHC fill info from CMSOMS and requires the CMSOMS Aggregation API python client [https://gitlab.cern.ch/cmsoms/oms-api-client]
# It will find any workloop and slink files with filename timestamps corresponding to each fill (within fill-start and fill-end timestamps)

import os, sys, pandas
import typing as t # [https://docs.python.org/3/library/typing.html]

venvDir      = f'{os.path.expanduser("~")}/.local/venv/plt'
certFilePath = f'{os.path.expanduser("~")}/private/myCertificate'

def printColor( color:str, message:str ):
    # print 'message' in foreground 'color' [http://ascii-table.com/ansi-escape-sequences.php]
    colors = { 'reset':'0', 'bold':'1', 'uline':'4', 'black':'30', 'red':'31', 'green':'32', 'yellow':'33', 'blue':'34', 'magenta':'35', 'cyan':'36', 'white':'37' }
    colors = { key: f'\033[{val}m' for key, val in colors.items() }
    print( f'{colors[color]}{message}{colors["reset"]}' )

def fileTimestamps( year:int, fileType:str ) -> pandas.Series:
    # return all file timestamps a pandas series of sorted timestamps, given a year as an argument
    import re, glob
    if fileType == 'wloop':
        globString = f'/localdata/{year}/WORKLOOP/Data_Scaler*'
        sliceFrom = 2
    elif fileType == 'slink':
        globString = f'/localdata/{year}/SLINK/Slink*'
        sliceFrom = 1
    tsList = list( set( [ str.join( '.', re.split( '_|\.', filename )[sliceFrom:sliceFrom+2] ) for filename in glob.glob( globString ) ] ) )
        # list(set()) will remove duplicate entries, usually from both .gz and uncompressed versions of files
    if not tsList:
        sys.exit( printColor( 'red', f'No {fileType} files found for {year}. Please make sure to run script from pltoffline machine' ) )
    return pandas.to_datetime( pandas.Series( tsList ), format = '%Y%m%d.%H%M%S' ).sort_values()

def lhcTimestamps( omsapi, year:str ) -> pandas.DataFrame:
    # return a pandas dataframe with start and end timestamps for all stable beam fills, given a year as an argument
        # CMSOMS Aggregation API python client [https://gitlab.cern.ch/cmsoms/oms-api-client]
    query = omsapi.query("fills")
    query.filter( 'stable_beams', 'true', 'EQ' ).filter( 'start_time', f'{year}-01-01', 'GE' ).filter( 'end_time', f'{year+1}-01-01', 'LE' )
    query.per_page = 1000
    printColor( 'yellow', f'querying CMSOMS fill info for {year}...' )
    jsonData = query.data().json()['data']
    filteredData = { fill['id']: [ fill['attributes']['start_time'], fill['attributes']['start_stable_beam'], fill['attributes']['end_stable_beam'], fill['attributes']['end_time'] ] for fill in jsonData }
        # filter CMSOMS JSON data using a dictionary comprehension with fill number as key and start & end timestamps as values
    lhcTS = pandas.DataFrame.from_dict( filteredData, orient = 'index', columns = [ 'start_time', 'start_stable_beam', 'end_stable_beam', 'end_time' ] ).rename_axis('fill')
        # convert filtered dictionary into a pandas dataframe
    return lhcTS.apply( pandas.to_datetime, format = '%Y-%m-%dT%H:%M:%SZ' ) # convert all timestamps in dataframe from string to pandas datetime objects

def sortTS( seriesTS:pandas.Series, startTS:pandas.Timestamp, endTS:pandas.Timestamp ) -> str:
    # find all timestamps within fill-start (with 10-second tolerance) and fill-end timestamps and return as a string
    tsList = seriesTS[ ( seriesTS >= startTS-pandas.Timedelta(seconds=10) ) & ( seriesTS <= endTS ) ].to_list()
    return str.join( ' ', [ ts.strftime("%Y%m%d.%H%M%S") for ts in tsList ] ) # return a string with timestamps in the list separated by spaces

def gainCal( pltTS:pandas.DataFrame ) -> pandas.DataFrame:
    # insert selected gain calibration file timestamps with "usable" results [https://github.com/cmsplt/PLTOffline/tree/master/GainCal/2020]
        # start with most recent gainCal timestamp and assign to all fills until fill start_stable_beam timestamp < gainCal timestamp. and also constrain to be in the same year
    def gainCalNextFill( gainCalTS:str ): return pltTS.iloc[ pandas.Index(pltTS['start_stable_beam']).get_loc( gainCalTS.replace('.', ' '), method='backfill' ) ].name
        # find index (fill number) of the most proximate (but still larger) start_stable_beam timestamp to the input gainCalTS
    gainCalTS = [ '20150811.120552', '20150923.225334', '20151029.220336', \
                '20160819.113115', \
                '20170518.143905', '20170717.224815', '20170731.122306', '20170921.104620', '20171026.164159', \
                '20180419.131317', '20180430.123258', '20180605.124858', '20180621.160623']
    for ts in sorted( gainCalTS, reverse=True):
        pltTS.loc[ ( pltTS.start_time.dt.year == int(ts[0:4]) ) & ( pltTS.index >= gainCalNextFill(ts) ), 'gainCal' ] = ts
    pltTS.gainCal.fillna( method='backfill', inplace=True ) # propagate empty gainCal entries backwards from last valid entry
    return pltTS

def alignment( pltTS:pandas.DataFrame ) -> pandas.DataFrame:
    # populate "standard" alignment files for each year in 'pltTS' [https://github.com/cmsplt/PLTOffline/blob/master/ALIGNMENT/README]
    # def filtYear(year:int): return (f'{year}-01-01'<pltTS.start_time) & (pltTS.end_time<f'{year}-12-31')
    pltTS.loc[ pltTS.start_time.dt.year == 2015, 'alignment' ] = 'Trans_Alignment_4449.dat'
    pltTS.loc[ pltTS.start_time.dt.year == 2016, 'alignment' ] = 'Trans_Alignment_4892.dat'
    pltTS.loc[ pltTS.start_time.dt.year == 2017, 'alignment' ] = 'Trans_Alignment_2017MagnetOn_Prelim.dat'
    # pltTS.loc[ 6570:6579, 'alignment' ] = 'Trans_Alignment_6666.dat' # (example)
    return pltTS

def trackDist( pltTS:pandas.DataFrame ) -> pandas.DataFrame:
    # populate "standard" track distribution files for each year in 'pltTS' [https://github.com/cmsplt/PLTOffline/blob/master/TrackDistributions/README]
    pltTS.loc[ pltTS.start_time.dt.year == 2015, 'trackDist' ] = 'TrackDistributions_MagnetOn.txt'
    pltTS.loc[ pltTS.start_time.dt.year == 2016, 'trackDist' ] = 'TrackDistributions_MagnetOn2016_4892.txt'
    pltTS.loc[ pltTS.start_time.dt.year == 2017, 'trackDist' ] = 'TrackDistributions_MagnetOn2017_5718.txt'
    return pltTS

def cmsomsAuth():
    # verify that the CMSOMS Aggregation API python client [https://gitlab.cern.ch/cmsoms/oms-api-client] is available
        # if so, import it, and try to authenticate with user certificate, else try kerberos auth
    import importlib.util
    if importlib.util.find_spec( 'omsapi' ) is not None:
        import omsapi
        omsapi = omsapi.OMSAPI()
        if os.path.isfile( f'{certFilePath}.pem' ) & os.path.isfile( f'{certFilePath}.key' ):
            omsapi.auth_cert( f'{certFilePath}.pem', f'{certFilePath}.key' )
            printColor( 'green', '\nomsapi certificate authentication successful!\n' )
        else:
            certInfo()
            omsapi.auth_krb()
            printColor( 'green', '\nomsapi kerberos authentication successful!\n' )
    elif os.path.isfile( f'{venvDir}/bin/activate' ): venvInfo()
    else: cmsomsInfo()
    return omsapi

def certInfo():
    printColor( 'red', f'\nUser certificates not found in "{certFilePath}.pem" & "{certFilePath}.key"')
    if os.path.isfile( f'{certFilePath}.p12' ):
        convertCert()
    else:
        info  = '\tSetting up a user certificate file is strongly recommended in order to allow unattended authentication\n'
        info += '\tPlease acquire a *passwordless* user certificate from [https://ca.cern.ch/ca/user/Request.aspx?template=EE2User]\n' # [https://gitlab.cern.ch/cmsoms/oms-api-client#requirements]
        printColor( 'red', info )

def venvInfo():
    info  = 'OMSAPI could not be imported\n'
    info += f'A venv seems to present in "{venvDir}". Please make sure to activate it:\n'
    info += f'source "{venvDir}/bin/activate"'
    sys.exit( printColor( 'red', info ) )

def cmsomsInfo():
    info  = 'OMSAPI is required [https://gitlab.cern.ch/cmsoms/oms-api-client]\n'
    info += 'A python3 virtual environment is recommended, which can be set up by running:\n'
    info += '[https://github.com/cmsplt/PLTOffline/blob/master/setup_pltoffline.sh]'
    sys.exit( printColor( 'red', info ) )

def convertCert():
    # convert user certificate [https://gitlab.cern.ch/cmsoms/oms-api-client#requirements] [https://linux.web.cern.ch/docs/cernssocookie/#User%20certificates]
    printColor( 'green', f'\n"{certFilePath}.p12" user certificate/key file must be converted to specific formats\n' )
    printColor( 'red', 'The user certificate needs to be passwordless, so "Import Password" should be left blank' )
    stdErr = os.system( f'openssl pkcs12 -clcerts -nokeys -in {certFilePath}.p12 -out {certFilePath}.pem' )
    printColor( 'red', '"Import Password" should be blank again. A (4-or-more digit) "PEM pass phrase" is required but only used in the next step' )
    stdErr = os.system( f'openssl pkcs12 -nocerts -in {certFilePath}.p12 -out {certFilePath}.tmp' )
    printColor( 'red', 'Please repeat the "PEM pass phrase" once more' )
    stdErr = os.system( f'openssl rsa -in {certFilePath}.tmp -out {certFilePath}.key' )
    if stdErr == 0:
        os.system( f'rm -f {certFilePath}.tmp' )
        os.system( f'chmod 644 {certFilePath}.pem' )
        os.system( f'chmod 400 {certFilePath}.key' )
        printColor( 'green', 'User certificate files were configured successfully!' )

def pltTimestamps( year:int, omsapi ) -> pandas.DataFrame:
    wloopTS = fileTimestamps( year, 'wloop' )
    slinkTS = fileTimestamps( year, 'slink' )
    yearTS  = lhcTimestamps( omsapi, year )
    printColor( 'green', f'processing timestamps. beep boop...' )
    yearTS['wloopTS'] = [ sortTS( wloopTS, startTS, endTS ) for startTS,endTS in zip(yearTS['start_time'], yearTS['end_time']) ]
        # note that workloop timestamps are included from fill-declared to fill-end
    yearTS['slinkTS'] = [ sortTS( slinkTS, startTS, endTS ) for startTS,endTS in zip(yearTS['start_stable_beam'], yearTS['end_stable_beam']) ]
        # whereas, slink timestamps are included from stableBeam-declared to stableBeam-end
    return yearTS

def main():
    omsapi = cmsomsAuth()
    pltTS = pandas.DataFrame()
    for year in 2015, 2016, 2017, 2018:
        yearTS = pltTimestamps( year, omsapi )
        pltTS = pltTS.append( yearTS )
    pltTS = gainCal( pltTS )
    pltTS = alignment( pltTS )
    pltTS = trackDist( pltTS )
    pltTS[ pltTS.columns[0:4] ] = pltTS[ pltTS.columns[0:4] ].apply( lambda col: col.dt.strftime("%Y%m%d.%H%M%S") )
    with open( f'pltTimestamps.csv', 'w' ) as outFile:
        pltTS.to_csv( outFile, sep = '|' )

if __name__ == "__main__":
    main()

# #import datetime
# import numpy as np
# import os, sys
#
# file = open('PLT-timestamps.txt','w')
# fills, declaredTS, beginTS, endTS = np.loadtxt('TimeStamps.StableBeams', unpack = True, delimiter = ' ') # https://docs.scipy.org/doc/numpy/reference/generated/numpy.loadtxt.html
# # list( *map(int, fills) )                                                                               # https://stackoverflow.com/a/7368801
# slinkTS                           = np.loadtxt('TimeStamps.Slink',       unpack = True, delimiter = ' ')
# workloopTS                        = np.loadtxt('TimeStamps.Workloop',    unpack = True, delimiter = ' ')
#
# file.write( 'fill|fillDeclared|fillEnd|slinkTS|workloopTS\n' )
# #for i in range(len(fills)):
# for i,fill in enumerate(fills, start=0):                                                                 # http://treyhunner.com/2016/04/how-to-loop-with-indexes-in-python/
#     # https://stackoverflow.com/a/13871987
#     # https://stackoverflow.com/a/43141552
#     slinkIndex    = np.where( (slinkTS    >= beginTS[i])    & (slinkTS    <= endTS[i]) )
#     workloopIndex = np.where( (workloopTS >= declaredTS[i]) & (workloopTS <= endTS[i]) )
#     print(str.format(  "Fill {:.0f} ( fillDeclared={:.6f} | fillEnd={:.6f} )", fill, declaredTS[i], endTS[i] ) )
#     fill = str.format( "{:.0f}|{:.6f}|{:.6f}",                                 fill, declaredTS[i], endTS[i] )
#     file.write( fill + '|')
#     # https://stackoverflow.com/questions/25315816/numpy-number-array-to-strings-with-trailing-zeros-removed#comment39461514_25315816
#     # https://stackoverflow.com/a/35119046
#     # https://stackoverflow.com/a/255172
#     print( "\tslink timestamps: ",    *np.char.mod('%0.6f', slinkTS[slinkIndex]),       sep='\n\t\t' )
#     print( "\tworkloop timestamps: ", *np.char.mod('%0.6f', workloopTS[workloopIndex]), sep='\n\t\t' )
#     # https://stackoverflow.com/a/9360197
#     slink    = ' '.join( map( str, np.char.mod( '%0.6f', slinkTS[slinkIndex]       ) ) )
#     workloop = ' '.join( map( str, np.char.mod( '%0.6f', workloopTS[workloopIndex] ) ) )
#     file.write( str( slink    ) + '|' )
#     file.write( str( workloop ) + '\n' )
#
# file.close()
