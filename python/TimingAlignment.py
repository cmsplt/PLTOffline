#!/usr/bin/env python

"""
Study timing between pixel and pad detectors.
"""

###############################
# Imports
###############################

import sys

import ROOT

ROOT.gStyle.SetPadLeftMargin(0.15)
ROOT.gStyle.SetPadBottomMargin(0.15)
ROOT.gROOT.ForceStyle()


###############################
# Configuration
###############################

run = 45

# Branchnames:
# Event Numbers
br_n_pad = "n"
br_n_pixel = "ievent"
# Time
br_t_pad = "time_stamp" # [Unix Time]
br_t_pixel = "time"     # clock-ticks (25 ns spacing)


###############################
# Helper Function
# pixel_to_pad_time
###############################

def pixel_to_pad_time( pixel_now, pixel_0, pad_0):
    
    # How many ticks have passed since first pixel time-stamp
    delta_pixel = pixel_now - pixel_0

    # Convert ticks to seconds (1 tick ~ 25 ns)
    delta_second = delta_pixel * 25e-9

    # Add time difference (in seconds) to initial pad time
    return pad_0 + delta_second
# End of pixel-to-pad conversion


###############################
# Get Trees
###############################

basedir_pad = "/home/diamond/Testbeam_September2014/drs4_data/"
basedir_pixel = "../plots/"

if run < 100:
    format_pad = "{0}run_00{1}.root"
    format_pixel = "{0}0000{1}/histos.root"
else:
    format_pad = "{0}run_0{1}.root"
    format_pixel = "{0}000{1}/histos.root"

filename_pad = format_pad.format(basedir_pad, run)
filename_pixel = format_pixel.format(basedir_pixel, run)

f_pad = ROOT.TFile.Open(filename_pad)
f_pixel = ROOT.TFile.Open(filename_pixel)

t_pad = f_pad.Get("rec")
t_pixel = f_pixel.Get("time_tree")

print "Read:"
print "PAD Tree: ", t_pad.GetEntries(), "entries"
print "Pixel Tree: ", t_pixel.GetEntries(), "entries"


###############################
# Sync
###############################

c = ROOT.TCanvas("","",800,800)

# Get the intial numbers for pad and pixel


#for test in range(0, 30):

t_pad.GetEntry(3)
t_pixel.GetEntry(3)

initial_n_pad = getattr(t_pad, br_n_pad)
initial_n_pixel = getattr(t_pixel, br_n_pixel)

initial_t_pad = getattr(t_pad, br_t_pad)
initial_t_pixel = getattr(t_pixel, br_t_pixel)

print "Pad: Initial n = {0}, Initial t = {1}".format(initial_n_pad, initial_t_pad)
print "Pixel: Initial n = {0}, Initial t = {1}".format(initial_n_pixel, initial_t_pixel)

# Get the last numbers for pad and pixel
t_pad.GetEntry(t_pad.GetEntries()-1)
t_pixel.GetEntry(t_pixel.GetEntries()-1)

final_n_pad = getattr(t_pad, br_n_pad)
final_n_pixel = getattr(t_pixel, br_n_pixel)

final_t_pad = getattr(t_pad, br_t_pad)
final_t_pixel = getattr(t_pixel, br_t_pixel)

print "Pad: Final n = {0}, Final t = {1}".format(final_n_pad, final_t_pad)
print "Pixel: Final n = {0}, Final t = {1}".format(final_n_pixel, final_t_pixel)

print "-->", pixel_to_pad_time(final_t_pixel, initial_t_pixel, initial_t_pad) - final_t_pad

i_pixel = 0

h = ROOT.TH1F("", "", 1000, -0.03, 0.03)
h2 = ROOT.TH2D("", "", 2000, 0, 70000, 100, -0.03, 0.03)

time_pad_last = 0

for i_pad in xrange(70000):

    if i_pad % 1000 == 0:
        print i_pad

    t_pad.GetEntry(i_pad)
    time_pad = getattr(t_pad, br_t_pad)

    delta_ts = []
    for i_pixel_test in range(i_pixel-20, i_pixel+20):        

        t_pixel.GetEntry(i_pixel_test)        
        time_pixel = getattr(t_pixel, br_t_pixel)

        delta_ts.append( [i_pixel_test, pixel_to_pad_time(time_pixel, 
                                                          initial_t_pixel, 
                                                          initial_t_pad) - time_pad])

    best_match =  sorted(delta_ts, key = lambda x:abs(x[1]))[0]


    i_pixel = best_match[0]

    h.Fill(best_match[1])
    h2.Fill(i_pad, best_match[1])
    #print i_pad, best_match[1]

    time_pad_last = time_pad

h.Draw()
#c.Print("{0}.pdf".format(test))

h2.Draw()
