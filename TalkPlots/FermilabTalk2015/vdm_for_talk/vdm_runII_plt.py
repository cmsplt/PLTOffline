import os
import tables
import pandas as pd
import numpy as np
from scipy import stats
import json
import math

import ROOT as r
import FitManager
import SG_Fit
import SGConst_Fit
import DG_Fit
import DGConst_Fit
import GSupGConst_Fit
import DG_2D_Fit

def getCurrents(scanpts):

#    datapath = "/Users/grothe/data/VdM/May2015/"
#    datapath = "/Users/grothe/data/VdM/May30_2015/"
#    datapath = "/Users/grothe/data/VdM/scanFill3804_15053018/"
    datapath = "/brildata/scanFill3804_15053018/"
    filelist = os.listdir(datapath+"central/")

    beam1data = [ [] for i in range(0, len(scanpts))]
    beam2data = [ [] for i in range(0, len(scanpts))]
    beamts = [ [] for i in range(0, len(scanpts))]
    beam1df = [ pd.DataFrame() for i in range(0, len(scanpts))]
    beam2df = [ pd.DataFrame() for i in range(0, len(scanpts))]
    filledBunches1=[]
    filledBunches2=[]
    collBunches=[]

    fbct1 = [ [] for i in range(0, len(scanpts))]
    fbct2 = [ [] for i in range(0, len(scanpts))]
    fbct1Err = [ [] for i in range(0, len(scanpts))]
    fbct2Err = [ [] for i in range(0, len(scanpts))]

    for idx, entry in enumerate(scanpts):
        # omit very first nibble because it may not be fully contained in VdM scan
        tw = '(timestampsec >' + str(entry[0]) + ') & (timestampsec <=' +  str(entry[1]) + ')'
        print "tw", tw

        for file in filelist:
            print file
            h5file = tables.open_file(datapath + "central/" + file, 'r')
            beamtable = h5file.root.beam
            bunchlist1 = [r['bxintensity1'] for r in beamtable.where(tw)] 
            bunchlist2 = [r['bxintensity2'] for r in beamtable.where(tw)]        

            if bunchlist1 and bunchlist2:
                filledBunches1 = list(np.nonzero(bunchlist1[0]))
                filledBunches2 = list(np.nonzero(bunchlist2[0]))
                helper = list(np.nonzero(bunchlist1[0]*bunchlist2[0]))
                collBunches = helper[0]

            beam1list = [r['bxintensity1'] for r in beamtable.where(tw)]
            beam2list = [r['bxintensity2'] for r in beamtable.where(tw)]
            beamtslist = [r['timestampsec'] for r in beamtable.where(tw)]

            beam1data[idx] = beam1data[idx] + beam1list
            beam2data[idx] = beam2data[idx] + beam2list
            beamts[idx] = beamts[idx] + beamtslist

            h5file.close()

        beam1df[idx] = pd.DataFrame(beam1data[idx], index = beamts[idx])
        beam2df[idx] = pd.DataFrame(beam2data[idx], index = beamts[idx])

        if beam1df[idx].empty or beam2df[idx].empty:
            print "Attention, beam current df empty because timestamp window not contained in file"
        else:
            print "scanpoint", idx, scanpts[idx][0], scanpts[idx][1]
            for bcid in collBunches:
                helper1 = beam1df[idx][bcid]
                helper2 = beam2df[idx][bcid]
                print helper1
                print "FBCT  mean for colliding bunch", bcid, helper1.mean(), helper2.mean() 
                fbct1[idx] = helper1.mean()
                fbct2[idx] = helper2.mean()
                fbct1Err[idx] = stats.sem(helper1)
                fbct2Err[idx] = stats.sem(helper2)
#                fbct1Err[idx] = 0.05*fbct1[idx]
#                fbct2Err[idx] = 0.05*fbct2[idx]

    return fbct1, fbct1Err, fbct2, fbct2Err


def getLumi(scanpts):

#    datapath = "/Users/grothe/data/VdM/May30_2015/"
#    datapath = "/Users/grothe/data/VdM/scanFill3804_15053018/"
    datapath = "/brildata/scanFill3804_15053018/"

