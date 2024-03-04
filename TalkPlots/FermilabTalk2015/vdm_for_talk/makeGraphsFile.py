import CorrectionManager
import BeamBeam_Corr
import LengthScale_Corr
import Ghosts_Corr
import Satellites_Corr
import ROOT as r
import sys
import json

from inputDataReader import * 
from vdmUtilities import showAvailableCorrs


def doMakeGraphsFile(ConfigInfo):

    AnalysisDir = str(ConfigInfo['AnalysisDir'])
    Luminometer = str(ConfigInfo['Luminometer'])
    inputScanFile = str(ConfigInfo['InputScanFile'])
    inputBeamCurrentFile = str(ConfigInfo['InputBeamCurrentFile'])
    inputLuminometerData = str(ConfigInfo['InputLuminometerData'])
    corrName = ConfigInfo['Corr']

# For scan 1, which is always there as long as there are any scans at all:

    inData1 = vdmInputData(1)

    inData1.GetScanInfo(AnalysisDir + '/'+ inputScanFile)
#inData1.PrintScanInfo()
 
#print inData1.fill
#print inData1.date
#print inData1.run
#print inData1.inputDIPFile
#print inData1.scanName
#print inData1.scanNamesAll
#print inData1.scanTimeWindows
#print inData1.betaStar
#print inData1.angle
#print inData1.particleTypeB1
#print inData1.particleTypeB2
#print inData1.filledBunchesB1
#print inData1.filledBunchesB2
#print inData1.collidingBunches
#print inData1.scanNumber
# print inData1.sp
#print inData1.tStart
#print inData1.tStop
#print inData1.displacement


    inData1.GetBeamCurrentsInfo(AnalysisDir + '/' + inputBeamCurrentFile)
#inData1.PrintBeamCurrentsInfo()

#print inData1.curr
#print inData1.avrgDcctB1
#print inData1.avrgDcctB2
#print inData1.sumAvrgFbctB1
#print inData1.sumAvrgFbctB2
#print "len(inData1.avrgFbctB1)", len(inData1.avrgFbctB1)
#print inData1.avrgFbctB1
#print inData1.avrgFbctB2
#print "len(inData1.avrgFbctB1PerSP)", len(inData1.avrgFbctB1PerSP)
#print inData1.avrgFbctB1PerSP
#print inData1.avrgFbctB2PerSP

    inData1.GetLuminometerData(AnalysisDir + '/' + inputLuminometerData)
#inData1.PrintLuminometerData()

#print inData1.lum
#    print inData1.usedCollidingBunches
#for entry in inData1.lumiPerSP:
#    print entry
#for entry in inData1.lumiErrPerSP:
#    print entry

    Fill = inData1.fill
    
    inData = []
    inData.append(inData1)

# For the remaining scans:

    for i in range(1,len(inData1.scanNamesAll)):
        inDataNext = vdmInputData(i+1)
        inDataNext.GetScanInfo(AnalysisDir + '/' + inputScanFile)
        inDataNext.GetBeamCurrentsInfo(AnalysisDir + '/' + inputBeamCurrentFile)
        inDataNext.GetLuminometerData(AnalysisDir + '/' + inputLuminometerData)
        inData.append(inDataNext)



# Apply corrections
# Note that calibrating SumFBCT to DCCT is done in makeBeamCurrentFile.py if calibration flag in config file is set to true

    showAvailableCorrs()

    availableCorr = CorrectionManager.get_plugins(CorrectionManager.CorrectionProvider)

    print "The following corrections will be applied, in order: "
    for i, entry in enumerate(corrName):
        print "Corr #"+str(i+1)  + ": " +entry

    corrFull = ''

    for entry in corrName:

        print "Now applying correction: ", entry

# Check whether correction requested in config json actually exists

        key = entry+'_Corr'
        if key in availableCorr:
            corrector= availableCorr[entry+'_Corr']()        
        else:
            if not entry == "noCorr":
                print "Correction " + entry + " requested via json file does not exist, ignore."
            continue

# Read Corr config in here

        corrValueFile = AnalysisDir + '/corr/'+ entry + '_' + Fill +'.pkl' 
        if entry == "BeamBeam":
            corrValueFile = AnalysisDir + '/corr/'+ entry + '_' +Luminometer + '_' + Fill +'.pkl' 
            

#    for element in inData:
#        print "before current correction B1 ", element.scanNumber
#        for i, bx in enumerate(element.collidingBunches):
#            print ">>>", i, element.avrgFbctB1[i]

        corrector.doCorr(inData, corrValueFile)

#    for element in inData:
#        print "after current correction B1 ", element.scanNumber
#        for i, bx in enumerate(element.collidingBunches):
#            print ">>>", i, element.avrgFbctB1[i]

        corrFull = corrFull + '_' + entry 

# check if any corrections are to be applied at all, if yes, define corr description string accordingly
# if no use "noCorr"

    import os

# empty strings are false, so if no corrections are to be applied, use noCorr as corr descriptor
    if  not corrFull:
        corrFull = "_noCorr"


# Now fill graphs for all scans
# Counting of scans starts with 1

    graphsListAll = {'Scan_'+ str(n+1):{} for n in range(len(inData))} 

    for entry in inData:
    
        scanNumber = entry.scanNumber
        print "Now at Scan number ", scanNumber
        nBX = len(entry.collidingBunches)
        prefix = ''
        if 'X' in entry.scanName:
            prefix = str(scanNumber) +'_X_'
        if 'Y' in entry.scanName:
            prefix = str(scanNumber)+'_Y_'

    # convert for TGraph
        from array import array

        graphsList = {}

        for i, bx in enumerate(entry.usedCollidingBunches):
            coord = entry.spPerBX[bx]
            coorde = [0.0 for a in coord] 
            coord = array("d",coord)
            coorde = array("d", coorde)
            currProduct = [ a*b/1e22 for a,b in zip(entry.avrgFbctB1[i],entry.avrgFbctB2[i])]
            lumi = [a/b for a,b in zip(entry.lumi[i],currProduct)]
            lumie = [a/b for a,b in zip(entry.lumiErr[i],currProduct)]
            lumie = array("d",lumie)
            lumi = array("d",lumi)
            name = prefix +str(bx)
            graph = r.TGraphErrors(len(coord),coord,lumi,coorde,lumie)
            graph.SetName(name)
            graph.SetTitle(name)
            graphsList[bx] = graph
        
        graphsListAll['Scan_'+ str(scanNumber)]=graphsList 

    return corrFull, graphsListAll

    

if __name__ == '__main__':

    configFile = sys.argv[1]

    config=open(configFile)
    ConfigInfo = json.load(config)
    config.close()

    AnalysisDir = str(ConfigInfo['AnalysisDir'])
    Luminometer = str(ConfigInfo['Luminometer'])
    Fill = str(ConfigInfo['Fill'])
    OutputSubDir = str(ConfigInfo['OutputSubDir'])

    graphsListAll = {}

    corrFull, graphsListAll = doMakeGraphsFile(ConfigInfo)

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


