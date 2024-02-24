import FitManager
import SG_Fit
import SGConst_Fit
import DG_Fit
import DGConst_Fit
import GSupGConst_Fit
import DG_2D_Fit
from vdmUtilities import showAvailableFits

import sys
import json
import pickle
import os
import ROOT as r

def doRunVdmFitter(Fill, FitName, InputGraphsFile, OutputDir, FitConfigInfo):

    showAvailableFits()
    
    availableFits = FitManager.get_plugins(FitManager.FitProvider)

    key = FitName + '_Fit'
    if key not in availableFits:
        print "Fit " + FitName + " requested via json file does not exist, nothing to fit with, exit."
        sys.exit(1)

    fitter = availableFits[FitName+'_Fit']()

    FitLogFile = OutputDir + FitName +'.log'
    fitlogfile = open(FitLogFile,'w') 
    sysstdout = sys.stdout
    sys.stdout = fitlogfile
    sysstderr = sys.stderr
    sys.stderr = fitlogfile

# temporarily: move graphs files over from forTmpStorage directory
#list = os.listdir('./forTmpStorage/')
#import shutil
#for element in list:
#    shutil.copy('./forTmpStorage/'+element, InputDataDir)

# for 2D fits

    if '_2D' in FitName:

# test that when 2D fit function is requested, the graph file used is also a 2D one
        if not '2D' in InputGraphsFile:
            print "--?? You selected a 2D fitting function, but chose as input a graphs file without the \"2D\" in its name"
            print "??--"
            print " "
            sys.exit(1)

    if not os.path.isfile(InputGraphsFile):        
        print "--?? Input data file ", InputGraphsFile ," does not exist"
        print "??--"
        print " "
        sys.exit(1)

    print " "
    print "Now open input graphs file: ", InputGraphsFile

    graphsAll = {}
    infile = open(InputGraphsFile, 'rb')
    graphsAll = pickle.load(infile)
    infile.close()

    print graphsAll

# if input file is 2D graphs file, also open the corresponding 1D graph file
    if '_2D' in FitName:
        fileName1D = InputGraphsFile.replace("graphs2D", "graphs")
        infile1D = open(fileName1D, 'rb')
        graphs1D = pickle.load(infile1D)

#        print graphsList1D

    resultsAll = {}

# first loop over scan numbers

    print ">>", graphsAll.keys()
    for keyAll in sorted(graphsAll.keys()):
        print "Looking at ", keyAll
        graphs = {}
        results = {}
        graphs = graphsAll[keyAll]
        if '_2D' in FitName:
            keyX = "Scan_" + keyAll.split("_")[1]
            keyY = "Scan_" + keyAll.split("_")[2]
            print "keyX", keyX
            print "keyY", keyY
            graphsX = graphs1D[keyX] 
            graphsY = graphs1D[keyY] 


# order keys in natural order, i.e. from smallest BCID to largest

# determine which of the bcid among those with collisions are indeed represented with a TGraphErrors() in the input graphs file
# need to do this because PCC uses only subset of 5 bcids of all possible bcids with collisions

        print graphs.keys()
        for key in sorted(graphs.keys(), key=int):
            print "------>>>>"
            print "Now fitting BCID ", key
            graph = graphs[key]
            if '_2D' in FitName:
                result = fitter.doFit2D(graph, graphsX[key], graphsY[key], FitConfigInfo)
                for entry in result:
                    print ">>", result, type(result)
                results[key] = result
                functions = result[0]
                canvas = fitter.doPlot2D(graphsX[key], graphsY[key], functions, Fill)
            else:    
                result = fitter.doFit(graph, FitConfigInfo)
                for entry in result:
                    print ">>", result, type(result)
                results[key] = result
                functions = result[0]
                canvas = fitter.doPlot(graph, functions, Fill)

        for entry in fitter.table:
            print entry

    resultsAll[keyAll] = results
    print resultsAll


    sys.stdout = sysstdout
    sys.stderr = sysstderr
    fitlogfile.close()

    return resultsAll, fitter.table


if __name__ == '__main__':


    ConfigFile = sys.argv[1]

    Config=open(ConfigFile)
    ConfigInfo = json.load(Config)
    Config.close()


    Fill = str(ConfigInfo['Fill'])
    Luminometer = str(ConfigInfo['Luminometer'])
    Corr = ConfigInfo['Corr']
    AnalysisDir = str(ConfigInfo['AnalysisDir'])
    FitName = str(ConfigInfo['FitName'])
    FitConfigFile = str(ConfigInfo['FitConfigFile'])
    InputGraphsFile = AnalysisDir + '/' + Luminometer + '/' + str(ConfigInfo['InputGraphsFile'])

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

#    outResults ='./'+ OutputDir + '/'+FitName+'_FitFunctions.pkl'
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




    


