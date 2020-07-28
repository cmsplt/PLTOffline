#!/usr/bin/env python3

# This script will create a csv file containing stable beam fill numbers with their corresponding start and end timestamps.
# It fetches LHC fill info from CMSOMS and requires the CMSOMS Aggregation API python client [https://gitlab.cern.ch/cmsoms/oms-api-client]
# It will find any workloop and slink files with filename timestamps corresponding to each fill (within fill-start and fill-end timestamps)

import os
venvDir      = f'{os.path.expanduser("~")}/.local/venv/plt'
certFilePath = f'{os.path.expanduser("~")}/private/myCertificate'

def printColor( color, message ):
    # Print 'message' in foreground 'color' [http://ascii-table.com/ansi-escape-sequences.php]
    colors = { 'reset':'0', 'bold':'1', 'uline':'4', 'black':'30', 'red':'31', 'green':'32', 'yellow':'33', 'blue':'34', 'magenta':'35', 'cyan':'36', 'white':'37' }
    colors = { key: f'\033[{val}m' for key, val in colors.items() }
    print( f'{colors[color]}{message}{colors["reset"]}' )

def cmsomsAuth():
    # Verify that the CMSOMS Aggregation API python client [https://gitlab.cern.ch/cmsoms/oms-api-client] is available
    # If so, import it, and try to authenticate with user certificate, else with kerberos
    # Otherwise, try authentivate with kerberos
    import os, sys, importlib.util
    if importlib.util.find_spec( 'omsapi' ) is not None:
        import omsapi
        omsapi = omsapi.OMSAPI()
        if os.path.isfile( f'{certFilePath}.pem' ) & os.path.isfile( f'{certFilePath}.key' ):
            omsapi.auth_cert( f'{certFilePath}.pem', f'{certFilePath}.key' )
        else:
            configCert()
            # omsapi.auth_krb()
    elif os.path.isfile( f'{venvDir}/bin/activate' ):
        info  = 'OMSAPI could not be imported\n'
        info += f'A venv seems to present in "{venvDir}". Please make sure to activate it:\n'
        info += f'source "{venvDir}/bin/activate"'
        sys.exit( printColor( 'red', info ) )
    else:
        info  = 'OMSAPI is required [https://gitlab.cern.ch/cmsoms/oms-api-client]\n'
        info += 'A python3 virtual environment is recommended, which can be set up by running:\n'
        info += '[https://github.com/cmsplt/PLTOffline/blob/master/setup_pltoffline.sh]'
        sys.exit( printColor( 'red', info ) )
    return omsapi

def configCert():
    # Check if user certificate file is present in 'certFilePath' and guide user through format conversions
    import os, sys
    if os.path.isfile( f'{certFilePath}.p12' ):
        printColor( 'green', f'{certFilePath}.p12 user certificate/key file must be converted to specific formats' )
        printColor( 'green', 'The user certificate needs to be passwordless, so import password should be left blank' )
        stderrCode     = os.system( f'openssl pkcs12 -clcerts -nokeys -in {certFilePath}.p12 -out {certFilePath}.pem' )
        if stderrCode == 0:
            printColor( 'green', 'Import password should be blank again. A PEM pass phrase is required but only needed in the next step' )
            stderrCode = os.system( f'openssl pkcs12 -nocerts         -in {certFilePath}.p12 -out {certFilePath}.tmp' )
        if stderrCode == 0:
            printColor( 'green', 'Please repeat the pass phrase once more' )
            stderrCode = os.system( f'openssl rsa                     -in {certFilePath}.tmp -out {certFilePath}.key' )
        if stderrCode == 0:
            printColor( 'green', 'User certificate was configured successfully! Please re-run the script' )
            stderrCode = os.system( f'rm -vf {certFilePath}.tmp' )
            stderrCode = os.system( f'chmod 644 {certFilePath}.pem' )
            stderrCode = os.system( f'chmod 400 {certFilePath}.key' )
        sys.exit()
    else:
        info  = 'Setting up a user certificate file is strongly recommended in order to allow unattended authentication (else, use omsapi.auth_krb() method)\n'
        info += 'Please acquire a *passwordless* user certificate from [https://ca.cern.ch/ca/user/Request.aspx?template=EE2User],\n' # [https://gitlab.cern.ch/cmsoms/oms-api-client#requirements]
        info += f'transfer it to {certFilePath}.p12, and re-run the script'
        sys.exit( printColor( 'red', info ) )

