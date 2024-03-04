import ROOT as r
import os.path
import sys
import json
import pickle

def doMakeGraphs2D(ConfigInfo):

    Fill = ConfigInfo['Fill']
    Scanpairs = ConfigInfo['Scanpairs']
    AnalysisDir = ConfigInfo['AnalysisDir']
    Luminometer = ConfigInfo['Luminometer']
    InOutSubDir = ConfigInfo['InOutSubDir']
    corrName = ConfigInfo['Corr']

    inputDir = AnalysisDir + '/' + Luminometer + '/' + InOutSubDir + '/'

    corrFull = ''
    for entry in corrName:
        corrFull = corrFull + '_' + entry 


    GraphsFile1D = inputDir + 'graphs_' + Fill + corrFull + '.pkl'
    if not(os.path.isfile(GraphsFile1D)):
        print "File with 1D graphs doesn't exist, needs to be available before 2D graphs can be filled"
        sys.exit(1)

    graphsListAll = {}
    graphs2DListAll = {}

    infile = open(GraphsFile1D, 'rb')
    graphsListAll = pickle.load(infile)
    infile.close()

    for entry in Scanpairs:
        graphsX = graphsListAll["Scan_" + str(entry[0])]
        graphsY = graphsListAll["Scan_" + str(entry[1])]

        if (len(graphsX) != len(graphsY)):
            print "Invalid X-Y scan pair, # of X-scan graphs, ' + len(graphsX) +', is different from # of Y-scan graphs, '+ len(graphsY)"
            sys.exit(1)

#        print Scanpairs
#        print graphsX
#        print graphsY

        graphs2D = {}

        for key in graphsX.keys():

            print key
            ndpX = graphsX[key].GetN()
            dpXx = graphsX[key].GetX()
            dpEXx = graphsX[key].GetEX()
            dpXy = graphsX[key].GetY()
            dpEXy = graphsX[key].GetEY()
    
            ndpY = graphsY[key].GetN()
            dpYx = graphsY[key].GetX()
            dpEYx = graphsY[key].GetEX()
            dpYy = graphsY[key].GetY()
            dpEYy = graphsY[key].GetEY()

            listX = [dpXx[i] for i in range(ndpX)]
            helperlist =[0.0 for i in range(ndpY)]
            listX.extend(helperlist)
            listEX = [dpEXx[i] for i in range(ndpX)]
            listEX.extend(helperlist)

            listY =[0.0 for i in range(ndpX)]
            helperlist = [dpYx[i] for i in range(ndpY)]
            listY.extend(helperlist)
            listEY =[0.0 for i in range(ndpX)]
            helperlist = [dpEYx[i] for i in range(ndpY)]
            listEY.extend(helperlist)
        
            listZ =[dpXy[i] for i in range(ndpX)]
            helperlist = [dpYy[i] for i in range(ndpY)]
            listZ.extend(helperlist)
            listEZ =[dpEXy[i] for i in range(ndpX)]
            helperlist = [dpEYy[i] for i in range(ndpY)]
            listEZ.extend(helperlist)
    
            import array
            aX = array.array("d",listX)
            aY = array.array("d",listY)
            aZ = array.array("d",listZ)
            aEX = array.array("d",listEX)
            aEY = array.array("d",listEY)
            aEZ = array.array("d",listEZ)
    
            name2D = "2D_"+str(entry[0]) + "_" + str(entry[1]) + "_" + str(key)
            g2D = r.TGraph2DErrors(ndpX+ndpY, aX, aY, aZ, aEX, aEY, aEZ, "")
            g2D.SetName(name2D)
            g2D.SetTitle(name2D)
            
            graphs2D[key] = g2D
    
        graphs2DListAll["Scanpair_" + str(entry[0]) + "_" + str(entry[1])] = graphs2D

    return corrFull, graphs2DListAll


if __name__ == '__main__':

    ConfigFile = sys.argv[1]

    Config=open(ConfigFile)
    ConfigInfo = json.load(Config)
    Config.close()

    Fill = ConfigInfo['Fill']
    AnalysisDir = ConfigInfo['AnalysisDir']
    Luminometer = ConfigInfo['Luminometer']
    InOutSubDir = ConfigInfo['InOutSubDir']

    graphs2DListAll = {}

    corrFull, graphs2DListAll = doMakeGraphs2D(ConfigInfo)

    outputDir = AnalysisDir + '/' + Luminometer + '/' + InOutSubDir + '/'
    outFileName = 'graphs2D_' + Fill + corrFull

#2D graph file should be called graphs2D_<n>_<corrFull>.pkl
    GraphFile2D = outputDir + outFileName + '.pkl'

    file2D = open(GraphFile2D, 'wb')
    pickle.dump(graphs2DListAll, file2D)
    file2D.close()
    

# save TGraphs in a ROOT file
    rfile = r.TFile(outputDir + outFileName + '.root',"recreate")

    for key in sorted(graphs2DListAll.iterkeys()):
        graphs2D = graphs2DListAll[key]
        for key_bx in sorted(graphs2D.iterkeys()):
            graphs2D[key_bx].Write()

    rfile.Write()
    rfile.Close()