#    filelist = os.listdir(datapath+"bcmf/")
    filelist = os.listdir(datapath+"plt/")
#    filelist = os.listdir(datapath+"central/")

    lumidata = [ [] for i in range(0, len(scanpts))]
    lumierrs = [ [] for i in range(0, len(scanpts))] #removeme
    lumits = [ [] for i in range(0, len(scanpts))]
    bxlumidata = [ [] for i in range(0, len(scanpts))]
    lumidf = [ pd.DataFrame() for i in range(0, len(scanpts))]
    bxlumidf = [ pd.DataFrame() for i in range(0, len(scanpts))]
    bxlumi = [ [] for i in range(0, len(scanpts))]
    bxlumiErr = [ [] for i in range(0, len(scanpts))]

    for idx, entry in enumerate(scanpts):
# omit very first nibble because it may not be fully contained in VdM scan
        tw = '(timestampsec >' + str(entry[0]) + ') & (timestampsec <=' +  str(entry[1]) + ')'
        print "tw", tw
	
        for file in filelist:
            print file
#            h5file = tables.open_file(datapath + "bcmf/" + file, 'r')
            h5file = tables.open_file(datapath + "plt/" + file, 'r')
#            h5file = tables.open_file(datapath + "central/" + file, 'r')
#            lumitable = h5file.root.bcm1flumi
            lumitable = h5file.root.pltagghist
#            lumitable = h5file.root.hflumi
#            lumilist = [r['avgraw'] for r in lumitable.where(tw)]
            lumitslist = [r['timestampsec'] for r in lumitable.where(tw)]
            lumits[idx] = lumits[idx] + lumitslist
            bxlumilist = [r['data'] for r in lumitable.where(tw)]
#	    print lumitable
            bxlumidata[idx] =  bxlumidata[idx] + bxlumilist
	    lumilist = []
            errslist = []
	    for bunchlumi in bxlumilist:
                totcounts = 0 # removeme
		agglumi = 0
		print bunchlumi
		for bcid in range (0,3563):
			counts = bunchlumi[bcid]
                        totcounts += counts # removeme
			zeros = (16384. - counts) / 16384.
			mu = -1. * math.log(zeros)
			agglumi += mu
		lumilist.append(agglumi)
                if (totcounts > 0):
                    errslist.append(agglumi/math.sqrt(totcounts))
                else:
                    errslist.append(0)
#	    print lumilist
	    lumidata[idx]= lumidata[idx] + lumilist
            lumierrs[idx] = lumierrs[idx] + lumilist
            h5file.close()

        lumidf[idx] = pd.DataFrame(lumidata[idx],index = lumits[idx])
#	print lumidf
        bxlumidf[idx] = pd.DataFrame(bxlumidata[idx], index = lumits[idx])

        print "scanpoint", idx, scanpts[idx][0], scanpts[idx][1]
        print "lumidf avgraw mean", lumidf[idx].mean()
#        bcid = 2 
    #   print "bxlumidf col ", bcid, bxlumidf[idx][bcid]
#        helper = bxlumidf[bcid][idx]
#        print "bxraw mean for col", bcid, helper.mean() 

# attention, this fill had only one bcid colliding

        bxlumi[idx] = float(lumidf[idx].mean())
        bxlumiErr[idx] = float(stats.sem(lumidf[idx]))
        
#        bxlumiErr[idx] = 0.05*bxlumi[idx]
        print "Scanpoint "+str(idx)+": lumi="+str(bxlumi[idx])
        print "lumiErr="+str(bxlumiErr[idx])
        rms = math.sqrt(sum(n*n for n in lumierrs[idx])/len(lumierrs[idx]))
        print "lumiErr the other way="+str(rms)
#        bxlumiErr[idx] = rms

    return bxlumi, bxlumiErr


if __name__ == '__main__':


# X scan

