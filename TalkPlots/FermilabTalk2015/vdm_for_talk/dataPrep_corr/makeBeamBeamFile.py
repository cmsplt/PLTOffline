import sys, json, pickle, csv
from inputDataReader import *

# calls python script written by T. Pieloni and W. Kozanecki
# needs shared library errffor.so
# to make shared library:
# f2py --opt="-O3" -c -m errffor --fcompiler=gfortran  --link-lapack_opt *.f
# This works under anaconda python, with ROOT version linked against anaconda python, see env_anaconda.py



def doMakeBeamBeamFile(ConfigInfo):

    import numpy as np
    from numpy import sqrt, pi, exp,sign

    from BB import BB

    AnalysisDir = ConfigInfo['AnalysisDir']
    Luminometer = ConfigInfo['Luminometer']
    InputScanFile = ConfigInfo['InputScanFile']
    InputBeamCurrentFile = ConfigInfo['InputBeamCurrentFile']
    InputCapSigmaFile = AnalysisDir + '/' + Luminometer + '/results/' + ConfigInfo['InputCapSigmaFile']
    Scanpairs = ConfigInfo['Scanpairs']

    inData1 = vdmInputData(1)
    inData1.GetScanInfo(AnalysisDir + '/'+ InputScanFile)
    inData1.GetBeamCurrentsInfo(AnalysisDir + '/' + InputBeamCurrentFile)

    Fill = inData1.fill
    
    inData = []
    inData.append(inData1)

# For the remaining scans:

    for i in range(1,len(inData1.scanNamesAll)):
        inDataNext = vdmInputData(i+1)
        inDataNext.GetScanInfo(AnalysisDir + '/' + InputScanFile)
        inDataNext.GetBeamCurrentsInfo(AnalysisDir + '/' + InputBeamCurrentFile)
        inData.append(inDataNext)

    from collections import defaultdict
    CapSigma = defaultdict(dict)
    
    from fitResultReader import *
    fitResult = fitResultReader(InputCapSigmaFile)
    CapSigma = fitResult.getFitParam("CapSigma")

## input to BB:
## CapSigma's must be in microns
    for entry in CapSigma:
        for param in CapSigma[entry]:
            value = CapSigma[entry][param]
            value =  value * 1000
            CapSigma[entry][param] = value 


# Separations must be in mm
# LHC tunes (units: 2*pi)
# taken from BBscan.py macro provided by T. Pieloni and W. Kozanecki
    tunex = 64.31
    tuney = 59.32

# for output
    table = {}
    csvtable = []
    csvtable.append(["ScanNumber, ScanNames, ScanPointNumber, corr_Xcoord in mm per BX, corr_Ycoord in mm per BX"])


# figure out which scans to consider as pair

    for entry in Scanpairs:
        scanx = entry[0]
        scany = entry[1]

# Counting of scans starts with 1, index in inData starts with 0

        inDataX = inData[scanx-1]
        inDataY = inData[scany-1]

        keyx = "Scan_"+ str(scanx)
        keyy = "Scan_"+ str(scany)

        table[keyx] = []
        table[keyy] = []

        CsigxList = CapSigma[keyx]
        CsigyList = CapSigma[keyy] 

# IP beta functions beta* of deflected bunch (units: m)
        betax = float(inDataX.betaStar)
        betax = betax/1000.
        betay = float(inDataY.betaStar)
        betay = betay/1000.

# <<<<<>>>>> For X scan:

        sepxList = inDataX.displacement
        sepyList = [0.0 for i in range(len(sepxList))]

# for output
        orbitCorrX_xcoord = [{} for i in range(len(sepxList))]
        orbitCorrX_ycoord = [{} for i in range(len(sepyList))]

# Attention: In the pPb and Pbp scans, one uses the proton-equivalent beam energy
# beam energy in eV
        Ep1 = inDataX.energyB1
        Ep1 = float(Ep1)*1e9
        Ep2 = inDataX.energyB2
        Ep2 = float(Ep2)*1e9

# Np = intensity of opposite beam in number of protons per bunch
        NpList1 = inDataX.avrgFbctB2PerSP
        NpList2 = inDataX.avrgFbctB1PerSP
        
        for bx in inDataX.collidingBunches:
            try:
                Csigx = CsigxList[str(bx)]
                Csigy = CsigyList[str(bx)]
                for i in range(len(sepxList)):
                    sepx= sepxList[i]
                    sepy= sepyList[i]
                    Np1 = NpList1[i][str(bx)]               
                    Np2 = NpList2[i][str(bx)]

