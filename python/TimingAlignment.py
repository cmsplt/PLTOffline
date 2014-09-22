#!/usr/bin/env python

"""
Study timing between pixel and pad detectors.
"""

###############################
# Imports
###############################

import os
import sys

import ROOT


###############################
# Init ROOT
###############################

c = ROOT.TCanvas("","",800,800)

ROOT.gStyle.SetPadLeftMargin(0.15)
ROOT.gStyle.SetPadBottomMargin(0.15)
ROOT.gROOT.ForceStyle()


###############################
# Configuration
###############################

run = 16
do_test    = True
find_align = False
plot_drift = True

di_offsets = {6:  0.0003045,
              12:-0.00030472,
              16:-0.000167307,
}

di_slopes = {6:  2.155918e-06,
             12: 1.95599961e-06,
             16: 1.821545e-06,
}


# Branchnames:
# Event Numbers
br_n_pad = "n"
br_n_pixel = "ievent"
# Time
br_t_pad = "time_stamp" # [Unix Time]
br_t_pixel = "time"     # clock-ticks (25 ns spacing)

try:
    os.mkdir("run_{0}".format(run))
except:
    pass

if do_test:
    test_string = "_test"
else:
    test_string = ""


###############################
# Helper Function
# pixel_to_pad_time
###############################

def pixel_to_pad_time( pixel_now, pixel_0, pad_now, pad_0):
    
    # How many ticks have passed since first pixel time-stamp
    delta_pixel = pixel_now - pixel_0

    # Convert ticks to seconds (1 tick ~ 25 ns)            
    delta_second = delta_pixel * 25e-9 + di_offsets[run]

    # Add time difference (in seconds) to initial pad time
    return pad_0 + delta_second +  di_slopes[run] * (pad_now - pad_0)

# End of pixel-to-pad time conversion


###############################
# Get Trees
###############################

basedir_pad = "../../drs4_data/"
basedir_pixel = "../plots/"

if run < 10:
    format_pad = "{0}run_2014_09r00000{1}.root"
    format_pixel = "{0}00000{1}/histos.root"
elif run < 100:
    format_pad = "{0}run_2014_09r0000{1}.root"
    format_pixel = "{0}0000{1}/histos.root"
else:
    format_pad = "{0}run_2014_09ro00{1}.root"
    format_pixel = "{0}000{1}/histos.root"

filename_pad = format_pad.format(basedir_pad, run)
filename_pixel = format_pixel.format(basedir_pixel, run)

f_pad = ROOT.TFile.Open(filename_pad)
f_pixel = ROOT.TFile.Open(filename_pixel)

print f_pad

tree_pad = f_pad.Get("rec")
tree_pixel = f_pixel.Get("time_tree")

print "Read:"
print "PAD Tree: ", tree_pad.GetEntries(), "entries"
print "Pixel Tree: ", tree_pixel.GetEntries(), "entries"


###############################
# Gather Information on Run
###############################

# Get the intial numbers for pad and pixel
tree_pad.GetEntry(0)
tree_pixel.GetEntry(0)

# Start with nominal (0/0) event align
# Try to find better pair of events below
initial_n_pad = getattr(tree_pad, br_n_pad)
initial_n_pixel = getattr(tree_pixel, br_n_pixel)

initial_t_pad = getattr(tree_pad, br_t_pad)
initial_t_pixel = getattr(tree_pixel, br_t_pixel)

print "Pad: Initial n = {0}, Initial t = {1}".format(initial_n_pad, initial_t_pad)
print "Pixel: Initial n = {0}, Initial t = {1}".format(initial_n_pixel, initial_t_pixel)

# Get the final numbers for pad and pixel
tree_pad.GetEntry(tree_pad.GetEntries()-1)
tree_pixel.GetEntry(tree_pixel.GetEntries()-1)

final_n_pad = getattr(tree_pad, br_n_pad)
final_n_pixel = getattr(tree_pixel, br_n_pixel)

final_t_pad = getattr(tree_pad, br_t_pad)
final_t_pixel = getattr(tree_pixel, br_t_pixel)

print "Pad: Final n = {0}, Final t = {1}".format(final_n_pad, final_t_pad)
print "Pixel: Final n = {0}, Final t = {1}".format(final_n_pixel, final_t_pixel)

print "Duration: {0} seconds".format(final_t_pad - initial_t_pad)


###############################
# Try to find two good events for aligning times
###############################

