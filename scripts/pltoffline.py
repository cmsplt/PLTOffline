#!/usr/bin/env python3

import os, sys, pandas
import typing as t # [https://docs.python.org/3/library/typing.html]

pltOfflinePath = '/afs/cern.ch/work/a/adelanno/PLT/PLTOffline'
os.chdir(pltOfflinePath)

fills = [ 4246, 4410, 4467, 4518, 4540, 4569, \
        4879, 5024, 5085, 5111, 5161, 5198, 5211, 5279, 5340, 5401, 5451, \
        5722, 5950, 6035, 6097, 6136, 6161, 6241, 6283, 6312, 6337, 6398, \
        6584, 6617, 6654, 6762, 6912, 6953, 7024, 7063, 7118, 7236, 7328 ]
    # subset of fills selected for further analysis
gainCalTS = [ '20150811.120552', '20150923.225334', '20151029.220336', \
            '20160819.113115', \
            '20170518.143905', '20170717.224815', '20170731.122306', '20170921.104620', '20171026.164159', \
            '20180419.131317', '20180430.123258', '20180605.124858', '20180621.160623']
    # selected gain calibration with "usable" results [https://github.com/cmsplt/PLTOffline/tree/master/GainCal/2020]
gainCalPerFill = { 4246:'20150811.120552', **dict.fromkeys( [4410,4467,4518],'20150923.225334' ), **dict.fromkeys( [4540,4569],'20151029.220336' ), \
                **dict.fromkeys( [4879,5024,5085,5111,5161,5198,5211,5279,5340,5401,5451],'20160819.113115' ), \
                5722:'20170518.143905', 5950:'20170717.224815', **dict.fromkeys( [6035,6097,6136,6161],'20170731.122306' ), **dict.fromkeys( [6241,6283,6312,],'20170921.104620' ), **dict.fromkeys( [6337,6398],'20171026.164159' ), \
                **dict.fromkeys( [6584,6617],'20180419.131317' ), 6654:'20180430.123258', 6762:'20180605.124858', **dict.fromkeys( [6912,6953,7024,7063,7118,7236,7328],'20180621.160623' ) }
    # assign same value to multiple keys [https://stackoverflow.com/a/45928598/13019084]

def printColor( color:str, message:str ):
    # print 'message' in foreground 'color' [http://ascii-table.com/ansi-escape-sequences.php]
    colors = { 'reset':'0', 'bold':'1', 'uline':'4', 'black':'30', 'red':'31', 'green':'32', 'yellow':'33', 'blue':'34', 'magenta':'35', 'cyan':'36', 'white':'37' }
    colors = { key: f'\033[{val}m' for key, val in colors.items() }
    print( f'{colors[color]}{message}{colors["reset"]}' )

def pltTimestamps() -> pandas.DataFrame:
    # import pltTimestamps.csv file as dataframe (contains slink and workloop timestamps corresponding to all stable beam fills)
    import csv
    def parseDate(x): return pandas.to_datetime( x, format = '%Y%m%d.%H%M%S' )
    with open( '/localdata/pltTimestamps.csv', 'r' ) as tsFile:
        cols = tsFile.readline().strip().split('|')
        tsFile.seek(0)
        dtypes = dict( zip( cols, ['int']+6*['str'] ) )
        pltTS = pandas.read_csv( tsFile, sep='|', dtype=dtypes, parse_dates=cols[1:5], date_parser=parseDate ) # [https://stackoverflow.com/a/37453925/13019084]
    return pltTS.set_index('fill').fillna('')

def writeBashCommand( scriptName:str, command:str ):
    # write or append command to a bash script
    scriptPath = f'{pltOfflinePath}/scripts/{scriptName}.sh'
    if not os.path.exists( scriptPath ):
        with open( f'{pltOfflinePath}/scripts/{scriptName}.sh', 'w' ) as scriptFile:
            scriptFile.write( '#!/usr/bin/env bash\n\n' )
            scriptFile.write( 'source "/cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.06.08/x86_64-centos7-gcc48-opt/root/bin/thisroot.sh"\n' )
            scriptFile.write( f'cd "{pltOfflinePath}/"\n\n' )
            scriptFile.write( f'{command}\n' )
    else:
        with open( f'{pltOfflinePath}/scripts/{scriptName}.sh', 'a' ) as scriptFile:
            scriptFile.write( f'{command}\n' )

