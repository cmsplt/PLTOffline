#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014.
Extract the efficiency as a function of flux.
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

nslices = 5

try:
    di_runs = RunInfos.di_di_runs[telescope]
    li_runs_up = RunInfos.di_li_runs_up[telescope]
    li_runs_down = RunInfos.di_li_runs_down[telescope]
    li_runs_final = RunInfos.di_li_runs_final[telescope]
except KeyError:
    print "Invalid telescope! Exiting.."
    sys.exit()


###############################
# Helper function:
# eff_and_unc
###############################

def eff_and_unc(h_num, h_denom):
    """ Calculate efficiency and uncertainty
    :param h_num: Numerator TH2
    :param h_denom: Denominator TH2
    :return: Efficiency, Uncertainty Error Up, Uncertainty Error Low (as a tuple floats)
    """
    num = 0
    denom = 0

    for ibin_x in range(1, h_denom.GetNbinsX() + 2):
        for ibin_y in range(1, h_denom.GetNbinsY() + 2):
            num += h_num.GetBinContent(ibin_x, ibin_y)
            denom += h_denom.GetBinContent(ibin_x, ibin_y)

    # Temporary Numerator Histogram
    h_tmp_num = ROOT.TH1D("", "", 1, 0, 1)
    h_tmp_num.SetBinContent(1, num)

    # Temporary Denominator Histogram
    h_tmp_denom = ROOT.TH1D("", "", 1, 0, 1)
    h_tmp_denom.SetBinContent(1, denom)

    # Use TEfficiency to calculate efficiency and uncertainty
    # Should be the frequentist Clopper-Pearson interval per default
    teff = ROOT.TEfficiency(h_tmp_num, h_tmp_denom)

    if denom > 0:
        e = teff.GetEfficiency(1)
        u_up = teff.GetEfficiencyErrorUp(1)
        u_low = teff.GetEfficiencyErrorLow(1)
    else:
        e = -1
        u_up = -1
        u_low = -1

    return e, u_up, u_low


###############################
# Extract histograms from files
###############################

# Dictionary of lists
# dict_keys = run numbers
# list entries: one per ROC
nums_si = {}
denoms_si = {}
eff_tr = {}
unc_up_tr = {}
unc_low_tr = {}

# More detailed efficiency - split in multiple slices per run
# Format
# { run:[ [roc1-slice0, roc1-slice1, roc1-slice2], [roc2-slice0,...]]}
eff_tr_slice = {}
unc_up_tr_slice = {}
unc_low_tr_slice = {}

# Loop over runs
for i_run, run in enumerate(di_runs):

    print "At: ", run

    nums_si[run] = []
    denoms_si[run] = []
    eff_tr[run] = []
    unc_up_tr[run] = []
    unc_low_tr[run] = []
    eff_tr_slice[run] = []
    unc_up_tr_slice[run] = []
    unc_low_tr_slice[run] = []

    input_rootfile_name = "../plots/000" + str(run) + "/histos.root"
    f = ROOT.TFile.Open(input_rootfile_name)

    # Silicon-based efficiency
    for iroc in range(6):
        nums_si[run].append(f.Get("SiliconEfficiencyNumeratorROC" + str(iroc)).GetVal())
        denoms_si[run].append(f.Get("SiliconEfficiencyDenominatorROC" + str(iroc)).GetVal())

    # Tracking-based efficiency
    for iroc in range(1, 5):

        # First get the full-run efficiency
        eff, unc_up, unc_low = eff_and_unc(f.Get("PlaneEfficiency_ROC" + str(iroc)),
                                           f.Get("TracksPassing_ROC" + str(iroc)))
        eff_tr[run].append(eff)
        unc_up_tr[run].append(unc_up)
        unc_low_tr[run].append(unc_low)

        # Efficiency per event-slice
        li_effs = []
        li_uncs_up = []
        li_uncs_low = []
        for islice in range(nslices):
            eff_slice, unc_up_slice, unc_low_slice = eff_and_unc(
                f.Get("Numerator_ROC{0}_slice{1}".format(iroc, islice)),
                f.Get("Denominator_ROC{0}_slice{1}".format(iroc, islice)))

            li_effs.append(eff_slice)
            li_uncs_up.append(unc_up_slice)
            li_uncs_low.append(unc_low_slice)
        # End loop over slices

        eff_tr_slice[run].append(li_effs)
        unc_up_tr_slice[run].append(li_uncs_up)
        unc_low_tr_slice[run].append(li_uncs_low)
# End loop over runs