if find_align:
    for i_align_pixel in range(6):

        tree_pixel.GetEntry(i_align_pixel)

        for i_align_pad in range(6):

            tree_pad.GetEntry(i_align_pad)

            initial_t_pad = getattr(tree_pad, br_t_pad)
            initial_t_pixel = getattr(tree_pixel, br_t_pixel)

            h = ROOT.TH1F("", "", 100, -0.005, 0.005)

            i_pixel = 20

            for i_pad in xrange(20, 1000):

                tree_pad.GetEntry(i_pad)
                time_pad = getattr(tree_pad, br_t_pad)

                delta_ts = []

                for i_pixel_test in range(i_pixel+1-10, i_pixel+1+10):        

                    tree_pixel.GetEntry(i_pixel_test)        
                    time_pixel = getattr(tree_pixel, br_t_pixel)

                    delta_ts.append( [i_pixel_test, pixel_to_pad_time(time_pixel, 
                                                                      initial_t_pixel, 
                                                                      time_pad, 
                                                                      initial_t_pad) - time_pad])


                best_match =  sorted(delta_ts, key = lambda x:abs(x[1]))[0]
                h.Fill(best_match[1])

                # Set the starting-value for the next iteration 
                # Our basis assumption is no-missing event
                i_pixel = best_match[0] + 1
            # End of loop over pad events
            
            h.Draw()
            c.Print("run_{0}/ipad_{1}_ipixel_{2}.pdf".format(run, i_align_pad, i_align_pixel))

            print "Pad Event: {0}  Pixel Event:{1}".format(i_align_pad, i_align_pixel)
            print h.GetMean(), h.GetRMS()
        

###############################
# Look at time drift
###############################

if plot_drift:

    if do_test:
        print "Doing Test - restricting to max 20k events"
        max_events = min(20000, tree_pad.GetEntries()-1)

        # Update final-times for test analysis
        tree_pad.GetEntry(max_events)
        tree_pixel.GetEntry(max_events)        
        final_t_pad = getattr(tree_pad, br_t_pad)
        final_t_pixel = getattr(tree_pixel, br_t_pixel)

    else:
        max_events = tree_pad.GetEntries()-1



    if do_test:
        h2 = ROOT.TH2D("", "", 2000, 0, final_t_pad-initial_t_pad, 300, -0.01, 0.01)
    else:
        h2 = ROOT.TH2D("", "", 2000, 0, final_t_pad-initial_t_pad, 300, -0.01, 0.01)

    h = ROOT.TH1D("","",500, -0.007, 0.007)
    h_delta_n = ROOT.TH1D("", "", 21, -10, 10)

    tree_pad.GetEntry(0)
    tree_pixel.GetEntry(0)

    initial_t_pad = getattr(tree_pad, br_t_pad)
    initial_t_pixel = getattr(tree_pixel, br_t_pixel)
        
    i_pixel = 20

    for i_pad in xrange(20,max_events):

        if i_pad % 1000 == 0:
            print "{0} / {1}".format(i_pad, max_events)

        tree_pad.GetEntry(i_pad)
        time_pad = getattr(tree_pad, br_t_pad)

    
        delta_ts = []
        for i_pixel_test in range(i_pixel-6, i_pixel+6):        

            if i_pixel_test < 0:
                continue

            tree_pixel.GetEntry(i_pixel_test)        
            time_pixel = getattr(tree_pixel, br_t_pixel)

            #print "\t\t", i_pixel_test, time_pixel

            delta_ts.append( [i_pixel_test, pixel_to_pad_time(time_pixel, 
                                                              initial_t_pixel, 
                                                              time_pad, 
                                                              initial_t_pad) - time_pad])

        best_match =  sorted(delta_ts, key = lambda x:abs(x[1]))[0]
        #print i_pad, time_pad, best_match, delta_ts
        
        h_delta_n.Fill(best_match[0]-i_pixel+1)

        # Set the starting-value for the next iteration 
        # Our basis assumption is no-missing event
        i_pixel = best_match[0] 

        h.Fill(best_match[1])
        h2.Fill(time_pad-initial_t_pad, best_match[1])
    # End of loop over pad events


    h2.GetXaxis().SetTitle("t_{pixel} - t_{pad} [s]")
    h2.GetYaxis().SetTitle("Events")
    h.Draw()
    c.Print("run_{0}/residual{1}.png".format(run, test_string))

    fun = ROOT.TF1("fun", "[0]+[1]*x")
    h2.Fit(fun,"","")
    h2.GetXaxis().SetTitle("t_{pad} [s]")
    h2.GetYaxis().SetTitle("t_{pixel} - t_{pad} [s]")
    h2.Draw()
    c.Print("run_{0}/time{1}.png".format(run, test_string))

    c.SetLogy(1)
    h_delta_n.Draw()
    c.Print("run_{0}/delta_n{1}.png".format(run, test_string))
    c.SetLogy(0)