def fileTimestamps( year, fileType ):
    # Return all file timestamps a pandas series of sorted floats, given a year as an argument
    import sys, re, glob, pandas
    if fileType == 'wloop':
        globString  = f'/localdata/{year}/WORKLOOP/Data_Scaler*'
        sliceFrom   = 2
    elif fileType == 'slink':
        globString  = f'/localdata/{year}/SLINK/Slink*'
        sliceFrom   = 1
    tsList = [ str.join( '.', re.split( '_|\.', filename )[sliceFrom:sliceFrom+2] ) for filename in glob.glob( globString ) ]
        # glob.glob( f'path' )                      create iterator for all workloop/slink filenames for {year}
        # re.split( '_|\.', filename )[from:to] )   split the filename using '_' and '.' as delimiters into a list of strings and slice it  to filter the date (YYYYMMDD) and time (HHmmss)
        # str.join( '.', splitFilename )            merge the date (YYYYMMDD) and time (HHmmss) strings with a dot
        # [ timestamps for filename in filenames ]  list comprehension: loop over all filenames to extract timestamps as a sorted list
    if not tsList: sys.exit( printColor( 'red', f'No {fileType} files found for {year}. Please make sure to run script from pltoffline machine' ) )
    return pandas.Series( tsList ).apply( pandas.to_datetime, format = '%Y%m%d.%H%M%S' ).sort_values()

def lhcTimestamps( omsapi, year ):
    # Return a pandas dataframe with start and end timestamps for all stable beam fills, given a year as an argument
    # CMSOMS Aggregation API python client [https://gitlab.cern.ch/cmsoms/oms-api-client]
    import pandas
    # omsapi  = cmsomsAuth()
    query   = omsapi.query("fills")
    query.filter( 'stable_beams', 'true', 'EQ' ).filter( 'start_time', f'{year}-01-01', 'GE' ).filter( 'end_time', f'{year+1}-01-01', 'LE' )
    query.per_page  = 1000
    print( f'querying CMSOMS fill info for {year}...' )
    jsonData        = query.data().json()['data']
    filteredData    = { fill['id']: [ fill['attributes']['start_time'], fill['attributes']['start_stable_beam'], fill['attributes']['end_stable_beam'], fill['attributes']['end_time'] ] for fill in jsonData }
        # filter CMSOMS JSON data using a dictionary comprehension with fill number as key and start & end timestamps as values
    lhcTS = pandas.DataFrame.from_dict( filteredData, orient = 'index', columns = [ 'start_time', 'start_stable_beam', 'end_stable_beam', 'end_time' ] ).rename_axis('fill')
        # convert filtered dictionary into a pandas dataframe
    return lhcTS.apply( pandas.to_datetime, format = '%Y-%m-%dT%H:%M:%SZ' ) # convert all timestamps in dataframe from string to pandas datetime objects

def sortTS( fill, seriesTS, startTS, endTS ):
    # find all timestamps within fill-declared and fill-end timestamps and store them into a list
    tsList = seriesTS[ ( seriesTS >= fill[startTS] ) & ( seriesTS <= fill[endTS] ) ].to_list()
    return str.join( ' ', [ ts.strftime("%Y%m%d.%H%M%S") for ts in tsList ] ) # return a string with timestamps in the list separated by spaces

def main():
    omsapi = cmsomsAuth()
    import pandas
    pltTimestamps = pandas.DataFrame()
    for year in 2015, 2016, 2017, 2018:
        wloopTS = fileTimestamps( year, 'wloop' )
        slinkTS = fileTimestamps( year, 'slink' )
        lhcTS   = lhcTimestamps( omsapi, year )
        print( f'processing timestamps. beep boop...' )
        lhcTS['wloopTS'] = lhcTS.apply( sortTS, seriesTS=wloopTS, startTS='start_time',        endTS='end_time',        axis=1 )
            # note that workloop timestamps are included from fill-declared to fill-end
        lhcTS['slinkTS'] = lhcTS.apply( sortTS, seriesTS=slinkTS, startTS='start_stable_beam', endTS='end_stable_beam', axis=1 )
            # whereas, slink timestamps are included from stableBeam-declared to stableBeam-end
        lhcTS[ lhcTS.columns.to_list()[0:-2] ] = lhcTS[ lhcTS.columns.to_list()[0:-2] ].apply( lambda col: col.dt.strftime("%Y%m%d.%H%M%S") )
        pltTimestamps = pltTimestamps.append( lhcTS )
    with open( f'PLT-timestamps.txt', 'w' ) as outFile:
        pltTimestamps.to_csv( outFile, sep = '|' ) #, float_format = '%.6f' )

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