###############################
# make_plots
###############################

def make_plots(add_si, do_zoom, do_slice):
    # Prepare pretty ROOT

    ROOT.gStyle.SetOptStat(0)

    c = ROOT.TCanvas("", "", 800, 800)

    legend_origin_x = 0.2
    legend_origin_y = 0.2
    legend_size_x = 0.1
    if add_si:
        legend_size_y = 0.045 * 4
    else:
        legend_size_y = 0.045 * 3

    c.SetLogx(1)

    for direction in ["up", "down", "final"]:
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
            if do_zoom:
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 1.05)
            else:
                h = ROOT.TH2F("", "", 100, 1, 40000, 100, 0., 1.1)
            h.GetXaxis().SetTitle("Flux [kHz/cm^{2}]")
            h.GetYaxis().SetTitle("#varepsilon")
            h.Draw()

            # Prepare efficiency TGraphs
            li_grs = []

            # Choose runs to use
            if direction == "up":
                li_runs = li_runs_up
            elif direction == "down":
                li_runs = li_runs_down
            else:
                li_runs = li_runs_final

            # Initialize TGraph objects. For tracking we need more points for multiple event slices
            gr_si = ROOT.TGraph(len(li_runs))
            if do_slice:
                gr_tr = ROOT.TGraphAsymmErrors(len(li_runs) * nslices)
            else:
                gr_tr = ROOT.TGraphAsymmErrors(len(li_runs))

            # Loop over runs
            for irun, run in enumerate(sorted(li_runs)):

                gr_si.SetPoint(irun, di_runs[run], 1. * nums_si[run][i_roc] / denoms_si[run][i_roc])

                # Get efficiency/uncertainty from the dictionaries and set TGraph's points
                if do_slice:
                    for islice in range(nslices):
                        gr_tr.SetPoint(irun * nslices + islice,
                                       di_runs[run] * (1 + 0.1 * (islice - 2)),
                                       eff_tr_slice[run][i_roc - 1][islice])
                        gr_tr.SetPointEXhigh(irun * nslices + islice, 0)
                        gr_tr.SetPointEXlow(irun * nslices + islice, 0)
                        gr_tr.SetPointEYhigh(irun * nslices + islice, unc_up_tr_slice[run][i_roc - 1][islice])
                        gr_tr.SetPointEYlow(irun * nslices + islice, unc_low_tr_slice[run][i_roc - 1][islice])
                else:
                    gr_tr.SetPoint(irun, di_runs[run], eff_tr[run][i_roc - 1])
                    gr_tr.SetPointEXhigh(irun, 0)
                    gr_tr.SetPointEXlow(irun, 0)
                    gr_tr.SetPointEYhigh(irun, unc_up_tr[run][i_roc - 1])
                    gr_tr.SetPointEYlow(irun, unc_low_tr[run][i_roc - 1])
            # end loop over runs

            # Make things look nice:
            # Legend Entries
            if direction == "up" and add_si:
                legend.AddEntry(gr_si, "Without Tracking (Require Silicon Hits)", "P")

            if direction == "up":
                legend.AddEntry(gr_tr, "Increasing Flux", "P")
            elif direction == "down":
                legend.AddEntry(gr_tr, "Decreasing Flux", "P")
            elif direction == "final":
                legend.AddEntry(gr_tr, "Final", "P")

            # Markers
            gr_si.SetMarkerStyle(2)
            gr_tr.SetMarkerSize(1.5)
            # going up
            if direction == "up":
                gr_tr.SetMarkerStyle(22)
                gr_tr.SetMarkerColor(ROOT.kRed)
            #  down
            elif direction == "down":
                gr_tr.SetMarkerStyle(23)
                gr_tr.SetMarkerColor(ROOT.kBlue)
            # final high flux
            else:
                gr_tr.SetMarkerStyle(21)
                gr_tr.SetMarkerColor(ROOT.kGreen)

            # Protect graphs from autodelete
            li_grs.append(gr_si)
            li_grs.append(gr_tr)

            if add_si:
                gr_si.Draw("PSAME")
            gr_tr.Draw("PSAME")
            legend.Draw()

            outfile_name = "Eff_Telescope{0}_ROC{1}_{2}".format(telescope, i_roc, direction)
            if do_slice:
                outfile_name += "_sliced"
            c.Print(outfile_name + ".png")
        # End loop over ROCs
    # End of loop over directions
# End of Make Plots

make_plots(add_si=False, do_zoom=False, do_slice=False)
make_plots(add_si=False, do_zoom=True, do_slice=True)