#    scanpts = [[1432200862, 1432200876], [1432200892, 1432200905], [1432200919, 1432200934], [1432200949, 1432200963], [1432200978, 1432200991], [1432201006, 1432201019], [1432201033, 1432201048]]
#    scanpts = [[1433010597, 1433010615], [1433010645, 1433010665], [1433010694, 1433010713], [1433010744, 1433010763], [1433010792, 1433010811], [1433010841, 1433010860],[1433010891, 1433010908], [1433010939, 1433010958], [1433010988, 1433011014]]
    scanpts = [[1433010599,1433010614],[1433010646,1433010664],[1433010696,1433010712],[1433010745,1433010761],[1433010793,1433010809],[1433010843,1433010859],[1433010892,1433010907],[1433010940,1433010956],[1433010990,1433011006]]

    displacement = [-0.06243, -0.04683, -0.03123, -0.01563, -3e-05, 0.01557, 0.03117, 0.04678, 0.06238]

    print "X scan"
    fbct1, fbct1Err, fbct2, fbct2Err = getCurrents(scanpts)
    print "fbct1", fbct1
    print "fbct1Err", fbct1Err
    print "fbct2", fbct2
    print "fbct2Err", fbct2Err
    bxlumi, bxlumiErr = getLumi(scanpts)
    print "bxlumi", bxlumi
    print "bxlumiErr", bxlumiErr

    from array import array

    name = "1_X_1"

    coord = displacement
    coorde = [0.0 for a in coord] 
    coord = array("d",coord)
    coorde = array("d", coorde)
    currProduct = [ a*b/1e22 for a,b in zip(fbct1, fbct2)]
    lumi = [a/b for a,b in zip(bxlumi,currProduct)]
    lumie = [a/b for a,b in zip(bxlumiErr,currProduct)]
    lumie = array("d",lumie)
    lumi = array("d",lumi)

    graphX = r.TGraphErrors(len(coord),coord,lumi,coorde,lumie)
    graphX.SetName(name)
    graphX.SetTitle(name)

# Y scan

#    scanpts = [[1433011166, 1433011185], [1433011216, 1433011235], [1433011265, 1433011284], [1433011315, 1433011334], [1433011364, 1433011383], [1433011414, 1433011433], [1433011462, 1433011481], [1433011512, 1433011530], [1433011560, 1433011579]]
    scanpts = [[1433011168,1433011184],[1433011217,1433011233],[1433011267,1433011283],[1433011316,1433011332],[1433011366,1433011382],[1433011415,1433011431],[1433011463,1433011479],[1433011513,1433011529],[1433011561,1433011577]]

    displacement = [-0.06243, -0.04682, -0.03123, -0.01563, -3e-05, 0.01557, 0.03117, 0.04677, 0.06237]

    print "Y scan"
    fbct1, fbct1Err, fbct2, fbct2Err = getCurrents(scanpts)
    print "fbct1", fbct1
    print "fbct1Err", fbct1Err
    print "fbct2", fbct2
    print "fbct2Err", fbct2Err
    bxlumi, bxlumiErr = getLumi(scanpts)
    print "bxlumi", bxlumi
    print "bxlumiErr", bxlumiErr

    name = "1_Y_1"

    coord = displacement
    coorde = [0.0 for a in coord] 
    coord = array("d",coord)
    coorde = array("d", coorde)
    currProduct = [ a*b/1e22 for a,b in zip(fbct1, fbct2)]
    lumi = [a/b for a,b in zip(bxlumi,currProduct)]
    lumie = [a/b for a,b in zip(bxlumiErr,currProduct)]
    lumie = array("d",lumie)
    lumi = array("d",lumi)

    graphY = r.TGraphErrors(len(coord),coord,lumi,coorde,lumie)
    graphY.SetName(name)
    graphY.SetTitle(name)

    availableFits = FitManager.get_plugins(FitManager.FitProvider)
    fitter = availableFits['SGConst_Fit']()

    FitConfigFile = "fits/SGConst_Config.json"
    FitConfig=open(FitConfigFile)
    FitConfigInfo = json.load(FitConfig)
    FitConfig.close()

#    result = fitter.doFit(graphX, FitConfigInfo)
#    functions = result[0]
#    canvas = fitter.doPlot(graphX, functions, "3805")

#    print "X scan"
#    for entry in fitter.table:
#        print entry

    result = fitter.doFit(graphY, FitConfigInfo)
    functions = result[0]
    canvas = fitter.doPlot(graphY, functions, "3805")

    print "Y scan"
    for entry in fitter.table:
        print entry
