#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014
"""

import ROOT

li_runs = [ 329, 330]

li_names = ["PuleHeightTrack6_ROC0_All"]

di_histos = {}

for name in li_names:
    di_histos[name] = {}




for i_run, run in enumerate(li_runs):

    input_rootfile_name = "../plots/000"+str(run)+"/histos.root"
    f = ROOT.TFile.Open( input_rootfile_name )
        
    for i_name, name in enumerate(li_names):
    
        h = f.Get( name ).Clone()
        h.SetFile(0)
        di_histos[name][run]=h

for name in li_names:

    for i_run, run in enumerate(li_runs):    
        
        if i_run == 0:
            di_histos[name][i_run].Draw()
        else:
            di_histos[name][i_run].Draw("SAME")
