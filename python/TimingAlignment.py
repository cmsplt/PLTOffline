#!/usr/bin/env python

"""
Study timing between pixel and pad detectors.

Also produce tracking based analysis plots.
"""

###############################
# Imports
###############################

import os
import sys
import array
import math

import ROOT

import TimingAlignmentHelpers as TAH


###############################
# Init ROOT
###############################

ROOT.gStyle.SetPadLeftMargin(0.15)
ROOT.gStyle.SetPadRightMargin(0.14)
ROOT.gStyle.SetPadBottomMargin(0.12)
ROOT.gStyle.SetPadTopMargin(0.05)
ROOT.gROOT.ForceStyle()

c = ROOT.TCanvas("","",800,800)


###############################
# Configuration
###############################

if not len(sys.argv) == 3:
    TAH.print_usage()
    sys.exit()

try:
    run = int(sys.argv[1])
    action = int(sys.argv[2])
except:
    TAH.print_usage()
    sys.exit()

print "Going to process run {0} with action = {1}".format(run, action)


###############################
# Diamonds
###############################

TAH.Diamond("dummy", -1., 1., -1., 1.)
TAH.Diamond("IIa-2", -0.2, 0.2, 0., 0.4)
TAH.Diamond("IIa-3", -0.25, 0.15, -0.05, 0.35)
TAH.Diamond("IIa-3-wide-open", -0.5, 0.5, -0.25, 0.65)
TAH.Diamond("IIa-5-pedestal", -0.4, 0.4, 0.4, 0.6)


###############################
# Run Timing Info
###############################

TAH.RunTiming(6  ,  0.0003045,  2.155918e-06, 0, 0)
TAH.RunTiming(12 , -0.00030472, 1.955999e-06, 0, 0)
TAH.RunTiming(38 ,  0.00052454, 1.9e-06,      4, 0)
TAH.RunTiming(63 , -0.00030911, 1.837389e-06, 0, 0)
TAH.RunTiming(65 ,  0.00034346, 1.864050e-06, 0, 0)
TAH.RunTiming(68 , -0.00085284, 2.026819e-06, 6, 0)
TAH.RunTiming(70 , -0.00028498, 1.910828e-06, 6, 0)
TAH.RunTiming(109, 0.000323963498273, 1.81841520034e-06, 15, 1)
TAH.RunTiming(131, -0.000191132147971, 1.93697727798e-06, 13, 2)
TAH.RunTiming(134, -3.1728796239e-05, 1.64755689822e-06, 14, 0)
TAH.RunTiming(354, 0, 1.66190809094e-06, 7, 0, "IIa-2")
TAH.RunTiming(355, -0.000314315985828, 1.66190809094e-06, 2, 1, "IIa-2", 500)
TAH.RunTiming(356, -0.000695592438193, 1.61339888272e-06, 7, 0, "IIa-2", 500)
TAH.RunTiming(358, 0.000249032875294, 1.61704852897e-06, 12, 1, "IIa-2", 500)
TAH.RunTiming(360, -0.000185791051023, 1.59938328397e-06, 5, 1, "IIa-2", 500)
TAH.RunTiming(362, 0.00042190730171, 1.64763938056e-06, 1, 1, "IIa-2", 500)

TAH.RunTiming(528, -0.000415933095508, 1.60475855132e-06, 6, 1, "IIa-3", -25, 3)
TAH.RunTiming(532, -5.38246743255e-05, 1.97836071279e-06, 14, 1, "IIa-3", -50, 3)
TAH.RunTiming(534, -0.000149410234456, 1.28514906983e-06, 0, 0, "IIa-3", -75, 3)

TAH.RunTiming(546, 1.52929003315e-05, 1.69038314973e-06, 0, 0, "IIa-3", -500, 4)
TAH.RunTiming(558, 0.000554312131921, 1.75928791575e-06, 0, 0, "IIa-3", -500, 4)
TAH.RunTiming(565, 0.000473639852545, 1.87068995292e-06, 13, 1, "IIa-3-wide-open", -1000, 4)
TAH.RunTiming(566, -9.5862348191e-05, 1.64943513686e-06, 16, 1, "IIa-3", -1000, 4)
TAH.RunTiming(568, 0.000443434862615, 1.57788860683e-06, 11, 1, "IIa-3-wide-open", -1000, 4)
TAH.RunTiming(630, 0.00028963428651, 1.70790800374e-06, 0, 0, "IIa-5-pedestal", 500, 4)


###############################
# Branch names
###############################