def runSubprocess( command:str ):
    # run shell process and print stdout in realtime (hangs and crashes for ./calculateAlignment)
    # [https://janakiev.com/blog/python-shell-commands/]
    import re, subprocess, shlex
    printColor( 'green', f'{command}' )
    logFileName = list( filter( None,  re.split( "[./ ]", command ) ) )[0]
    logFile = open( f'{pltOfflinePath}/{logFileName}.log', 'a' )
    logFile.write( f'{80*"#"}\n$ {command}\n' )
    process = subprocess.Popen( shlex.split(command), shell=False, encoding='utf-8', stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
    while True:
        # [https://zaiste.net/posts/realtime-output-from-shell-command-python/]
        output = process.stdout.readline().rstrip()
        if output:
            print(output)
            logFile.write(f'{output}\n')
        else:
            break
    returnCode = process.wait() # [https://docs.python.org/3/library/subprocess.html#subprocess.Popen.wait]
    return returnCode

def moveOutputFiles( outFiles:t.List[str], outDir:str ):
    # verbosely move outFiles to outDir
    import os, shutil
    printColor( 'green', f'mkdir -p {outDir}; mv {outFiles} {outDir}' )
    os.makedirs( outDir, exist_ok=True )
    for file in outFiles:
        shutil.move( f'{file}', f'{outDir}/{file.split("/")[-1]}' )

def slink( fill:str ) -> t.List[str]:
    # return slinkFilesList corresponding to input fill
    pltTS = pltTimestamps()
    slinkFilesTS = pltTS.loc[fill].slinkTS.split()
    return slinkFilesTS

def gainCalFits( fill:str ) -> str:
    # return gainCalFitsFile corresponding to input fill (find closest fill if not defined in gainCalPerFill)
    def minKey(x): return abs(x-fill)
    closestFill = min( fills, key=minKey )
    gainCalTS = gainCalPerFill[ closestFill ]
    return f'{pltOfflinePath}/GainCal/2020/GainCalFits_{gainCalTS}.dat'

def gainCal( fill:str ) -> str:
    # return gainCalFile corresponding to input fill (find closest fill if not defined in gainCalPerFill)
    def minKey(x): return abs(x-fill)
    closestFill = min( fills, key=minKey )
    gainCalTS = gainCalPerFill[ closestFill ]
    return f'/localdata/{gainCalTS[:4]}/CALIB/GainCal/gaincalfast_{gainCalTS}.avg.txt'

def createGainCalFitFiles():
    # loop over all "usable" gainCal files and produce gainCalFits files
    import glob
    for ts in sorted( gainCalTS ):
        gainCalFile = f'/localdata/{ts[:4]}/CALIB/GainCal/gaincalfast_{ts}.avg.txt'
        rc = runSubprocess( f'./GainCalFastFits {gainCalFile}' )
        if rc == 0:
            moveOutputFiles( outFiles=glob.glob(f'{pltOfflinePath}/plots/GainCal/*gif'), outDir=f'{pltOfflinePath}/plots/GainCal/{ts}' )
    moveOutputFiles( outFiles=glob.glob(f'{pltOfflinePath}/GainCalFits_*'), outDir=f'{pltOfflinePath}/GainCal' )

def occupancyPlots( fill:int, **kwargs ):
    # produce occupancy plots for fills
    import glob
    if 'scriptName' in kwargs: scriptName = kwargs.get('scriptName')
    else: scriptName = fill
    for slinkTS in slink(fill):
        slinkFile = f'/localdata/{slinkTS[:4]}/SLINK/Slink_{slinkTS}.dat'
        outDir = f'{pltOfflinePath}/plots/Occupancy/Fill{fill}-Slink{slinkTS}/'
        writeBashCommand( scriptName, f'\nprintf "\\n$(tput setaf 2)%s$(tput sgr 0 0)\\n\\n" "./OccupancyPlots {slinkFile}"' )
        writeBashCommand( scriptName, f'./OccupancyPlots "{slinkFile}"' )
        writeBashCommand( scriptName, f'mkdir -p "{outDir}"' )
        writeBashCommand( scriptName, f'mv -fv "{pltOfflinePath}/plots/"Occupancy_* "{outDir}"' )
        # rc = runSubprocess( f'./OccupancyPlots {slinkFile}' )
        # if rc == 0:
        #     moveOutputFiles( outFiles=glob.glob('plots/Occupancy_*'), outDir=f'plots/Occupancy/Slink{slinkTS}-Fill{fill}' )

def calculateAlignment( fill:int, alignmentFile:str='{pltOfflinePath}/ALIGNMENT/Alignment_IdealInstall.dat', **kwargs ):
    # run ./CalculateAlignment for the given fill and alignment file
    import glob, shutil
    if 'scriptName' in kwargs: scriptName = kwargs.get('scriptName')
    else: scriptName = fill
    gainCalFile = gainCalFits(fill)
    for slinkTS in slink(fill):
        slinkFile = f'/localdata/{slinkTS[:4]}/SLINK/Slink_{slinkTS}.dat'
        outDir = f'{pltOfflinePath}/plots/Alignment/Fill{fill}-Slink{slinkTS}'
        writeBashCommand( scriptName, f'\nprintf "\\n$(tput setaf 2)%s$(tput sgr 0 0)\\n\\n" "./CalculateAlignment {slinkFile} {gainCalFile} {alignmentFile}"' )
        writeBashCommand( scriptName, f'./CalculateAlignment "{slinkFile}" "{gainCalFile}" "{alignmentFile}"' )
        writeBashCommand( scriptName, f'mkdir -p "{outDir}/"' )
        writeBashCommand( scriptName, f'mv -fv "{pltOfflinePath}/plots/Alignment/"*gif "{outDir}/"' )
        writeBashCommand( scriptName, f'mv -fv "{pltOfflinePath}/"*lignment.* "{outDir}/"' )
        writeBashCommand( scriptName, f'cp -v  "{outDir}/Trans_Alignment.dat" "{pltOfflinePath}/ALIGNMENT/Trans_Alignment_{fill}.dat"' ) # this will overwrite files if there are multiple slink files per fill
        # rc = runSubprocess( f'./CalculateAlignment {slinkFile} {gainCalFile} {alignmentFile}' )
        # if rc == 0:
        #     moveOutputFiles( outFiles=glob.glob('plots/Alignment/*gif'), outDir=f'plots/Alignment/Slink{slinkTS}-Fill{fill}' )
        #     moveOutputFiles( outFiles=glob.glob('*lignment.*'), outDir=f'ALIGNMENT/Slink{slinkTS}-Fill{fill}' )
        #     shutil.move( f'ALIGNMENT/Slink{slinkTS}-Fill{fill}/Trans_Alignment_.dat', f'ALIGNMENT/Trans_Alignment_{fill}.dat' )

def createAlignmentPerFill():
    # loop over fills and run ./CalculateAlignment (use "freshly-produced" Trans_Alignment_.dat as input to next iteration)
    alignmentFile = f'{pltOfflinePath}/ALIGNMENT/Alignment_IdealInstall.dat'
    for fill in fills:
        calculateAlignment( fill, alignmentFile, scriptName='createAlignmentPerFill' )
        alignmentFile = f'{pltOfflinePath}/ALIGNMENT/Trans_Alignment_{fill}.dat'

def createAlignmentPerYear():
    # loop over fills and run ./CalculateAlignment (use the "standard" alignment file for each year [https://github.com/cmsplt/PLTOffline/blob/master/ALIGNMENT/README])
    alignmentFile = '{pltOfflinePath}/ALIGNMENT/Alignment_IdealInstall.dat'
    for fill2015 in [ fill for fill in fills if fill<=4569 ]:
        calculateAlignment( fill2015, f'{pltOfflinePath}/ALIGNMENT/Trans_Alignment_4449.dat', scriptName='createAlignmentPerYear' )
    for fill2016 in [ fill for fill in fills if 4879<=fill<=5451 ]:
        calculateAlignment( fill2016, f'{pltOfflinePath}/ALIGNMENT/Trans_Alignment_4892.dat', scriptName='createAlignmentPerYear' )
    for fill2017 in [ fill for fill in fills if 5722<=fill<=6398 ]:
        calculateAlignment( fill2017, f'{pltOfflinePath}/ALIGNMENT/Trans_Alignment_6110.dat', scriptName='createAlignmentPerYear' )
    for fill2018 in [ fill for fill in fills if 6584<=fill<=7328 ]:
        calculateAlignment( fill2018, f'{pltOfflinePath}/ALIGNMENT/Trans_Alignment_2017MagnetOn_Prelim.dat', scriptName='createAlignmentPerYear' )

def main():
    # if os.system( 'command -v root 2>&1 >/dev/null' ) != 0:
    #     sys.exit( printColor( 'red', "source '/cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.06.08/x86_64-centos7-gcc48-opt/root/bin/thisroot.sh'" ) )
    # testFill = 5194
    # occupancyPlots( testFill )
    # calculateAlignment( testFill, f'{pltOfflinePath}/ALIGNMENT/Trans_Alignment_4892.dat' )
    createAlignmentPerFill()
    createAlignmentPerYear()

if __name__ == "__main__":
        main()

################################################################################

def defGainCalFilesPerFill() -> t.Dict[int,str]:
    # Dict of fills and corresponding gainCalib files
    # You will find below all the Fill-gaincal file pairs that I have used for the PH plots. As you will notice for 2016 data I'm using onle one gaincalfile, because this is the only one we have. Similar for late 2018 data; actually in this case the closest gaincal file (GainCalFits_20180802.181425.dat)  gives us weird results so I had to use the preceding file for all the last 3 months.
    # 2015: 4246 GainCalFits_20150811.120552.dat/
        #   4410 GainCalFits_20150923.225334.dat/
        #   4467 GainCalFits_20150923.225334.dat/
        #   4518 GainCalFits_20150923.225334.dat/
        #   4540 GainCalFits_20151029.220336.dat/
        #   4569 GainCalFits_20151029.220336.dat
    # 2016: 4879 GainCalFits_20160819.113115.dat/
        #   5024 GainCalFits_20160819.113115.dat/
        #   5085 GainCalFits_20160819.113115.dat/
        #   5111 GainCalFits_20160819.113115.dat/
        #   5161 GainCalFits_20160819.113115.dat/
        #   5198 GainCalFits_20160819.113115.dat/
        #   5211 GainCalFits_20160819.113115.dat/
        #   5279 GainCalFits_20160819.113115.dat/
        #   5340 GainCalFits_20160819.113115.dat/
        #   5401 GainCalFits_20160819.113115.dat/
        #   5451 GainCalFits_20160819.113115.dat
    # 2017: 5722 GainCalFits_20170518.143905.dat/
        #   5950 GainCalFits_20170717.224815.dat/
        #   6035 GainCalFits_20170731.122306.dat/
        #   6097 GainCalFits_20170731.122306.dat/
        #   6136 GainCalFits_20170731.122306.dat/
        #   6161 GainCalFits_20170731.122306.dat/
        #   6241 GainCalFits_20170921.104620.dat/
        #   6283 GainCalFits_20170921.104620.dat/
        #   6312 GainCalFits_20170921.104620.dat/
        #   6337 GainCalFits_20171026.164159.dat/
        #   6398 GainCalFits_20171026.164159.dat
    # 2018: 6584 GainCalFits_20180419.131317.dat/
        #   6617 GainCalFits_20180419.131317.dat/
        #   6654 GainCalFits_20180430.123258.dat/
        #   6762 GainCalFits_20180605.124858.dat/
        #   6912 GainCalFits_20180621.160623.dat/
        #   6953 GainCalFits_20180621.160623.dat/
        #   7024 GainCalFits_20180621.160623.dat/
        #   7063 GainCalFits_20180621.160623.dat/
        #   7118 GainCalFits_20180621.160623.dat/
        #   7236 GainCalFits_20180621.160623.dat/
        #   7328 GainCalFits_20180621.160623.dat
    gainCalFiles2015 = { 4246:'20150811.120552', 4410:'20150923.225334', 4467:'20150923.225334', 4518:'20150923.225334', 4540:'20151029.220336', 4569:'20151029.220336' }
    gainCalFiles2016 = { 4879:'20160819.113115', 5024:'20160819.113115', 5085:'20160819.113115', 5111:'20160819.113115', 5161:'20160819.113115', 5198:'20160819.113115', 5211:'20160819.113115', 5279:'20160819.113115', 5340:'20160819.113115', 5401:'20160819.113115', 5451:'20160819.113115' }
    gainCalFiles2017 = { 5722:'20170518.143905', 5950:'20170717.224815', 6035:'20170731.122306', 6097:'20170731.122306', 6136:'20170731.122306', 6161:'20170731.122306', 6241:'20170921.104620', 6283:'20170921.104620', 6312:'20170921.104620', 6337:'20171026.164159', 6398:'20171026.164159' }
    gainCalFiles2018 = { 6584:'20180419.131317', 6617:'20180419.131317', 6654:'20180430.123258', 6762:'20180605.124858', 6912:'20180621.160623', 6953:'20180621.160623', 7024:'20180621.160623', 7063:'20180621.160623', 7118:'20180621.160623', 7236:'20180621.160623', 7328:'20180621.160623' }
    gainCalFilesPerFill = { **gainCalFiles2015, **gainCalFiles2016, **gainCalFiles2017, **gainCalFiles2018 }
    return gainCalFilesPerFill

###########################################################################################

def manageOutputFiles( *args, **kwargs ):
    # define where various output files will be move to
    import glob
    out = { 'alignment':[ { f'plots/Alignment/{slinkTS}-{fill}':glob.glob('plots/Alignment/*gif') }, { f'ALIGNMENT/{slinkTS}-{fill}':glob.glob('*lignment.*') }  ] }

def loopOverFills( command:str, outFilesDict:t.Dict[t.List[str],str] ):
    # loop over fills and run stuff
    import glob, shutil
    gainCalFilesPerFill = defGainCalFilesPerFill()
    pltTS = pltTimestamps()
    for fill in fills:
        gainCalFile = f'GainCal/GainCalFits_{gainCalFilesPerFill[fill]}.dat'
        slinkListTS = pltTS.loc[fill].slinkTS.split()
        for slinkTS in slinkListTS:
            slinkFile = f'/localdata/{slinkTS[:4]}/SLINK/Slink_{slinkTS}.dat'
            rc = runSubprocess( command )
            if rc == 0:
                _ = [ moveOutputFiles( outFiles, outDir ) for outDir,outFiles in outFilesDict.items() ]

def createAlignmentFiles():
    command = './CalculateAlignment {slinkFile} {gainCalFile} {alignmentFile}'
    outFilesDict = { f'plots/Alignment/{slinkTS}-{fill}':glob.glob('plots/Alignment/*gif'), f'ALIGNMENT/{slinkTS}-{fill}':glob.glob('*lignment.*') }
    loopOverFills( f'{command}', outFilesDict )

# sorted( set( defGainCalFilesPerFill().keys() ) )
# sorted( set( defGainCalFilesPerFill().values() ) )
# gainCalFilesTS = ['20150811.120552', '20150923.225334', '20151029.220336', '20160819.113115', '20170518.143905', '20170717.224815', '20170731.122306', '20170921.104620', '20171026.164159', '20180419.131317', '20180430.123258', '20180605.124858', '20180621.160623']
# gainCalFilesTS = pandas.to_datetime( pandas.Index( gainCalFilesTS ), format = "%Y%m%d.%H%M%S" )
# pltTS = pltTimestamps()
# pltTS.loc[fills[10]]['start_stable_beam']
# gainCalFilesTS[ gainCalFilesTS.get_loc( pltTS.loc[fills[10]]['start_stable_beam'], method='ffill' ) ]

# fills2017 = [5722, 5833, 5950, 6035, 6097, 6136, 6161, 6241, 6263, 6283, 6312, 6337, 6370, 6398]
# fills2018 = [6584, 6617, 6654, 6694, 6719, 6762, 6912, 6953, 7024, 7063, 7118, 7236, 7328]

# 2015: 4246, 4349, 4410, 4467, 4518, 4540, 4569
# 2016: 4879, 4924, 5024, 5052, 5085, 5111, 5161, 5198, 5211, 5257, 5279, 5340, 5401, 5451
# 2017: 5722, 5834, 5950, 6035, 6097, 6138, 6165, 6241, 6263, 6283, 6312, 6343, 6370, 6399
# 2018: 6584, 6617, 6650, 6693, 6719, 6763, 6913, 6953, 7024, 7063, 7118, 7236, 7333

# 2017
# 5834: Noise in all channels towards the end of the fill. Workloop restart seems to have fixed the issue. No elog(?). Suggested replacement fill: 5833
# 6138: This is the exact fill where we lost Ch0 ROC0 [elog]. Suggested replacement fill: 6136
# 6165: PLT OFF due to cooling issue [elog]. Suggested replacement fill: 6161
# 6343: [Not sure if SLINK data is affected] Partial drop in Ch3 ROC2 after ~17h38 (305814:1839). SLINK processor crash [elog]. Suggested replacement fill: 6337
# 6399: PLT OFF due to cooling intervention [elog]. Suggested replacement fill: 6398

# 2018
# 6650: Partial drop in FED ch 11 (-zFar hub29) ROC0 [elog]. Suggested replacement fill: 6654
# 6693: Data interrupted towards beginning of fill. No elog(?). Suggested replacement fill: 6694
# 6763: Partial drop in Ch ROC1. Suggested replacement fill: 6762
# 6913: Very short fill. Suggested replacement fill: 6912
# 7118: Ch4 ROC0 dropped out
# 7236: Noise in Ch ROC0, Ch8 ROC0, Ch11 ROC2.
# 7333: Partial drop in -zFar from 20181023.091036 until 20181023.101236. Suggested replacement fill: 7328

def gaincalFiles( year ):
    import glob, re
    gainCalFilesList = sorted( glob.glob( f'/localdata/{year}/CALIB/GainCal/gaincalfast_*.avgErr.txt' ) )
    gaincalTS        = [ re.search( r'\d{8}\.\d{6}', gaincalFile ).group(0) for gaincalFile in gainCalFilesList ]
    # gaincalFilename = lambda year, tsList: [ f'/localdata/{year}/CALIB/GainCal/gaincalfast_{ts}.avgErr.txt' for ts in tsList ]
    # # ls /localdata/2017/CALIB/GainCal/*avgErr.txt | grep -Eo '[0-9]{8}.[0-9]{6}' | awk 'BEGIN {RS=""}{gsub(/\n/,"'\'','\''",$0); printf("gaincalTS2017 = ['\''"); printf("%s'\'']",$0)}'
    # gaincalTS2017 = ['20170504.113854','20170504.121153','20170505.163140','20170515.170709','20170518.143905','20170601.090534','20170613.184357','20170619.104910','20170707.214121','20170717.224815','20170731.122306','20170810.170619','20170815.135532','20170921.104620','20171026.164159','20171105.175146','20171126.141225']
    # gaincalFiles2018 = [ f'/localdata/2017/CALIB/GainCal/gaincalfast_{ts}.avgErr.txt' for ts in gaincalTS ]
    # # ls /localdata/2018/CALIB/GainCal/*avgErr.txt | grep -Eo '[0-9]{8}.[0-9]{6}' | awk 'BEGIN {RS=""}{gsub(/\n/,"'\'','\''",$0); printf("gaincalTS2018 = ['\''"); printf("%s'\'']",$0)}'
    # gaincalTS2018    = ['20180305.200728','20180419.131317','20180430.123258','20180518.122939','20180605.124858','20180621.160623','20180728.063248','20180802.181425']
    # gaincalFiles2018 = [ f'/localdata/2018/CALIB/GainCal/gaincalfast_{ts}.avgErr.txt' for ts in gaincalTS ]

# def identifyGaincalFile( wloopTS, gaincalTS ):
#     return gaincalTS.get_loc( wloopTS, method = 'pad' )

# pltTS = pltTimestamps()
# pltTS[ pltTS.stack().str.contains('20180510.205417').unstack().any(axis='columns') ] # pandas.stack() [https://stackoverflow.com/a/52338849/13019084] # any() [https://stackoverflow.com/a/58185114/13019084]
# pltTS[ pltTS.wloopTS.str.contains( '20180510.205417' ) ]
# pltTS[ [ '20180510.205417' in row for row in pltTS.wloopTS ] ] # [https://stackoverflow.com/a/55335207/13019084]
# year=2016; pltTS[ (pandas.to_datetime(f'{year}') < pltTS.start_time) & (pltTS.start_time<pandas.to_datetime(f'{year+1}')) ]
# [ f'Data_{type}_{file}.dat.gz' for file in pltTS[ pltTS.fill == 6666 ].wloopTS.iloc[0].split() for type in ['Histograms','Scaler','Status','Transparent'] ] # note Data_Histograms_*_V2.dat.gz

# for fill in fills2018:
#     slinkTS = pltTS[ pltTS.fill == fill ]['slinkTS'].to_list()

###########################################################################################

def parseInputArg( arg ):
    # parse 'arg' and return a list of corresponding file(s)
    import os, sys, re
    if re.search( r'\d{8}\.\d{6}', arg ):
        # if 'arg' contains 8 integers, followed by a 'dot' character, and followed 6 integers,
        # then parse as a date-time (YYYYMMDD.HHmmss) and check if corresponding scaler or slink file exists
        wloopFile = f'/localdata/{arg[0:4]}/WORKLOOP/Data_Scaler_{arg}.dat.gz'
        slinkFile = f'/localdata/{arg[0:4]}/SLINK/Slink/Slink_{arg}.dat'
        if os.path.isfile( wloopFile ):
            wloopFiles = [ wloopFile ]
        elif os.path.isfile( slinkFile ):
            slinkFiles = [ slinkFile ]
    elif re.search( r'\d{4,5}', arg ):
        # else, check if 'arg' contains 4 or 5 consecutive integers
        # then parse as fill number using readWorkloopTimestamps()
        # note that 'YYYYMMDD.HHmmss' will be a match, so check for a dot-separated datetime first
        wloopFiles = readWorkloopTimestamps( arg )
    return wloopFiles, slinkFiles

def readWorkloopTimestamps( fill ):
    # return list of scaler files corresponding to a fill
    # note that 'PLT-timestamps.txt' is created by 'PLT-timestamps.py' and relies on the CMSOMS API
    import os, csv
    tsFile = '/localdata/PLT-timestamps/PLT-timestamps.txt'
    if not os.path.isfile( tsFile ):
        _ = os.system( f'curl -s "https://raw.githubusercontent.com/cmsplt/PLTOffline/master/PLT-timestamps.txt" -o {tsFile}' )
    with open( tsFile, 'r' ) as pltTS:
        pltCSV = csv.reader( pltTS, delimiter='|' )
        fillData = [ line for line in pltCSV if line[0] == fill  ][0]
            # return the first line matching 'fill' in the first field of the csv file
    if fillData:
        tsList = fillData[5].split(' ') # 5th field corresponds to workloop timestamps
    wloopFiles = [ f'/localdata/{ts[0:4]}/WORKLOOP/Data_Scaler_{ts}.dat.gz' for ts in tsList ]
        # don't bother to check os.path.isfile() since fillData[5] is populated by glob().glob()
    return wloopFiles

def stripDateTS( wloopFile ):
    # return unix timestamp of date given file timestamp (ignore the time)
    import re, datetime
    fileDate = re.split( '[_\.]', scalerFile )[-4]
        # Splits scalerFile into underscore- and dot-separated list of strings and selects the 4th from last element
        # which should correspond to the date, given scalerFile = 'Data_Scaler_YYYYMMDD.HHmmss.dat.gz'
    fileDate = datetime.datetime.strptime( fileDate, '%Y%m%d' ).date()
        # converts date string into date object [https://www.journaldev.com/23365/python-string-to-datetime-strptime]
    fileDate = datetime.datetime.combine( fileDate, datetime.time( 0, 0, 0 ) )
        # adds midnight to the date object, making it a datetime object [https://stackoverflow.com/a/29298798/13019084]
    fileDateTS = fileDate.replace( tzinfo = datetime.timezone.utc ).timestamp()
        # applies UTC and converts datetime object to unix timestamp [https://www.tutorialspoint.com/How-to-convert-Python-date-to-Unix-timestamp]
    return fileDateTS

# $ ls *avgErr.txt
# gaincalfast_20180305.200728.avgErr.txt  gaincalfast_20180430.123258.avgErr.txt  gaincalfast_20180605.124858.avgErr.txt  gaincalfast_20180728.063248.avgErr.txt
# gaincalfast_20180419.131317.avgErr.txt  gaincalfast_20180518.122939.avgErr.txt  gaincalfast_20180621.160623.avgErr.txt  gaincalfast_20180802.181425.avgErr.txt

# fills2018=($( awk 'BEGIN {FS="|"} /\|2018/ {print $1}'  "${pltOfflinePath}/PLT-timestamps.txt" | tr '\n' ' ' ))
# slink2018=($( awk 'BEGIN {FS="|"} /\|2018/ {print $NF}' "${pltOfflinePath}/PLT-timestamps.txt" | tr '\n' ' ' ))
# alignmentFile='ALIGNMENT/Trans_Alignment_2017MagnetOn_Prelim.dat'
# gaincalFastFiles=('/localdata/2018/CALIB/GainCal/gaincalfast'*'.avg.txt')
# gaincalFastFile='/localdata/2018/CALIB/GainCal/gaincalfast_20180419.131317.avg.txt'
# ./CalculateAlignment "${slinkFile}" "${gaincalFastFile}" "${alignmentFile}"
