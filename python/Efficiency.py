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

telescope = 2

if telescope == 1:

    # Flux in kHz
    # (taken from Steve's Spreadsheet)
    di_runs = {
        322 :      1.6,
        325 :     12.9,
        327 :    130.6,
        330 :   1167.,
        333 :  20809.,
        338 :   1137.,
        340 :    125.4,
        343 :     10.8,
        347 :      1.4,
        348 :      1.5,
        350 :  20809.2,
        352 :  21387.3,
        }

    li_runs_up    = [322,325,327,330,333]
    li_runs_down  = [338,340,343,347,348]
    li_runs_final = [350,352]

elif telescope ==2:
    di_runs = {
        466 :     2.,
        467 :    18.2,
#        468 :   151.4,
        469 :  1313.9,
        470 :  9445.1,
        471 :  1269.4,
        472 :  1272.8,
        473 :   143.4,
        474 :    13.6,
        475 :     2.0,
        476 :  9398.8,
        478 : 22138.7
        }
    li_runs_up    = [466,467,469,470]
    li_runs_down  = [471, 472, 473, 474, 475]
    li_runs_final = [476, 478]


###############################
# Extract histograms from files
###############################


# Dictionary of lists
# dict_keys = run numbers
# list entries: one per ROC
nums_si   = {}
denoms_si = {}
eff_tr   = {}


for i_run, run in enumerate(di_runs):
    print "At: ", run

    nums_si[run]   = []
    denoms_si[run] = []
    eff_tr[run] = []

    input_rootfile_name = "../plots/000"+str(run)+"/histos.root"
    f = ROOT.TFile.Open( input_rootfile_name )

    for iroc in range(6):
        nums_si[run].append( f.Get("SiliconEfficiencyNumeratorROC"+str(iroc)).GetVal())
        denoms_si[run].append( f.Get("SiliconEfficiencyDenominatorROC"+str(iroc)).GetVal())

    for iroc in range(1,5):

        h =  f.Get("PlaneEfficiency_ROC"+str(iroc))
        cnt =0
        sum =0.
        for ibin_x in range(1, h.GetNbinsX()+2):
            for ibin_y in range(1, h.GetNbinsY()+2):
                if h.GetBinError(ibin_x, ibin_y)>0:
                    sum += h.GetBinContent(ibin_x, ibin_y)
                    cnt += 1

        if cnt>0:
            eff_tr[run].append( sum/cnt)
        else:
            eff_tr[run].append( -1)




###############################
# Prepare pretty ROOT
###############################

c = ROOT.TCanvas("","",800,800)

legend_origin_x     = 0.2
legend_origin_y     = 0.2
legend_size_x       = 0.1
legend_size_y       = 0.045 *4


li_colors = [ROOT.kRed,      ROOT.kBlue+1,     ROOT.kBlack,
             ROOT.kOrange-1, ROOT.kViolet+1,   ROOT.kGreen+1,
             ROOT.kGray,     ROOT.kYellow,
         ]*10

c.SetLogx(1)

for iroc in range(1,5):


    legend = ROOT.TLegend( legend_origin_x,
                       legend_origin_y,
                       legend_origin_x + legend_size_x,
                       legend_origin_y + legend_size_y )
    legend.SetBorderSize(1)
    legend.SetFillColor(0)
    legend.SetTextSize(0.04)
    legend.SetBorderSize(0)


    h = ROOT.TH2F("","", 100, 1, 25000, 100, 0, 1.1)
    h.GetXaxis().SetTitle( "Flux [kHz/cm^{2}]")
    h.GetYaxis().SetTitle( "#varepsilon")
    h.Draw()

    li_grs =[]
    for direction in ["up","down","final"]:

        if direction == "up":
            li_runs = li_runs_up
        elif direction == "down":
            li_runs = li_runs_down
        else:
            li_runs = li_runs_final

        gr_si = ROOT.TGraph( len(li_runs))
        gr_tr = ROOT.TGraph( len(li_runs))

        for irun, run in enumerate(sorted(li_runs)):
            gr_si.SetPoint(irun, di_runs[run],1.*nums_si[run][iroc]/denoms_si[run][iroc])
            gr_tr.SetPoint(irun, di_runs[run], eff_tr[run][iroc-1] )

        if direction == "up":
            legend.AddEntry( gr_si, "Without Tracking (Require Silicon Hits)","P")

        if direction == "up":
            legend.AddEntry( gr_tr, "Tracking (Up)","P")
        elif direction == "down":
            legend.AddEntry( gr_tr, "Tracking (Down)","P")
        elif direction == "final":
            legend.AddEntry( gr_tr, "Tracking (Final)","P")




        gr_si.SetMarkerStyle(2)

        # going up
        if direction == "up":
            gr_tr.SetMarkerStyle(22)
        #  down
        elif direction == "down":
            gr_tr.SetMarkerStyle(23)
        # final high flux
        else:
            gr_tr.SetMarkerStyle(21)

        gr_tr.SetMarkerSize(2)
        gr_tr.SetMarkerColor(ROOT.kRed)

        li_grs.append(gr_si)
        li_grs.append(gr_tr)

        gr_si.Draw("PSAME")
        gr_tr.Draw("PSAME")
    legend.Draw()

    c.Print("Eff_Telescope{0}_ROC{1}.png".format(telescope,iroc))