branch_names = {
    # Event Numbers
    "n_pad"   : "n",
    "n_pixel" :"ievent",
    # Time
    "t_pad"   : "time_stamp",  # [seconds]
    "t_pixel" :"time",  # clock-ticks (25 ns spacing)
    # Pixel only:
    # - Hit plane bits 
    "plane_bits_pixel": "hit_plane_bits",
    # -Tracks
    "track_x" : "track_x",
    "track_y" : "track_y",
    
    # Pad only:
    # - Calibration flag
    "calib_flag_pad" : "calibflag",
    # - Integral50
    "integral_50_pad" : "Integral50"
}

# Legacy! Remove ASAP
br_n_pad = "n"
br_n_pixel = "ievent" 
br_t_pad = "time_stamp" # [Unix Time]
br_t_pixel = "time"     # clock-ticks (25 ns spacing)
br_hit_plane_bits_pixel = "hit_plane_bits"
br_track_x = "track_x"
br_track_y = "track_y"
br_calib_flag_pad = "calibflag"
br_integral50 = "Integral50"


###############################
# Prepare Output
###############################

if action == 0:
    test_string = ""
elif action == 1:
    test_string = "_short"
elif action == 2:
    test_string = "_align"
else:
    print "Invalid action: ", action
    print "Exiting"
    sys.exit()

try:
    os.mkdir("run_{0}".format(run))
except:
    pass

# Output ROOT File
filename_out = "run_{0}/track_info{1}.root".format(run, test_string)
f_out = ROOT.TFile(filename_out, "recreate")

# Output Tree
tree_out = ROOT.TTree("track_info", "track_info")

# Output branches
out_branches = {}

# Event Number (from pad)
out_branches["n_pad"] = array.array( 'i', [ 0 ] ) 
tree_out.Branch( 'n_pad', out_branches["n_pad"], 'n_pad/I' )

# Did we accept this event in the pixel+timing analysis
# Possible reasons for rejection:
#   - could not find event in the pixel stream
#   - event found in the pixel stream but time difference too large
#   - event matched but no track from pixels
out_branches["accepted"] = array.array( 'i', [ 0 ] )
tree_out.Branch( 'accepted', out_branches["accepted"], 'accepted/I' )

# Track interesect with pad
out_branches["track_x"] = array.array( 'f', [ 0. ] ) 
out_branches["track_y"] = array.array( 'f', [ 0. ] )
tree_out.Branch( 'track_x', out_branches["track_x"], 'track_x/F' )
tree_out.Branch( 'track_y', out_branches["track_y"], 'track_y/F' )

# Pad integral
out_branches["integral50"] = array.array( 'f', [ 0. ] ) 
tree_out.Branch( 'integral50', out_branches["integral50"], 'integral50/F' )


###############################
# Get Trees
###############################

if TAH.RunTiming.runs[run].n_channels == 4:
    basedir_pad = "../../padreadout-devel-4chan/data/output/"
else:
    basedir_pad = "../../padreadout-devel/data/output/"

basedir_pixel = "../plots/"

if run < 10:
    format_pad = "{0}run_2014_09r00000{1}.root"
    format_pixel = "{0}00000{1}/histos.root"
elif run < 100:
    format_pad = "{0}run_2014_09r0000{1}.root"
    format_pixel = "{0}0000{1}/histos.root"
else:
    format_pad = "{0}run_2014_09r000{1}.root"
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



if action == 2:
    TAH.find_alignment(run, tree_pixel, tree_pad, branch_names, c)
    sys.exit()

###############################
# Look at time drift
###############################

if action == 1:
    print "Doing Initial run - restricting events"
    max_events = min(25000, tree_pad.GetEntries()-1)

    # Update final-times for test analysis
    tree_pad.GetEntry(max_events)
    tree_pixel.GetEntry(max_events)        
    final_t_pad = getattr(tree_pad, br_t_pad)
    final_t_pixel = getattr(tree_pixel, br_t_pixel)

else:
    max_events = tree_pad.GetEntries()-1


# Book histograms
h2 = ROOT.TH2D("", "", 2000, 0, final_t_pad-initial_t_pad, 300, -0.01, 0.01)
h = ROOT.TH1D("","",500, -0.007, 0.007)
h_delta_n = ROOT.TH1D("", "", 21, -10, 10)
h_calib_events = ROOT.TH2D("", "", 16, -0.5, 15.5, 2, -0.5, 1.5)

h_tracks = ROOT.TH2D("","", 100, -1, 1, 100, -1, 1)
h_integral = ROOT.TH3D("","", 100, -1, 1, 100, -1, 1, 200, -1000, 1000)