# >>> Effect of beam2 on beam1:
# sepx and sepy are in mm, deltaOrbitX and deltaOrbitY are in microns
                    deflectionXB1, deflectionYB1, deltaOrbitXB1, deltaOrbitYB1 = BB(Csigx,Csigy,sepx,sepy,betax,betay,tunex,tuney,Np1, Ep1)

# >>> Effect of beam1 on beam2:
                    deflectionXB2, deflectionYB2, deltaOrbitXB2, deltaOrbitYB2 = BB(Csigx,Csigy,sepx,sepy,betax,betay,tunex,tuney,Np2, Ep2)

                    orbitCorrX_xcoord[i][str(bx)] = (deltaOrbitXB1+deltaOrbitXB2)*1e-3
                    orbitCorrX_ycoord[i][str(bx)] = 0.0

            except KeyError:
                print "From makeBeambeamFile.py: bx = ", bx, "does not exist in CsigxList, CsigyList" 

        csvtable.append([keyx])
        for i in range(len(sepxList)):
            row = [scanx, "X"+str(scanx), i+1, orbitCorrX_xcoord[i], orbitCorrX_ycoord[i]]
            table[keyx].append(row)
            csvtable.append(row)
                        


# <<<<<>>>>> For Y scan:

        sepyList = inDataY.displacement

        sepxList = [0.0 for i in range(len(sepxList))]

# for output
        orbitCorrY_xcoord = [{} for i in range(len(sepxList))]
        orbitCorrY_ycoord = [{} for i in range(len(sepyList))]

# Attention: In the pPb and Pbp scans, one uses the proton-equivalent beam energy
# beam energy in eV
        Ep1 = inDataY.energyB1
        Ep1 = float(Ep1)*1e9
        Ep2 = inDataY.energyB2
        Ep2 = float(Ep2)*1e9

# Np = intensity of opposite beam in number of protons per bunch
        NpList1 = inDataY.avrgFbctB2PerSP
        NpList2 = inDataY.avrgFbctB1PerSP

        for bx in inDataY.collidingBunches:
            try:
                Csigx = CsigxList[str(bx)]
                Csigy = CsigyList[str(bx)]
                for i in range(len(sepyList)):
                    sepx= sepxList[i]
                    sepy= sepyList[i]
                    Np1 = NpList1[i][str(bx)]               
                    Np2 = NpList2[i][str(bx)]

#                print ">>>"
#                print Csigx, Csigy, sepx, sepy, betax,betay, tunex, tuney, Np2, Ep2

# >>> Effect of beam2 on beam1:
# sepx and sepy are in mm, deltaOrbitX and deltaOrbitY are in microns
                    deflectionXB1, deflectionYB1, deltaOrbitXB1, deltaOrbitYB1 = BB(Csigx,Csigy,sepx,sepy,betax,betay,tunex,tuney,Np1, Ep1)

# >>> Effect of beam1 on beam2:
                    deflectionXB2, deflectionYB2, deltaOrbitXB2, deltaOrbitYB2 = BB(Csigx,Csigy,sepx,sepy,betax,betay,tunex,tuney,Np2, Ep2)

                    orbitCorrY_ycoord[i][str(bx)] = (deltaOrbitYB1+deltaOrbitYB2)*1e-3
                    orbitCorrY_xcoord[i][str(bx)] = 0.0

            except KeyError:
                print "From makeBeambeamFile.py: bx = ", bx, "does not exist in CsigxList, CsigyList" 

        csvtable.append([keyy])
        for i in range(len(sepyList)):
            row = [scany, "Y"+str(scany), i+1, orbitCorrY_xcoord[i], orbitCorrY_ycoord[i]]
            table[keyy].append(row)
            csvtable.append(row)

    return table, csvtable



if __name__ == '__main__':

# read config file
    ConfigFile = sys.argv[1]

    Config=open(ConfigFile)
    ConfigInfo = json.load(Config)
    Config.close()

    Fill = ConfigInfo["Fill"]
    AnalysisDir = ConfigInfo["AnalysisDir"]
    Luminometer = ConfigInfo["Luminometer"]
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


 
