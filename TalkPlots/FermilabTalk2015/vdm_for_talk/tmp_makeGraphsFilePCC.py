import ROOT as r
import pickle

in_file_name = 'forTmpStorage/PCC_Fill3503/pixel_vdm_graphs_210986.root'
Fill = 3503

PCC_BX=[ 1, 233, 892, 1124, 1356]
scanNumber = [1,2,3,4]
scanType = ['x','y','y','x']

in_file = r.TFile.Open(in_file_name, "READ")

out_file_name = 'forTmpStorage/PCC_Fill'+str(Fill)+'/'+'graphs_' + str(Fill) + '_noCorr.pkl'

graphsListAll = {'Scan_'+ str(n+1):{} for n in range(len(scanNumber))} 


for number, type in zip(scanNumber,scanType):
    graph = r.TGraphErrors()
    graphs={}
    for bx in PCC_BX:
        print "Now copying graph_pixel_scan" + str(number-1) + "_" + type + "_bx"+str(bx)
        graph = in_file.Get("graph_pixel_scan" + str(number-1) + "_" + type + "_bx"+str(bx))
        if type == 'x':
            name = str(number) + "_X_" + str(bx)
        if type == 'y':
            name = str(number) + "_Y_" + str(bx)
        graph.SetName(name)
        graph.SetTitle(name)
        graphs[bx] = graph
        
    graphsListAll['Scan_'+ str(number)]=graphs


out_file = open(out_file_name, 'wb')
pickle.dump(graphsListAll, out_file)
out_file.close()

in_file.Close()
