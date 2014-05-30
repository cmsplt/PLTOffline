#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014
"""

###############################
# Imports
###############################

import ROOT


###############################
# Select runs and plots
###############################

li_runs = [ 322, 325 , 327, 330, 333, 338, 340, 343, 347 ]
li_names  =  ["PulseHeightTrack6_ROC"+str(iroc)+"_All" for iroc in range(6)]
li_names +=  ["PulseHeightTrack6_ROC"+str(iroc)+"_NPix1" for iroc in range(6)]
li_names +=  ["PulseHeightTrack6_ROC"+str(iroc)+"_NPix2" for iroc in range(6)]
li_names +=  ["PulseHeightTrack6_ROC"+str(iroc)+"_NPix3Plus" for iroc in range(6)]


###############################
# Extract histograms from files
###############################

di_histos = {}

for name in li_names:
    di_histos[name] = {}

for i_run, run in enumerate(li_runs):

    print "Doing", run
    input_rootfile_name = "../plots/000"+str(run)+"/histos.root"
    f = ROOT.TFile.Open( input_rootfile_name )

    for name in li_names:

        h = f.Get( name ).Clone()
        h.SetDirectory(0)
        if h.Integral()>0:
            h.Scale(1./h.Integral())
        di_histos[name][run]=h


###############################
# Prepare pretty ROOT
###############################

c = ROOT.TCanvas("","",800,800)

legend_origin_x     = 0.75
legend_origin_y     = 0.3
legend_size_x       = 0.1
legend_size_y       = 0.045 * len(li_runs)


li_colors = [ROOT.kRed,      ROOT.kBlue+1,     ROOT.kBlack,
             ROOT.kOrange-1, ROOT.kViolet+1,   ROOT.kGreen+1,
             ROOT.kGray,     ROOT.kYellow,
         ]*10


###############################
# Combine histos into plot
###############################

for name in li_names:

    legend = ROOT.TLegend( legend_origin_x,
                       legend_origin_y,
                       legend_origin_x + legend_size_x,
                       legend_origin_y + legend_size_y )
    legend.SetBorderSize(1)
    legend.SetFillColor(0)
    legend.SetTextSize(0.04)
    legend.SetBorderSize(0)

    the_max = max( [di_histos[name][run].GetMaximum() for run in li_runs])
    the_max *= 1.1

    for i_run, run in enumerate(li_runs):

        di_histos[name][run].SetLineColor( li_colors[i_run])
        legend.AddEntry( di_histos[name][run], str(run), "L" )

        di_histos[name][run].SetMinimum(0)
        di_histos[name][run].SetMaximum(the_max)

        if i_run == 0:
            di_histos[name][run].Draw()
        else:
            di_histos[name][run].Draw("SAME")

    legend.Draw()
    c.Print("plots/"+name+".pdf")
