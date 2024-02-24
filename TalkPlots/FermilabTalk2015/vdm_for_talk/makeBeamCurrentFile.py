import ROOT

def mkdata(dipfile, runNumber):

    import os
    vdmdir = os.getenv("VDMPATH") 
    ROOT.gROOT.LoadMacro(vdmdir+'/dict/DIP_dict.C+')

    points = []    # to collect per-scanpoint times and bunch intensities

    # get TTree with recorded HF data
    f = ROOT.TFile(dipfile)
    tree = f.Get('VdMDIPCombined')

    # loop over tree entries and read all lumi sections
    for i in range(tree.GetEntriesFast()):
        if tree.GetEntry(i) <= 0:
            raise Exception('TTree::GetEntry() failed')

        # a shorthand notation
        dip = tree.VdMDIPCombined

        if dip.runNumber != runNumber:
            print('WARN: dip.runNumber != runNumber, entry skipped')
            continue

        # take stable beams
        beamMode = dip.beamMode.strip('\x00') # remove zero characters at the end
        if beamMode != 'STABLE BEAMS':
            print('WARN: dip.beamMode = "{0}", entry skipped'.format(beamMode))
            continue

        # take lumi sections with recorded data
        if dip.VdMScan.RecordDataFlag == 0:
            print('WARN: dip.VdMScan.RecordDataFlag = 0, entry skipped')
            continue

        beam_info1 = ROOT.BEAM_INFO()
        beam_info2 = ROOT.BEAM_INFO()
        ROOT.getBeam(dip, beam_info1, 0)
        ROOT.getBeam(dip, beam_info2, 1)

        beam1 = list(beam_info1.averageBunchIntensities)
        beam2 = list(beam_info2.averageBunchIntensities)

        dcct1 = beam_info1.averageBeamIntensity
        dcct2 = beam_info2.averageBeamIntensity

        # verify that FBCT data was written indeed
        if sum(beam1) < 1e9 or sum(beam2) < 1e9:
            print('WARN: sum of averageBunchIntensities < 1e9, entry skipped')
            continue


        point = {}
        point.setdefault('times', []).append(dip.timestamp)
        point.setdefault('beam1', []).append(beam1)
        point.setdefault('beam2', []).append(beam2)
        point.setdefault('dcct1', []).append(dcct1)
        point.setdefault('dcct2', []).append(dcct2)

        points.append(point)

    return points


def checkFBCTcalib(table, CalibrateFBCTtoDCCT):

    h_ratioB1 = ROOT.TGraph()
    h_ratioB1.SetMarkerStyle(8)
    h_ratioB1.SetMarkerSize(0.4)
    h_ratioB1.SetTitle("SumFBCT/DCCT for B1, for scan "+str(table[0][1]))
    h_ratioB1.GetXaxis().SetTitle("Scan point number")
    h_ratioB1.GetYaxis().SetTitle("SumFBCT(active bunches)/DCCT")

    h_ratioB2 = ROOT.TGraph()
    h_ratioB2.SetMarkerStyle(8)
    h_ratioB2.SetMarkerSize(0.4)
    h_ratioB2.SetTitle("SumFBCT/DCCT for B2, for scan "+str(table[0][1]))
    h_ratioB2.GetXaxis().SetTitle("Scan point number")
    h_ratioB2.GetYaxis().SetTitle("SumFBCT(active bunches)/DCCT")

    for idx, entry in enumerate(table):
        h_ratioB1.SetPoint(idx, entry[2], entry[5]/entry[3])
        h_ratioB2.SetPoint(idx, entry[2], entry[6]/entry[4])


    h_ratioB1.Fit("pol0")
    h_ratioB2.Fit("pol0")

    fB1 = ROOT.TF1()
    fB2 = ROOT.TF1()
    fB1 = h_ratioB1.GetFunction("pol0")
    fB2 = h_ratioB2.GetFunction("pol0")

    corrB1 = fB1.GetParameter(0)
    corrB2 = fB2.GetParameter(0)

    if CalibrateFBCTtoDCCT == True:

        print "Applying FBCT to DCCT calibration"
        for idx, entry in enumerate(table):
            old1 = entry[7]
#            entry[7] = entry[5]/entry[3]*old1
            entry[7] = corrB1*old1
            old2 =  entry[8]
#            entry[8] = entry[6]/entry[4]*old2
            entry[8] = corrB2*old1

    return [h_ratioB1, h_ratioB2]


def doMakeBeamCurrentFile(ConfigInfo):

    import csv, pickle

    AnalysisDir = str(ConfigInfo['AnalysisDir'])
    InputDIPFile = str(ConfigInfo['InputDIPFile'])
    InputScanFile = './' + AnalysisDir + '/' + str(ConfigInfo['InputScanFile'])
    OutputSubDir = str(ConfigInfo['OutputSubDir'])

    outpath = './' + AnalysisDir + '/' + OutputSubDir 

    CalibrateFBCTtoDCCT = False
    CalibrateFBCTtoDCCT = str(ConfigInfo['CalibrateFBCTtoDCCT'])

    with open(InputScanFile, 'rb') as f:
        scanInfo = pickle.load(f)

    points = mkdata(InputDIPFile, int(scanInfo["Run"]))

    Fill = scanInfo["Fill"]     
    ScanNames = scanInfo["ScanNames"]     

    dcct1 = [[] for i in range(len(ScanNames))]
    dcct2 = [[] for i in range(len(ScanNames))]