h_tracks_zoom = ROOT.TH2D("","", 
                          50, # bins in x 
                          TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].x_pos_min,
                          TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].x_pos_max,
                          50, # bins in y
                          TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].y_pos_min,
                          TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].y_pos_max)

h_integral_zoom = ROOT.TH3D("","", 
                            50, # bins in x 
                            TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].x_pos_min,
                            TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].x_pos_max,
                            50, # bins in y
                            TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].y_pos_min,
                            TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].y_pos_max,
                            200, -1000, 1000)

# Also create a matrix of histograms for the PH as a function of the
# location
n_boxes = 5 # How many boxes per side. Will use the boundaries of the
            # diamond and the coordinate_to_box function
integral_box_matrix = []
for x_pos in range(n_boxes):
    tmp_li = []
    for y_pos in range(n_boxes):
        if TAH.RunTiming.runs[run].bias_voltage > 0:
            tmp_li.append(ROOT.TH1D("", "", 200, -500, 200))
        else:
            tmp_li.append(ROOT.TH1D("", "", 200, -200, 500))
    # End of x-loop
    integral_box_matrix.append(tmp_li)
# End of y-loop        

        
tree_pad.GetEntry(TAH.RunTiming.runs[run].align_pad)
tree_pixel.GetEntry(TAH.RunTiming.runs[run].align_pixel)

initial_t_pad = getattr(tree_pad, br_t_pad)
initial_t_pixel = getattr(tree_pixel, br_t_pixel)

i_pixel = 0

for i_pad in xrange(max_events):

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

        delta_ts.append( [i_pixel_test, TAH.pixel_to_pad_time(time_pixel, 
                                                              initial_t_pixel, 
                                                              time_pad, 
                                                              initial_t_pad,
                                                              TAH.RunTiming.runs[run].offset,
                                                              TAH.RunTiming.runs[run].slope) - time_pad])

    best_match =  sorted(delta_ts, key = lambda x:abs(x[1]))[0]

    i_pixel = best_match[0] 
    tree_pixel.GetEntry(i_pixel)        

    h_delta_n.Fill(best_match[0]-i_pixel+1)
    h.Fill(best_match[1])
    h2.Fill(time_pad-initial_t_pad, best_match[1])

    # Check if we are happy with the timing
    # (residual below 1 ms)
    if abs(best_match[1]) < 0.001:        
        hit_plane_bits = getattr(tree_pixel, br_hit_plane_bits_pixel)
        calib_flag = getattr(tree_pad, br_calib_flag_pad)
        h_calib_events.Fill(hit_plane_bits, calib_flag)

        out_branches["n_pad"][0] = getattr(tree_pad, br_n_pad)
        out_branches["accepted"][0] = 1
        out_branches["track_x"][0] = getattr(tree_pixel, br_track_x)
        out_branches["track_y"][0] = getattr(tree_pixel, br_track_y)
        out_branches["integral50"][0] = getattr(tree_pad, br_integral50)

        tree_out.Fill()            
        
        h_tracks.Fill(getattr(tree_pixel, br_track_x),
                      getattr(tree_pixel, br_track_y))

        h_tracks_zoom.Fill(getattr(tree_pixel, br_track_x),
                           getattr(tree_pixel, br_track_y))
        
        h_integral.Fill(getattr(tree_pixel, br_track_x),
                        getattr(tree_pixel, br_track_y),
                        getattr(tree_pad, br_integral50))

        h_integral_zoom.Fill(getattr(tree_pixel, br_track_x),
                             getattr(tree_pixel, br_track_y),
                             getattr(tree_pad, br_integral50))
        
        ret = TAH.coordinate_to_box(getattr(tree_pixel, br_track_x), 
                                    getattr(tree_pixel, br_track_y),
                                    TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].x_pos_min,
                                    TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].x_pos_max,
                                    TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].y_pos_min,
                                    TAH.Diamond.diamonds[TAH.RunTiming.runs[run].diamond_name].y_pos_max, n_boxes)
        if ret != -1:
            x_box = ret[0]
            y_box = ret[1]
            integral_box_matrix[x_box][y_box].Fill( getattr(tree_pad, br_integral50))
        

    # done filling tree and calibration histogram

# End of loop over pad events



h.GetXaxis().SetTitle("t_{pixel} - t_{pad} [s]")
h.GetYaxis().SetTitle("Events")
h.Draw()
c.Print("run_{0}/residual{1}.pdf".format(run, test_string))

