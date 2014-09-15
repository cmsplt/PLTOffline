#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014.
Extract the cluster size, leading and second highest charge of  hit as a function of flux.
"""

# ##############################
# Imports
# ##############################

import sys

import ROOT

import RunInfos


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
    li_runs_down_extended = RunInfos.di_li_runs_down_extended[telescope]
    li_runs_final = RunInfos.di_li_runs_final[telescope]
    di_rocs = RunInfos.di_di_rocs[telescope]
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
sum_charge = {}
cluster_size = {}
cluster_size_fraction_1 = {}
cluster_size_fraction_12 = {}
second_charge = {}

# Loop over runs
for i_run, run in enumerate(di_runs):

    print "At: ", run

    charge[run] = []
    sum_charge[run] = []
    cluster_size[run] = []
    cluster_size_fraction_1[run] = []
    cluster_size_fraction_12[run] = []
    second_charge[run] = []

    input_rootfile_name = "../plots/000" + str(run) + "/histos.root"
    f = ROOT.TFile.Open(input_rootfile_name)

    for i_roc in range(1, 5):
        # Get the mean cluster size
        h_cs = f.Get("ClusterSize_ROC" + str(i_roc))
        h_cs_proj = h_cs.Project3D("z")
        cluster_size[run].append(h_cs_proj.GetMean())

        # Get the the fraction of events where the cluster size:
        #   is equal to 1
        #   is equal to 1 or 2
        bin_1 = h_cs_proj.FindBin(1)
        bin_2 = h_cs_proj.FindBin(2)
        cluster_size_fraction_1[run].append(1. * h_cs_proj.GetBinContent(bin_1) / h_cs_proj.Integral())
        cluster_size_fraction_12[run].append(1. * h_cs_proj.Integral(bin_1, bin_2) / h_cs_proj.Integral())

        # Get the charge of the second in the cluster
        h_se = f.Get("2ndCharge4_ROC" + str(i_roc))
        second_charge[run].append(h_se.Project3D("z").GetMean())

        # Use the leading charge within a 2-pixel radius
        h_charge = f.Get("1stCharge4_ROC" + str(i_roc))
        charge[run].append(h_charge.Project3D("z").GetMean())

        # Use the sum of  charges within a 2-pixel radius
        h_sumcharge = f.Get("SumCharge4_ROC" + str(i_roc))
        sum_charge[run].append(h_sumcharge.Project3D("z").GetMean())

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
    ROOT.gStyle.SetPadLeftMargin(0.23)
    ROOT.gStyle.SetPadBottomMargin(0.15)
    ROOT.gStyle.SetPadRightMargin(0.05)
    ROOT.gStyle.SetPadTopMargin(0.05)
    ROOT.gROOT.ForceStyle()

    c = ROOT.TCanvas("", "", 800, 800)

    c.SetLogx(1)
    c.SetGrid(1, 1)

    for plot_var in ["cluster_size", 
                     #"cluster_size_fraction_1", 
                     #"cluster_size_fraction_12", 
                     "charge", 
                     "sum_charge", 
                     #"second_charge"
    ]:

        if plot_var == "cluster_size":
            di_values = cluster_size
            legend_origin_x = 0.3
            legend_origin_y = 0.2
        elif plot_var == "cluster_size_fraction_1":
            di_values = cluster_size_fraction_1
            legend_origin_x = 0.3
            legend_origin_y = 0.2
        elif plot_var == "cluster_size_fraction_12":
            di_values = cluster_size_fraction_12
            legend_origin_x = 0.3
            legend_origin_y = 0.2
        elif plot_var == "charge":
            di_values = charge
            legend_origin_x = 0.3
            legend_origin_y = 0.8
        elif plot_var == "sum_charge":
            di_values = sum_charge
            legend_origin_x = 0.3
            legend_origin_y = 0.8
        elif plot_var == "second_charge":
            di_values = second_charge
            legend_origin_x = 0.3
            legend_origin_y = 0.65
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
            legend.SetTextSize(0.06)
            legend.SetBorderSize(0)

            if plot_var == "cluster_size":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 3)
                h.GetYaxis().SetTitle("Mean Cluster Size")
            elif plot_var == "cluster_size_fraction_1":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 1.)
                h.GetYaxis().SetTitle("Fraction of size == 1 Clusters")
            elif plot_var == "cluster_size_fraction_12":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 1.)
                h.GetYaxis().SetTitle("Fraction of 1 <= size <= 2 Clusters")
            elif plot_var == "charge":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 30000)
                h.GetYaxis().SetTitle("Leading charge within 4-pixel radius")
            elif plot_var == "sum_charge":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 30000)
                h.GetYaxis().SetTitle("Sum of charge within 4-pixel radius")
            elif plot_var == "second_charge":
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 15000)
                h.GetYaxis().SetTitle("Charge of second hit in cluster")
            else:
                raise VariableError

            h.GetXaxis().SetTitle("Flux [kHz/cm^{2}]")
            #

            h.GetXaxis().SetTitleSize(0.06)
            h.GetYaxis().SetTitleSize(0.06)
            h.GetXaxis().SetLabelSize(0.06)
            h.GetYaxis().SetLabelSize(0.06)

            if plot_var ==  "cluster_size":
                h.GetYaxis().SetTitleOffset(1.5)
            else:
                h.GetYaxis().SetTitleOffset(1.85)

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
                    legend.AddEntry(gr, "Increasing Flux", "P")
                elif direction == "down":
                    legend.AddEntry(gr, "Decreasing Flux", "P")
                elif direction == "final":
                    legend.AddEntry(gr, "Highest Flux", "P")

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
                    outfile_name = "ClusterSize"
                elif plot_var == "cluster_size_fraction_1":
                    outfile_name = "ClusterSizeFraction1"
                elif plot_var == "cluster_size_fraction_12":
                    outfile_name = "ClusterSizeFraction12"
                elif plot_var == "charge":
                    outfile_name = "LeadingCharge"
                elif plot_var == "sum_charge":
                    outfile_name = "SumCharge"
                elif plot_var == "second_charge":
                    outfile_name = "SecondCharge"
                else:
                    raise VariableError
        
            outfile_name += "_Telescope{0}_{1}_{2}".format(telescope, di_rocs[i_roc], "all")

            c.Print(outfile_name + ".png")

# End of Make Plots

make_plots()
