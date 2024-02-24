import ROOT as r
from lumi import *
from rawlumi import *

'''
TO DO: Check whether you have Olga's corrected error formula for the rates
'''


'''
The data in a VdM scan are recorded such that one scanpoint of typically 30s length is fully contained in one 
CMS_VdMScanHLXData_RAW_.....root file.
Should this change in the future, code needs to be adjusted.

'''


def extractHFlumiRates(HLXFileDir, PUCorr, run, colliding, scanpoints):

    table = []
    nBX = len(colliding)

    import os
    filelist = os.listdir(HLXFileDir)
# This will order filenames such that data_9999 comes before data_10000
    for element in filelist:
        if element.find("HLXData") < 0:
            del filelist[filelist.index(element)]
            filelist = sorted(filelist, key = lambda x: int(x.split("_")[5]))

    if len(filelist) == 0:
        print "Attention !!!"
        print "There are no HLX files in ", HLXFileDir

# hlx data file loop
    l=lumianalysis()
    for filename in filelist:

# sanity check that we got the data for the right run
        if not str(run) in str(filename):
            raise Exception('Run number in config file and run number in HLX file names do not match !')

        l.initVdM(HLXFileDir+"/"+filename)

# determine scan point
        ScanPoint = None
        for ls in range(l.nLS):
				
            l.loadLS(ls)
            ts = l.data.header.timestamp

            for point in scanpoints:

                if ts > point[3] and ts < point[4]:
                    ScanPoint = point
                    break
            if ScanPoint is not None: # multiple break required to exit the for loop
                break

#        print "ScanPoint", ScanPoint
        if ScanPoint is None : continue # no scan here

#        print "file, scanpoint", filename, point, ScanPoint

# calculate luminosities
        lumi_val = [0]*nBX
        lumi_err = [0]*nBX
        
        for ls in range(1,l.nLS-1): # omit the boundary lumi sections for safety
            l.loadLS(ls)

## All of HF
# colliding bunches
# includes poor man's afterglow correction
            for i in range (nBX):
                bx = colliding[i]
                lumi_val[i] += RawLumiOcc1Th1BX(l.data.occ,bx) - RawLumiOcc1Th1BX(l.data.occ,bx-1)
                lumi_err[i] += RawLumiOcc1Th1BXErr(l.data.occ,bx)**2 + RawLumiOcc1Th1BXErr(l.data.occ,bx-1)**2


# legacy correction, no longer applied, here for the record
# colliding bunches with beam-gas from non-colliding bunches corrected
#		        lumi_val = [a/float(l.nLS-2) - lumib1*b1 - lumib2*b2 for a,b1,b2 in zip(lumi_val,beam1,beam2)]
#		        lumi_err = [2*math.sqrt(a/((l.nLS-2)**2) + lumib1e*b1/((l.nLS-2)*nBXb1)**2 + lumib2e*b2/((l.nLS-2)*nBXb2)**2) for a,b1,b2 in zip(lumi_err,beam1,beam2)]

        import math

        lumi_val = [a/float(l.nLS-2) for a in lumi_val]
        lumi_err = [2*math.sqrt(a/((l.nLS-2)**2)) for a in lumi_err]


########### Pile-Up correction ###############################

        Alpha1 = PUCorr[0]
        Alpha2 = PUCorr[1]

        lumi_val = [a/(1 + Alpha1*a + Alpha2*a*a) for a in lumi_val]



# rearrange into dictionary with bx as key

        lumibx={}
        lumierrbx={}
#        print "ScanPoint", ScanPoint
#        print lumi_val
        for i in range(len(colliding)):
            index = colliding[i]
            lumibx[str(index)]=lumi_val[i]
            lumierrbx[str(index)]=lumi_err[i]

        row = ScanPoint[:3]
        row.append([lumibx, lumierrbx])
        table.append(row)

#    print table
    
    return table


def doMakeHFRateFile(ConfigInfo):
    
    HLXFileDir = str(ConfigInfo['HLXFileDir'])
    AnalysisDir = str(ConfigInfo['AnalysisDir'])
    InputScanFile = AnalysisDir + "/" + str(ConfigInfo['InputScanFile'])
    PUCorrStrg = ConfigInfo['PUCorr']

# to use eval in a  safe way, see http://lybniz2.sourceforge.net/safeeval.html:
# overkill in this context, but good to know in any case
    alpha1 = eval(PUCorrStrg[0],{"__builtins__":None},{})
    alpha2 = eval(PUCorrStrg[1],{"__builtins__":None},{})
    PUCorr = [alpha1, alpha2]

    import pickle
    with open(InputScanFile, 'rb') as f:
        scanInfo = pickle.load(f)

    Fill = scanInfo["Fill"]     
    Run = scanInfo["Run"]
    ScanNames = scanInfo["ScanNames"]     
    CollidingBunches = scanInfo["CollidingBunches"]

    csvtable = []
    csvtable.append(["ScanNumber, ScanNames, ScanPointNumber, HFRates per bx, HFRateErr per bx"])

    table = {}

    for i in range(len(ScanNames)):
        key = "Scan_" + str(i+1)
        scanpoints = scanInfo[key]
        table[key] = extractHFlumiRates(HLXFileDir, PUCorr, Run, CollidingBunches, scanpoints )
        csvtable.append([key] )

        helper = table[key]
        for entry in helper:
            csvtable.append(entry[:3])
            helper1= entry[3][0]
            helper1 = sorted(helper1.items(), key=lambda x: int(x[0])) 
            csvtable.append([helper1])
            helper2 = entry[3][1]
            helper2 = sorted(helper2.items(), key=lambda x: int(x[0]))
            csvtable.append([helper2])


    return table, csvtable


if __name__ == '__main__':

    import sys, json, pickle

    # read config file
    ConfigFile = sys.argv[1]

    Config=open(ConfigFile)
    ConfigInfo = json.load(Config)
    Config.close()

    Fill = str(ConfigInfo['Fill'])
    AnalysisDir = str(ConfigInfo['AnalysisDir'])
    OutputSubDir = AnalysisDir + "/" + str(ConfigInfo['OutputSubDir'])

    import csv
    csvfile = open(OutputSubDir+'/Rates_HF_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)

    table = {}
    csvtable = []

    table, csvtable = doMakeHFRateFile(ConfigInfo)

    with open(OutputSubDir+'/Rates_HF_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)
            
    writer.writerows(csvtable)

    csvfile.close()




