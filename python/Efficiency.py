#!/usr/bin/env python

"""
Analyze multiple runs from PSI testbeam in May 2014
"""

###############################
# Imports
###############################

import ROOT
ROOT.gStyle.SetOptStat(0)


###############################
# Select runs
###############################

# Flux in kHz
# (taken from Steve's Spreadsheet)
di_runs = {
    322 : 1.6,
    325 : 12.9,
    327 : 130.6,
    330 : 1167,
    333 : 20809,
    338 : 1137,
    340 : 125.4,
    343 : 10.8,
    347 : 1.4,
    348 : 1.5,
    350 : 20809.2,
    352 : 21387.3,
    }


###############################
# Extract histograms from files
###############################


# Dictionary of lists
# dict_keys = run numbers
# list entries: one per ROC
nums_si   = {}
denoms_si = {}


for i_run, run in enumerate(di_runs):

    nums_si[run]   = []
    denoms_si[run] = []

    input_rootfile_name = "../plots/000"+str(run)+"/histos.root"
    f = ROOT.TFile.Open( input_rootfile_name )

    for iroc in range(6):
        nums_si[run].append( f.Get("SiliconEfficiencyNumeratorROC"+str(iroc)).GetVal())
        denoms_si[run].append( f.Get("SiliconEfficiencyDenominatorROC"+str(iroc)).GetVal())



###############################
# Prepare pretty ROOT
###############################

c = ROOT.TCanvas("","",800,800)

legend_origin_x     = 0.75
legend_origin_y     = 0.3
legend_size_x       = 0.1
legend_size_y       = 0.045 * len(di_runs)


li_colors = [ROOT.kRed,      ROOT.kBlue+1,     ROOT.kBlack,
             ROOT.kOrange-1, ROOT.kViolet+1,   ROOT.kGreen+1,
             ROOT.kGray,     ROOT.kYellow,
         ]*10

c.SetLogx(1)

for iroc in range(1,5):

    h = ROOT.TH2F("","", 100, 1, 25000, 100, 0, 1.1)
    gr = ROOT.TGraph( len(di_runs))

    for irun, run in enumerate(sorted(di_runs.keys())):
        gr.SetPoint(irun, di_runs[run],1.*nums_si[run][iroc]/denoms_si[run][iroc])

    gr.SetMarkerStyle(2)
    h.Draw()
    gr.Draw("PSAME")

    c.Print("Eff_ROC"+str(iroc)+".png")