# Associate individual measurements (points) per small LS with one scanpoint; 
# since small LS is about 1.5 s and scanpoint is typically measured over 30s, there should be about 20 points per scanpoint

    for point in points:
        for i in range(len(ScanNames)):
            key = "Scan_" + str(i+1)
            scanpoints = scanInfo[key]
            if len(scanpoints) == 0:
                print "Problem with scan point info, appears to be empty, please check contents of ", InputScanFile
                import sys
                sys.exit(1)
            for j in range(len(scanpoints)):
                if scanpoints[j][3] <= point['times'][0] <= scanpoints[j][4]:
                    scanpoints[j].append(point)
                    
# Now determine current averages

    maskB1 = [0.0 for i in range(3564)]
    for entry in scanInfo['FilledBunchesB1']:
        maskB1[int(entry)-1] = 1.0

    maskB2 = [0.0 for i in range(3564)]
    for entry in scanInfo['FilledBunchesB2']:
        maskB2[int(entry)-1] = 1.0

    table = {}

    for i in range(len(ScanNames)):
        key = "Scan_" + str(i+1)

        scanpoints = scanInfo[key]
        table["Scan_" + str(i+1)]=[]
        for j in range(len(scanpoints)):
            sumdcct1 = 0.0
            sumdcct2 = 0.0
            fbctB1 = [0.0 for k in range(3564)]
            fbctB2 = [0.0 for k in range(3564)]
            numAssociatedPoints = len(scanpoints[j][6:])
            for k in range(6,len(scanpoints[j])):
                sumdcct1 += (scanpoints[j][k]['dcct1'][0])
                sumdcct2 += (scanpoints[j][k]['dcct2'][0])
# zero fbct values in nominally empty bunches (noise)
                fbct1_helper = scanpoints[j][k]['beam1'][0]
                fbct1_helper = [a*b for a,b in zip(fbct1_helper,maskB1)]
                fbct2_helper = scanpoints[j][k]['beam2'][0]
                fbct2_helper = [a*b for a,b in zip(fbct2_helper,maskB2)]
# now do sums
                fbctB1 = [a+b for a,b in zip(fbctB1, fbct1_helper)] 
                fbctB2 = [a+b for a,b in zip(fbctB2, fbct2_helper)] 
# average
            avrgdcct1 = sumdcct1/numAssociatedPoints
            avrgdcct2 = sumdcct2/numAssociatedPoints
            avrgfbctB1 = [a/numAssociatedPoints for a in fbctB1]
            avrgfbctB2 = [a/numAssociatedPoints for a in fbctB2]
# now rearrange fbct lists into dictionnary with bx as key
            avrgfbct1={}
            avrgfbct2={}
            for entry in scanInfo['FilledBunchesB1']:
                avrgfbct1[str(entry)]=avrgfbctB1[entry-1]
            for entry in scanInfo['FilledBunchesB2']:
                avrgfbct2[str(entry)]=avrgfbctB2[entry-1]
            row = [i+1, str(ScanNames[i]), j+1, avrgdcct1, avrgdcct2, sum(avrgfbctB1), sum(avrgfbctB2), avrgfbct1, avrgfbct2]
            table["Scan_" + str(i+1)].append(row)


    canvas = ROOT.TCanvas()

    ROOT.gStyle.SetOptFit(111)
    ROOT.gStyle.SetOptStat(0)

    h_ratioB1 = ROOT.TGraph()
    h_ratioB2 = ROOT.TGraph()

    outpdf = outpath+'/checkFBCTcalib_'+str(Fill)+'.pdf'
    for i in range(len(ScanNames)):
        key = "Scan_" + str(i+1)
        [h_ratioB1, h_ratioB2] = checkFBCTcalib(table[key], CalibrateFBCTtoDCCT)
        h_ratioB1.Draw("AP")
        canvas.SaveAs(outpdf + '(')
        h_ratioB2.Draw("AP")
        canvas.SaveAs(outpdf + '(')

    canvas.SaveAs(outpdf + ']')

    csvtable = []
    csvtable.append(["ScanNumber, ScanNames, ScanPointNumber, avrgdcct1, avrgdcct2, sum(avrgfbctB1), sum(avrgfbctB2), fbct1 per Bx, fbct2 per BX"])

    for i in range(len(ScanNames)):
        key = "Scan_" + str(i+1)
        csvtable.append([str(key)] )
        for j in range(len(scanpoints)):
            row =  table[str(key)][j]
            csvtable.append(row)

    return table, csvtable
        

if __name__ == '__main__':

    import pickle, csv, sys, json

    ConfigFile = sys.argv[1]

    Config=open(ConfigFile)
    ConfigInfo = json.load(Config)
    Config.close()

    AnalysisDir = str(ConfigInfo["AnalysisDir"])
    OutputSubDir = str(ConfigInfo["OutputSubDir"])

    outpath = './' + AnalysisDir + '/' + OutputSubDir 

    InputScanFile = './' + AnalysisDir + '/' + str(ConfigInfo['InputScanFile'])
    with open(InputScanFile, 'rb') as f:
        scanInfo = pickle.load(f)

    Fill = scanInfo["Fill"]     

    table = {}
    csvtable = []

    table, csvtable = doMakeBeamCurrentFile(ConfigInfo)
    
    csvfile = open(outpath+'/BeamCurrents_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(outpath+'/BeamCurrents_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)
            
