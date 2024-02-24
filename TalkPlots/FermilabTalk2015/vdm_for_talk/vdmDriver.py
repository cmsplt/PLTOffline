import pickle, csv, sys, json, os
import ROOT as r
from vdmUtilities import setupDirStructure
from makeScanFile import doMakeScanFile
from dataPrep_HF.makeHFRateFile import doMakeHFRateFile
from makeBeamCurrentFile import doMakeBeamCurrentFile
from dataPrep_corr.makeBeamBeamFile import doMakeBeamBeamFile
from dataPrep_corr.makeGhostsFile import doMakeGhostsFile
from dataPrep_corr.makeSatellitesFile import doMakeSatellitesFile
from dataPrep_corr.makeLengthScaleFile import doMakeLengthScaleFile
from makeGraphsFile import doMakeGraphsFile
from makeGraphs2D import doMakeGraphs2D
from vdmFitter import doRunVdmFitter

ConfigFile = sys.argv[1]

Config=open(ConfigFile)
ConfigInfo = json.load(Config)
Config.close()


Fill = str(ConfigInfo['Fill'])
Date = str(ConfigInfo['Date'])
Luminometer = str(ConfigInfo['Luminometer'])
AnalysisDir = str(ConfigInfo['AnalysisDir'])    

Corr = ConfigInfo['Corr']

makeScanFile = False
makeScanFile = ConfigInfo['makeScanFile']

makeHFRateFile = False
makeHFRateFile = ConfigInfo['makeHFRateFile']

makeBeamCurrentFile = False
makeBeamCurrentFile = ConfigInfo['makeBeamCurrentFile']

makeBeamBeamFile = False
makeBeamBeamFile =  ConfigInfo['makeBeamBeamFile']

makeGhostsFile =  False
makeGhostsFile =  ConfigInfo['makeGhostsFile']

makeSatellitesFile = False
makeSatellitesFile = ConfigInfo['makeSatellitesFile'] 

makeLengthScaleFile = False
makeLengthScaleFile = ConfigInfo['makeLengthScaleFile']

makeGraphsFile = False
makeGraphsFile = ConfigInfo['makeGraphsFile']

makeGraphs2D = False
makeGraphs2D = ConfigInfo['makeGraphs2D']

runVdmFitter = False
runVdmFitter = ConfigInfo['runVdmFitter']

print ""
print "Running with this config info:"
print "Fill ", Fill
print "Date ", Date
print "Luminometer ", Luminometer
print "AnalysisDir ", AnalysisDir
print "Corrections ", Corr
print "makeScanFile ", makeScanFile
print "makeBeamCurrentFile ", makeBeamCurrentFile
print "makeGraphsFile ", makeGraphsFile
print "runVdmFitter ", runVdmFitter

print ""
setupDirStructure(AnalysisDir, Luminometer, Corr)
print ""

if makeScanFile == True:

    makeScanFileConfig = ConfigInfo['makeScanFileConfig']
    
    print "Running makeScanFile with config info:"
    for key in makeScanFileConfig:
        print key, makeScanFileConfig[key]
    print ""

    makeScanFileConfig['Fill'] = Fill
    makeScanFileConfig['Date'] = Date

    OutputSubDir = str(makeScanFileConfig['OutputSubDir'])    
    outpath = './' + AnalysisDir + '/'+ OutputSubDir 

    table = {}
    csvtable = []

    table, csvtable = doMakeScanFile(makeScanFileConfig)

    csvfile = open(outpath+'/Scan_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(outpath+'/Scan_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)


if makeHFRateFile == True:

    makeHFRateFileConfig = ConfigInfo['makeHFRateFileConfig']
    makeHFRateFileConfig['Fill'] = Fill
    makeHFRateFileConfig['AnalysisDir'] = AnalysisDir

    print "Running makeHFRateFile with config info:"
    for key in makeHFRateFileConfig:
        print key, makeHFRateFileConfig[key]
    print ""

    OutputSubDir = AnalysisDir + "/" + str(makeHFRateFileConfig['OutputSubDir'])

    table = {}
    csvtable = []

    table, csvtable = doMakeHFRateFile(makeHFRateFileConfig)

    csvfile = open(OutputSubDir+'/Rates_HF_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(OutputSubDir+'/Rates_HF_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)
            

if makeBeamCurrentFile == True:

    makeBeamCurrentFileConfig = ConfigInfo['makeBeamCurrentFileConfig']
    makeBeamCurrentFileConfig['AnalysisDir'] = AnalysisDir

    print "Running makeBeamCurrentFile with config info:"
    for key in makeBeamCurrentFileConfig:
        print key, makeBeamCurrentFileConfig[key]
    print ""

    OutputSubDir = str(makeBeamCurrentFileConfig['OutputSubDir'])
    outpath = './' + AnalysisDir + '/' + OutputSubDir 

    InputScanFile = './' + AnalysisDir + '/' + str(makeBeamCurrentFileConfig['InputScanFile'])
    with open(InputScanFile, 'rb') as f:
        scanInfo = pickle.load(f)

    Fill = scanInfo["Fill"]     

    table = {}
    csvtable = []

    table, csvtable = doMakeBeamCurrentFile(makeBeamCurrentFileConfig)
    
    csvfile = open(outpath+'/BeamCurrents_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(outpath+'/BeamCurrents_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)


