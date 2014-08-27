#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014
"""

###############################
# Imports
###############################

from math import log10, floor
import sys

import ROOT

import RunInfos



###############################
# Helpers
###############################

def round_to_2(x):
    return round(x, -int(floor(log10(x)))+1)


###############################
# Read telescope from the commandline
###############################

if not len(sys.argv)==2:
    print "Wrong number of input arguments!"
    print "Usage: {0} telescopeID".format(sys.argv[0])
    sys.exit()
else:
    telescope = int(sys.argv[1])

###############################
# Configuration
###############################

try:
    di_runs = RunInfos.di_di_runs[telescope]
    li_runs_up = RunInfos.di_li_runs_up[telescope]
    li_runs_down = RunInfos.di_li_runs_down[telescope]
    li_runs_final = RunInfos.di_li_runs_final[telescope]
except KeyError:
    print "Invalid telescope! Exiting.."
    sys.exit()


li_names = []
#li_names +=  ["PulseHeightTrack6_ROC"+str(iroc)+"_All" for iroc in range(6)]
#li_names +=  ["PulseHeightTrack6_ROC"+str(iroc)+"_NPix1" for iroc in range(6)]
#li_names +=  ["PulseHeightTrack6_ROC"+str(iroc)+"_NPix2" for iroc in range(6)]
#li_names +=  ["PulseHeightTrack6_ROC"+str(iroc)+"_NPix3Plus" for iroc in range(6)]


li_names +=  ["1stCharge4_ROC{0}_z".format(iroc) for iroc in range(1,5)]

###############################
# Extract histograms from files
###############################

di_histos = {}

for name in li_names:
    di_histos[name] = {}

for i_run, run in enumerate(li_runs_up + li_runs_down + li_runs_final):

    print "Doing", run
    input_rootfile_name = "../plots/000"+str(run)+"/histos.root"
    f = ROOT.TFile.Open( input_rootfile_name )

    for name in li_names:
        h = f.Get( name ).Clone()
        h.SetDirectory(0)
        if h.Integral()>0:
            h.Scale(100./h.Integral())
        h.SetTitle("")
        h.GetXaxis().SetNdivisions(505)
        if "Charge" in name:
            h.GetXaxis().SetTitle("Charge [electrons]")
        h.GetYaxis().SetTitle("Fraction of Clusters [%]")
        di_histos[name][run]=h


###############################
# Prepare pretty ROOT
###############################

c = ROOT.TCanvas("","",800,800)



li_colors = [ROOT.kRed,      ROOT.kBlue+1,     ROOT.kBlack,
             ROOT.kOrange-1, ROOT.kViolet+1,   ROOT.kGreen+1,
             ROOT.kGray,     ROOT.kYellow,
         ]*10


def GetMaximumExceptBin(h, ibin=1):

  if (h.GetMaximumBin() == ibin):
    return h.GetMaximum(h.GetMaximum())
  else:
    return h.GetMaximum()


###############################
# Combine histos into plot
###############################

for name in li_names:
    for direction in ["up", "down", "final"]:

        # Choose runs to use
        if direction == "up":
            li_runs = li_runs_up
        elif direction == "down":
            li_runs = li_runs_down
        else:
            li_runs = li_runs_final

        legend_origin_x     = 0.6
        legend_origin_y     = 0.5
        legend_size_x       = 0.1
        legend_size_y       = 0.045 * len(li_runs)

        legend = ROOT.TLegend( legend_origin_x,
                           legend_origin_y,
                           legend_origin_x + legend_size_x,
                           legend_origin_y + legend_size_y )
        legend.SetBorderSize(1)
        legend.SetFillColor(0)
        legend.SetTextSize(0.04)
        legend.SetBorderSize(0)


        the_max = max( [GetMaximumExceptBin(di_histos[name][run])for run in li_runs])
        the_max *= 1.1

        for i_run, run in enumerate(li_runs):

            di_histos[name][run].SetLineColor( li_colors[i_run])
            rate = int(round((round_to_2(di_runs[run]))))
            legend.AddEntry( di_histos[name][run], "{0} kHz".format(rate), "L" )

            di_histos[name][run].SetMinimum(0)
            di_histos[name][run].SetMaximum(the_max)

            if i_run == 0:
                di_histos[name][run].Draw()
            else:
                di_histos[name][run].Draw("SAME")

        legend.Draw()
        c.Print("plots/{0}_{1}.pdf".format(name, direction))
