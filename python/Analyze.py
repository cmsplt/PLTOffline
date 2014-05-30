#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014
"""

import ROOT

li_runs = [ 328, 330]

li_names = ["PulseHeightTrack6_ROC0_All"]

di_histos = {}

for name in li_names:
    di_histos[name] = {}




for i_run, run in enumerate(li_runs):

    print "Doing", run
    input_rootfile_name = "../plots/plots/000"+str(run)+"/histos.root"
    f = ROOT.TFile.Open( input_rootfile_name )

    for i_name, name in enumerate(li_names):

        h = f.Get( name ).Clone()
        di_histos[name][run]=h

for name in li_names:

    for i_run, run in enumerate(li_runs):

        if i_run == 0:
            di_histos[name][i_run].Draw()
        else:
            di_histos[name][i_run].Draw("SAME")