if makeBeamBeamFile == True:

    makeBeamBeamFileConfig = ConfigInfo['makeBeamBeamFileConfig']

    print "Running makeBeamBeamFile with config info:"
    for key in makeBeamBeamFileCOnfig:
        print key, makeBeamBeamFileConfig[key]
    print ""

    makeBeamBeamFileConfig['AnalysisDir'] = AnalysisDir
    makeBemBeamFielConfig['Fill'] =  Fill
    makeBeamBeamFileConfig['Luminometer'] =  Luminometer

    OutputDir = AnalysisDir +'/'+ConfigInfo["OutputSubDir"]

    table = {}
    csvtable = []
    table, csvtable = doMakeBeamBeamFile(ConfigInfo)

    csvfile = open(OutputDir+'/BeamBeam_' + Luminometer + '_' + str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(OutputDir+'/BeamBeam_'+ Luminometer + '_' +str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)




if makeGhostsFile == True:
    
    makeGhostsFileConfig = ConfigInfo['makeGhostsFileConfig']

    print "Running makeGhostsFile with config info:"
    for key in makeGhostsFileConfig:
        print key, makeGhostsFileConfig[key]
    print ""

    makeGhostsFileConfig['AnalysisDir'] = AnalysisDir
    makeGhostsFileConfig['Fill'] = Fill

    OutputDir = AnalysisDir +'/'+ makeGhostsFileConfig['OutputSubDir']

    table = {}
    csvtable = []
    table, csvtable = doMakeGhostsFile(makeGhostsFileConfig)

    csvfile = open(OutputDir+'/Ghosts_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(OutputDir+'/Ghosts_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)


if makeSatellitesFile == True:
    
    makeSatellitesFileConfig = ConfigInfo['makeSatellitesFileConfig']

    print "Running makeSatellitesFile with config info:"
    for key in makeSatellitesFileConfig:
        print key, makeSatellitesFileConfig[key]
    print ""

    makeSatellitesFileConfig['AnalysisDir'] = AnalysisDir
    makeSatellitesFileConfig['Fill'] = Fill

    OutputDir = AnalysisDir +'/'+ makeSatellitesFileConfig['OutputSubDir']

    table = {}
    csvtable = []
    table, csvtable = doMakeSatellitesFile(makeSatellitesFileConfig)

    csvfile = open(OutputDir+'/Satellites_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(OutputDir+'/Satellites_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)


if makeLengthScaleFile == True:
    
    makeLengthScaleFileConfig = ConfigInfo['makeLengthScaleFileConfig']

    print "Running makeLengthScaleFile with config info:"
    for key in makeLengthScaleFileConfig:
        print key, makeLengthScaleFileConfig[key]
    print ""

    makeLengthScaleFileConfig['AnalysisDir'] = AnalysisDir
    makeLengthScaleFileConfig['Fill'] = Fill

    OutputDir = AnalysisDir +'/'+ makeLengthScaleFileConfig['OutputSubDir']

    table = {}
    csvtable = []
    table, csvtable = doMakeLengthScaleFile(makeLengthScaleFileConfig)

    csvfile = open(OutputDir+'/LengthScale_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(OutputDir+'/LengthScale_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)


if makeGraphsFile == True:

    makeGraphsFileConfig = ConfigInfo['makeGraphsFileConfig']

    print "Running makeGraphsFile with config info:"
    for key in makeGraphsFileConfig:
        print key, makeGraphsFileConfig[key]
    print ""

    makeGraphsFileConfig['AnalysisDir'] = AnalysisDir
    makeGraphsFileConfig['Luminometer'] = Luminometer
    makeGraphsFileConfig['Fill'] = Fill
    makeGraphsFileConfig['Corr'] = Corr

    graphsListAll = {}

    corrFull, graphsListAll = doMakeGraphsFile(makeGraphsFileConfig)

    OutputSubDir = str(makeGraphsFileConfig['OutputSubDir'])
    outputDir = AnalysisDir +'/' + Luminometer + '/' + OutputSubDir + '/'
    outFileName = 'graphs_' + str(Fill) + corrFull

# save TGraphs in a ROOT file
    rfile = r.TFile(outputDir + outFileName + '.root',"recreate")

    for key in sorted(graphsListAll.iterkeys()):
        graphsList = graphsListAll[key]
        for key_bx in sorted(graphsList.iterkeys()):
            graphsList[key_bx].Write()

    rfile.Write()
    rfile.Close()

    with open(outputDir + outFileName + '.pkl', 'wb') as file:
        pickle.dump(graphsListAll, file)


if makeGraphs2D == True:

    makeGraphs2DConfig = ConfigInfo['makeGraphs2DConfig']

    print "Running makeGraphs2D with config info:"
    for key in makeGraphs2DConfig:
        print key, makeGraphs2DConfig[key]
    print ""

    makeGraphs2DConfig['AnalysisDir'] = AnalysisDir
    makeGraphs2DConfig['Luminometer'] = Luminometer
    makeGraphs2DConfig['Fill'] = Fill
    makeGraphs2DConfig['Corr'] = Corr

    graphs2DListAll = {}

    corrFull, graphs2DListAll = doMakeGraphs2D(makeGraphs2DConfig)

    InOutSubDir = makeGraphs2DConfig['InOutSubDir']
    outputDir = AnalysisDir + '/' + Luminometer + '/' + InOutSubDir + '/'

#2D graph file should be called graphs2D_<n>_<corrFull>.pkl
    GraphFile2D = outputDir + 'graphs2D_' + Fill + corrFull + '.pkl'

    file2D = open(GraphFile2D, 'wb')
    pickle.dump(graphs2DListAll, file2D)
    file2D.close()


if runVdmFitter == True:

    vdmFitterConfig = ConfigInfo['vdmFitterConfig']

    print "Running runVdmFitter with config info:"
    for key in vdmFitterConfig:
        print key, vdmFitterConfig[key]
    print ""

    vdmFitterConfig['AnalysisDir'] = AnalysisDir
    vdmFitterConfig['Luminometer'] = Luminometer
    vdmFitterConfig['Fill'] = Fill
    vdmFitterConfig['Corr'] = Corr

    FitName = vdmFitterConfig['FitName']
    FitConfigFile = vdmFitterConfig['FitConfigFile']
    InputGraphsFile = AnalysisDir + '/' + Luminometer + '/' + vdmFitterConfig['InputGraphsFile']

    corrFull = ""
    for entry in Corr:
        corrFull = corrFull + '_' + str(entry)
    if corrFull[:1] == '_':
        corrFull = corrFull[1:]
    if  not corrFull:
        corrFull = "noCorr"

    OutputDir = './' + AnalysisDir + '/' + Luminometer + '/results/' + corrFull + '/'

    if not os.path.isdir(OutputDir):
        print "Requested output directory ", OutputDir , " does not exist."
        print "Please check if input for chosen corrections is available."
        sys.exit(1)

    print " "
    print "ATTENTION: Output will be written into ", OutputDir
    print "Please check there for log files."

    print " "

    FitConfig=open(FitConfigFile)
    FitConfigInfo = json.load(FitConfig)
    FitConfig.close()

# needs to be the same name as assumed in the fit function python files, where it is ./minuitlogtmp/Minuit.log
    MinuitLogPath = './minuitlogtmp/'
    MinuitLogFile = MinuitLogPath + 'Minuit.log'
    if not os.path.isdir(MinuitLogPath):
        os.mkdir(MinuitLogPath, 0755)

# need to do this before each fitting loop
    if os.path.isfile(MinuitLogFile):
        os.remove(MinuitLogFile)

# needs to be the same name as assumed in vdmUtilities, where it is ./plotstmp
    PlotsPath = './plotstmp/'
    if not os.path.isdir(PlotsPath):
        os.mkdir(PlotsPath, 0755)
    else:
        filelist = os.listdir(PlotsPath)
        for element in filelist:
            os.remove(PlotsPath+element)

    resultsAll = {}
    table = []

    resultsAll, table = doRunVdmFitter(Fill, FitName, InputGraphsFile, OutputDir, FitConfigInfo)

    outResults ='./'+ OutputDir + '/'+FitName+'_FitResults.pkl'
    outFile = open(outResults, 'wb')
    pickle.dump(table, outFile)
    outFile.close()

    import csv
    csvfile = open('./'+ OutputDir + '/'+FitName+'_FitResults.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(table)
    csvfile.close()

#    outResults ='./'+ OutputDir + '/'+FitName+'_Functions.pkl'
#    outFile = open(outResults, 'wb')
#    pickle.dump(resultsAll, outFile)
#    outFile.close()

    outFileMinuit = './'+OutputDir + '/'+FitName+'_Minuit.log'
    os.rename(MinuitLogFile, outFileMinuit)

    outPdf = './'+OutputDir + '/'+FitName+'_FittedGraphs.pdf'
    filelist = os.listdir(PlotsPath)
    merge =-999.
    for element in filelist:
        if element.find(".ps") > 0:
            merge = +1.
    if merge > 0:
        os.system("gs -dNOPAUSE -sDEVICE=pdfwrite -dBATCH -sOutputFile="+outPdf+" " + PlotsPath+"/*.ps")

    outRoot = './'+OutputDir + '/'+FitName+'_FittedGraphs.root'
    if os.path.isfile(outRoot):
        os.remove(outRoot)
    merge =-999.
    for element in filelist:
        if element.find(".root") > 0:
            merge = +1.
    if merge > 0:
        os.system("hadd " + outRoot + "  " + PlotsPath + "*.root")



