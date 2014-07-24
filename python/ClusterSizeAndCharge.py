#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014.
Extract the cluster size, total charge in radius and charge of second highest hit as a function of flux.
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
charge = {}
cluster_size = {}
second_charge = {}

# Loop over runs
for i_run, run in enumerate(di_runs):

    print "At: ", run

    charge[run] = []
    cluster_size[run] = []
    second_charge[run] = []

    input_rootfile_name = "../plots/000" + str(run) + "/histos.root"
    f = ROOT.TFile.Open(input_rootfile_name)

    for iroc in range(1, 5):
        # Get the cluster size
        h_cs = f.Get("ClusterSize_ROC" + str(iroc))
        cluster_size[run].append(h_cs.Project3D("z").GetMean())

        # Get the charge of the second in the cluster
        h_se = f.Get("SecondCharge_ROC" + str(iroc))
        second_charge[run].append(h_se.GetMean())

        # Use the charge within a 2-pixel radius
        h_charge = f.Get("Charge2_ROC" + str(iroc))
        charge[run].append(h_charge.Project3D("z").GetMean())

    # End of loop over ROCs
# End loop over runs


###############################
# Exceptions: Variable Error
###############################

class VariableError(Exception):
    def __str__(self):
        return "Do not know how to handle this plotting variable."


###############################
# make_plots
###############################

def make_plots():

    # Prepare pretty ROOT
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetPadLeftMargin(0.16)
    ROOT.gStyle.SetPadRightMargin(0.05)
    ROOT.gStyle.SetPadTopMargin(0.05)
    ROOT.gROOT.ForceStyle()

    c = ROOT.TCanvas("", "", 800, 800)

    c.SetLogx(1)

    for plot_var in ["cluster_size", "charge", "second_charge"]:

        if plot_var == "cluster_size":
            di_values = cluster_size
            legend_origin_x = 0.2
            legend_origin_y = 0.2
        elif plot_var == "charge":
            di_values = charge
            legend_origin_x = 0.2
            legend_origin_y = 0.8
        elif plot_var == "second_charge":
            di_values = second_charge
            legend_origin_x = 0.2
            legend_origin_y = 0.5
        else:
            raise VariableError(plot_var)

        legend_size_x = 0.1
        legend_size_y = 0.045 * 3

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

            if plot_var == "cluster_size":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 3)
                h.GetYaxis().SetTitle("Mean Cluster Size")
            elif plot_var == "charge":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 50000)
                h.GetYaxis().SetTitle("Total Charge within 2-pixel radius")
            elif plot_var == "second_charge":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 15000)
                h.GetYaxis().SetTitle("Charge of second hit in cluster")
            else:
                raise VariableError

            h.GetXaxis().SetTitle("Flux [kHz/cm^{2}]")
            h.GetYaxis().SetTitleOffset(2.)

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
                    gr.SetPoint(irun, di_runs[run], di_values[run][i_roc-1])  # i_roc-1 since we have no data for ROC0

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
                gr.SetMarkerSize(1.5)
                gr.Draw("PSAME")
            legend.Draw()

            if plot_var == "cluster_size":
                outfile_name = "ClusterSize_Telescope{0}_ROC{1}".format(telescope, i_roc)
            elif plot_var == "charge":
                outfile_name = "Charge_Telescope{0}_ROC{1}".format(telescope, i_roc)
            elif plot_var == "second_charge":
                outfile_name = "SecondCharge_Telescope{0}_ROC{1}".format(telescope, i_roc)
            else:
                raise VariableError

            c.Print(outfile_name + ".png")
            # End loop over ROCs
# End of Make Plots

make_plots()