print h2,c
fun = ROOT.TF1("fun", "[0]+[1]*x")
h2.Fit(fun,"","")
h2.GetYaxis().SetTitleOffset(1.9)
h2.GetXaxis().SetTitle("t_{pad} [s]")
h2.GetYaxis().SetTitle("t_{pixel} - t_{pad} [s]")
h2.Draw()
c.Print("run_{0}/time{1}.pdf".format(run, test_string))

TAH.RunTiming.runs[run].offset -= fun.GetParameter(0)
TAH.RunTiming.runs[run].slope  -= fun.GetParameter(1)    


c.SetLogy(1)
h_delta_n.Draw()
c.Print("run_{0}/delta_n{1}.pdf".format(run, test_string))
c.SetLogy(0)


ROOT.gStyle.SetOptStat(0)
c.SetLogz(1)
h_calib_events.GetXaxis().SetTitle("Pixel Plane Hit Bit")
h_calib_events.GetYaxis().SetTitle("Pad Calibration Flag")
h_calib_events.GetYaxis().SetTitleOffset(1.5)
h_calib_events.Draw("COLZTEXT")
c.Print("run_{0}/calib_events{1}.pdf".format(run, test_string))

ROOT.gStyle.SetOptStat(0)
c.SetLogz(1)
h_tracks.GetXaxis().SetTitle("Pad position x [cm]")
h_tracks.GetYaxis().SetTitle("Pad position y [cm]")
h_tracks.GetYaxis().SetTitleOffset(1.5)
h_tracks.Draw("COLZ")
c.Print("run_{0}/tracks{1}.pdf".format(run, test_string))

ROOT.gStyle.SetOptStat(0)
c.SetLogz(1)
h_tracks_zoom.GetXaxis().SetTitle("Pad position x [cm]")
h_tracks_zoom.GetYaxis().SetTitle("Pad position y [cm]")
h_tracks_zoom.GetYaxis().SetTitleOffset(1.5)
h_tracks_zoom.Draw("COLZ")
c.Print("run_{0}/tracks_zoom{1}.pdf".format(run, test_string))


ROOT.gStyle.SetOptStat(0)
c.SetLogz(0)
proj = h_integral.Project3DProfile("yx")
proj.SetTitle("")
proj.GetXaxis().SetTitle("Pad Position x [cm]")
proj.GetYaxis().SetTitle("Pad Position y [cm]")
proj.GetXaxis().SetTitleOffset(1.2)
proj.GetYaxis().SetTitleOffset(1.5)

proj.Draw("COLZ")
c.Print("run_{0}/integral{1}_fullrange.pdf".format(run, test_string))


ROOT.gStyle.SetOptStat(0)
c.SetLogz(0)
proj_zoom = h_integral_zoom.Project3DProfile("yx")
proj_zoom.SetTitle("")
proj_zoom.GetXaxis().SetTitle("Pad Position x [cm]")
proj_zoom.GetYaxis().SetTitle("Pad Position y [cm]")
proj_zoom.GetXaxis().SetTitleOffset(1.2)
proj_zoom.GetYaxis().SetTitleOffset(1.5)

proj_zoom.Draw("COLZ")
c.Print("run_{0}/integral{1}_zoom_fullrange.pdf".format(run, test_string))


if TAH.RunTiming.runs[run].bias_voltage > 0:
    proj.SetMinimum(-550)
    proj.SetMaximum(50)

    proj_zoom.SetMinimum(-550)
    proj_zoom.SetMaximum(50)
else:
    proj.SetMinimum(-50)
    proj.SetMaximum(500)

    proj_zoom.SetMinimum(-50)
    proj_zoom.SetMaximum(500)

proj.Draw("COLZ")
c.Print("run_{0}/integral{1}.pdf".format(run, test_string))

proj_zoom.Draw("COLZ")
c.Print("run_{0}/integral_zoom{1}.pdf".format(run, test_string))





for x_pos in range(n_boxes):
    for y_pos in range(n_boxes):
        
        fun = ROOT.TF1("", "gaus")
        integral_box_matrix[x_pos][y_pos].Fit(fun)
        print "XXX X: {0} Y: {1} Mean: {2:2.2f} RMS {3:2.2f}".format(x_pos, 
                                                                     y_pos, 
                                                                     fun.GetParameter(1), 
                                                                     fun.GetParameter(2))
        integral_box_matrix[x_pos][y_pos].Draw()
        c.Print("run_{0}/1d_integral_x_{1}_y_{2}{3}.pdf".format(run, x_pos, y_pos, test_string))
        c.Print("run_{0}/1d_integral_x_{1}_y_{2}{3}.png".format(run, x_pos, y_pos, test_string))


f_out.Write()

TAH.RunTiming.runs[run].print_info()
