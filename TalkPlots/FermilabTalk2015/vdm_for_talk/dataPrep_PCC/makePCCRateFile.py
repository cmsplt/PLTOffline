import ROOT as r
from inputDataReader import *
import sys, json, pickle


def ClusterCount(data):

    """This is the method to be called on an event stored by the
    PixelLumiTupler to compute the number of pixel clusters to be used
    for the lumi analysis.
    """

    # Excludes the innermost barrel layer.
    num_clus = data.nPixelClustersB[1] + \
               data.nPixelClustersB[2] + \
               data.nPixelClustersF[0] + \
               data.nPixelClustersF[1]

    return num_clus



def addScanInfoToTree(chain, inData, intermediateFile):

# adapted from build_pixel_scan_tree.py

    out_file = r.TFile.Open(intermediateFile, "RECREATE")
    sub_tree = chain.CloneTree(0)

    r.gROOT.ProcessLine("struct ScanInfo {" \
                      "Int_t fScanIndex;" \
                      "Int_t fPointIndex;};")
    from ROOT import ScanInfo
    tmp_scan_info = ScanInfo()
    print tmp_scan_info

    tree_scan_info = r.TTree("tree_scan_info", "tree_scan_info")
    tree_scan_info.Branch("scan_info", tmp_scan_info,
                          "scanIndex/I:" \
                          "pointIndex/I")

    import time

    print "Building tree with scan info"
    num_missing = 0
    num_entries = chain.GetEntries()
    step = num_entries / 100
    time_start = time.time()

    for (index, data) in enumerate(chain):
        if index and not (index % step):
            time_now = time.time()
            time_per_evt = (time_now - time_start) / index
            time_left = time_per_evt * (num_entries - index)
            eta = time.localtime(time_now + time_left)
            print "  %3.0f%% (ETA: %s)" % \
                  (100. * index / num_entries, time.strftime("%H:%M:%S", eta))
        timestamp = data.timestamp

        i = None
        j = None
        scan_point = None

        for (i, in_data)  in enumerate(inData):
            time_window = [in_data.tStart[0], in_data.tStop[-1]]
            if (timestamp >= time_window[0]) and (timestamp <= time_window[1]):
                for (j, point) in enumerate(in_data.sp):
##                    print in_data.tStart[j], in_data.tStop[j]
##                    print j, point
                    if (timestamp >= in_data.tStart[j]) and (timestamp <= in_data.tStop[j]):
                        scan_point = point
                        break
                if scan_point:
                    break

        if not scan_point:
            i = -1
            j = -1

        tmp_scan_info.fScanIndex = i
        tmp_scan_info.fPointIndex = j

        if scan_point:
            sub_tree.Fill()
            tree_scan_info.Fill()
        else:
            num_missing += 1

    assert sub_tree.GetEntries() == tree_scan_info.GetEntries()

    if num_missing:
        print "WARNING Ignoring %d out of %d events (%.1f%%) " \
              "since they fall outside the scan point time windows" % \
              (num_missing, chain.GetEntries(),
               100. * num_missing / chain.GetEntries())

    sub_tree.AddFriend(tree_scan_info)
    tree_scan_info.AddFriend(sub_tree)

    print "Storing resulting trees in file combinedTree.root"
    sub_tree.Write()
    tree_scan_info.Write()
    out_file.Close()



def fillClusterCounts(pcc_tree, tree_scan_info, inData, PCC_BCID):

# sanity check: determine which bcid are actually present in the tree
# typically, PCC data is available only for 5 out of all colliding bx

    maxNoBCIDinLHC = 3564
    histBx = r.TH1F("histBx", "BCID actually pesent in pixel data tree", maxNoBCIDinLHC, 1., maxNoBCIDinLHC + 1.)
    pcc_tree.Draw("bxNo >> histBx")
    PCC_bcid_test = []
    for i in xrange(1, maxNoBCIDinLHC+1):
        if histBx.GetBinContent(i) > 0:
            PCC_bcid_test.append(i)
            
    print "In PCC tree find these bcid with PCC data: ", PCC_bcid_test
    for entry in PCC_bcid_test:
        if entry not in PCC_BCID:
            print "WARNING: Discrepancy between expected BCIDs (from config) and actually present BCIDs in PCC ntuple"

    clusters = []
    for (i, in_data)  in enumerate(inData):
        clusters.append([])
        for j in xrange(in_data.nSP):
            clusters[i].append({})
            for bx in PCC_BCID:
                clusters[i][j][bx] = []

    import time

    num_entries = pcc_tree.GetEntries()
    step = num_entries / 100
    time_start = time.time()

    for (i, data) in enumerate(pcc_tree):
        if i and not (i % step):
            time_now = time.time()
            time_per_evt = (time_now - time_start) / i
            time_left = time_per_evt * (num_entries - i)
            eta = time.localtime(time_now + time_left)
            print "  %3.0f%% (ETA: %s)" % \
                  (100. * i / num_entries, time.strftime("%H:%M:%S", eta))
        if data.scanIndex < 0:
            print "WARNING Found scanIndex < 0. Should this happen?"
        num_clus = ClusterCount(data)
        clusters[data.scanIndex][data.pointIndex][data.bxNo].append(num_clus)

    with open('clusterCounts.pkl', 'wb') as f:
        pickle.dump(clusters, f)

    return clusters



