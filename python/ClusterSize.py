#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014.
Extract the cluster size as a function of flux.
"""

# ##############################
# Imports
# ##############################

import sys

import ROOT

import RunInfos

###############################
# Configuration
###############################

telescope = 1

try:
    di_runs = RunInfos.di_di_runs[telescope]
    li_runs_up = RunInfos.di_li_runs_up[telescope]
    li_runs_down = RunInfos.di_li_runs_down[telescope]
    li_runs_final = RunInfos.di_li_runs_final[telescope]
except KeyError:
    print "Invalid telescope! Exiting.."
    sys.exit()


###############################
# Extract histograms from files
###############################

# Dictionary of lists
# dict_keys = run numbers
# list entries: one per ROC
cluster_size = {}

# Loop over runs
for i_run, run in enumerate(di_runs):

    print "At: ", run

    cluster_size[run] = []

    input_rootfile_name = "../plots/000" + str(run) + "/histos.root"
    f = ROOT.TFile.Open(input_rootfile_name)

    for iroc in range(1, 5):
        h_cs = f.Get("ClusterSize_ROC" + str(iroc))
        cluster_size[run].append(h_cs.Project3D("z").GetMean())
    # End of loop over ROCs
# End loop over runs


###############################
# make_plots
###############################

def make_plots():

    # Prepare pretty ROOT
    ROOT.gStyle.SetOptStat(0)

    c = ROOT.TCanvas("", "", 800, 800)

    legend_origin_x = 0.2
    legend_origin_y = 0.2
    legend_size_x = 0.1
    legend_size_y = 0.045 * 3

    c.SetLogx(1)

    # Loop over ROCs
    for i_roc in range(1, 5):

        # Prepare Legend
        legend = ROOT.TLegend(legend_origin_x,
                              legend_origin_y,
                              legend_origin_x + legend_size_x,
                              legend_origin_y + legend_size_y)
        legend.SetBorderSize(1)
        legend.SetFillColor(0)
        legend.SetTextSize(0.04)
        legend.SetBorderSize(0)

        # Prepare 'background' TH2.
        h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 3)
        h.GetXaxis().SetTitle("Flux [kHz/cm^{2}]")
        h.GetYaxis().SetTitle("Mean Cluster Size")
        h.Draw()

        # Prepare efficiency TGraphs
        li_grs = []
        for direction in ["up", "down", "final"]:

            # Choose runs to use
            if direction == "up":
                li_runs = li_runs_up
            elif direction == "down":
                li_runs = li_runs_down
            else:
                li_runs = li_runs_final

            # Initialize TGraph objects. For tracking we need more points for multiple event slices
            gr = ROOT.TGraph(len(li_runs))

            # Loop over runs
            for irun, run in enumerate(sorted(li_runs)):
                gr.SetPoint(irun, di_runs[run], cluster_size[run][i_roc-1])  # i_roc-1 since we don't have data for ROC0

            # Make things look nice:
            # Legend Entries

            if direction == "up":
                legend.AddEntry(gr, "Up", "P")
            elif direction == "down":
                legend.AddEntry(gr, "Down", "P")
            elif direction == "final":
                legend.AddEntry(gr, "Final", "P")

            # Markers
            # going up
            if direction == "up":
                gr.SetMarkerStyle(22)
                gr.SetMarkerColor(ROOT.kRed)
            #  down
            elif direction == "down":
                gr.SetMarkerStyle(23)
                gr.SetMarkerColor(ROOT.kBlue)
            # final high flux
            else:
                gr.SetMarkerStyle(21)
                gr.SetMarkerColor(ROOT.kGreen)

            # Protect graphs from autodelete
            li_grs.append(gr)

            gr.Draw("PSAME")
        legend.Draw()

        outfile_name = "ClusterSize_Telescope{0}_ROC{1}".format(telescope, i_roc)
        c.Print(outfile_name + ".png")
        # End loop over ROCs
# End of Make Plots

make_plots()