def extractPCCRates(clusters, PCC_BCID, in_data):
    
# use same format as was used for HF

    import numpy as np
    import math

    helpertable = []
    for j in xrange(in_data.nSP):
        row = [in_data.scanNumber, in_data.scanName, str(j+1)]
        lumibx = {}
        lumierrbx = {}
        for bx in PCC_BCID:
            data = clusters[in_data.scanNumber-1][j][bx]    
            mean = np.mean(data)
            std = np.std(data)
            mean_unc = std / math.sqrt(len(data))
            lumibx[str(bx)] = mean
            lumierrbx[str(bx)] = mean_unc
        row.append([lumibx, lumierrbx])
        helpertable.append(row)

    return helpertable



def doMakePCCRateFile(ConfigInfo):
    
    InputPCCFiles = ConfigInfo['InputPCCFiles']
    AnalysisDir = ConfigInfo['AnalysisDir']
    InputScanFile = AnalysisDir + "/" + ConfigInfo['InputScanFile']
    PCCTreeName = ConfigInfo['PCCTreeName']
    PCC_BCID = ConfigInfo['PCC_BCID']
    addScanInfo = False
    addScanInfo = ConfigInfo['addScanInfo']

    inData1 = vdmInputData(1)
    inData1.GetScanInfo(InputScanFile)
    inData = []
    inData.append(inData1)

    scanNames = inData1.scanNamesAll

# For the remaining scans:
    for i in range(1,len(scanNames)):
        inDataNext = vdmInputData(i+1)
        inDataNext.GetScanInfo(InputScanFile)
        inData.append(inDataNext)

    colliding = inData1.collidingBunches
    for entry in PCC_BCID:
        if entry not in colliding:
            print "Problem in makePCCRateFile_config.json, specified PCC_BCID not among colliding bunches specified in scan file, exit program"
            print "Exit program"
            sys.exit(1)

    chain = r.TChain(PCCTreeName)
    for name in InputPCCFiles:
        chain.Add(name)
    numFiles = chain.GetListOfFiles().GetEntries()

    print "Chain contains " + str(numFiles) + " files"
    print "Chain contains events", chain.GetEntries()

    intermediateFile = "combinedTree.root"
    if addScanInfo == True:
        addScanInfoToTree(chain, inData, intermediateFile)

# default for sub_tree_name is "treePixelLumi"
    sub_tree_name = PCCTreeName.split("/")[-1]
    scan_info_tree_name = "tree_scan_info"

# Read in the combined tree containing both the tree from the
# PixelLumiTupler and the VdM scan information.
    in_file = r.TFile.Open(intermediateFile, "READ")
    pcc_tree = in_file.Get(sub_tree_name)
    tree_scan_info = in_file.Get(scan_info_tree_name)

    if not pcc_tree or not tree_scan_info:
        print >> "ERROR Did not find both VdM trees - Exit program"
        sys.exit(1)

    clusters = []
    clusters = fillClusterCounts(pcc_tree, tree_scan_info, inData, PCC_BCID)

    with open('clusterCounts.pkl', 'rb') as f:
        clusters = pickle.load(f)

    csvtable = []
    csvtable.append(["ScanNumber, ScanName, ScanPointNumber, PCCRates per bx, PCCRateErr per bx"])

    table = {}

    for (i, in_data) in enumerate(inData):
#    for i in range(1):
        key = "Scan_" + str(i+1)
        print "NOW AT SCAN", key
        table[key] = extractPCCRates(clusters, PCC_BCID, in_data)
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

    # read config file
    ConfigFile = sys.argv[1]

    Config=open(ConfigFile)
    ConfigInfo = json.load(Config)
    Config.close()

    Fill = str(ConfigInfo['Fill'])
    AnalysisDir = str(ConfigInfo['AnalysisDir'])
    OutputSubDir = AnalysisDir + "/" + str(ConfigInfo['OutputSubDir'])

    import csv
    csvfile = open(OutputSubDir+'/Rates_PCC_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)

    table = {}
    csvtable = []

    table, csvtable = doMakePCCRateFile(ConfigInfo)

    with open(OutputSubDir+'/Rates_PCC_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)
            
    writer.writerows(csvtable)

    csvfile.close()




