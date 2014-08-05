#!/usr/bin/env python

"""
Check for fitting problems in the pulse height calibrations
"""

###############################
# Imports
###############################

import sys
import glob

import ROOT

ROOT.gStyle.SetOptStat(1111111)

###############################
# Configuration
###############################

telescope = "2"
DEBUG = True

# Use an absolute path as the calibrations are in different places on different machines anyways
if telescope == "1":
    input_directory = "/Users/gregor/work/DHidasPLT/Telescope_Silicon0_4_PolyA_PolyD_S86_S105_29052014_0408/"
elif telescope == "1a":
    input_directory = "/Users/gregor/work/DHidasPLT/Calibrations/Telescope_Silicon0_4_PolyA_PolyD_S86_S105_29052014_1730/"
elif telescope == "2":
    input_directory = "/Users/gregor/work/DHidasPLT/Calibrations/Telescope_Si0_DiaB_DiaD_DiaS108_Si4_Si1_20140601_2136/"
else:
    print "Invalid Telescope. Exiting.."
    sys.exit()


###############################
# get_calibration_files
###############################

def get_calibration_filenames(directory):
    """ Receive a directory. Return a list of tuples:
        [ [full_name, short_name], [...] ]
    Assume the following naming scheme ({X} being a one or two digit integer):
    phCalibration_C{X}.dat.fit.root
    The short name is just {X}.
    """

    name_prefix = "phCalibration_C"
    name_postfix = ".dat.fit.root"

    # Glob seems not to support the '+'-modifier
    glob_string_one_digit = directory + name_prefix + "[0-9]" + name_postfix
    glob_string_two_digit = directory + name_prefix + "[0-9][0-9]" + name_postfix

    li_filenames = glob.glob(glob_string_one_digit)
    li_filenames += glob.glob(glob_string_two_digit)

    if DEBUG:
        for fn in li_filenames:
            print fn

    # Also extract the short-names
    # (might be more elegant to use a proper regex instead of the glob)
    li_shortnames = [fn.replace(directory, "").replace(name_prefix, "").replace(name_postfix, "") for fn in li_filenames]

    print li_shortnames
    return zip(li_filenames, li_shortnames)


###############################
# extract_fit_chi2
###############################

def extract_fit_chi2(gr):
    """ Receive a ROOT TGraph. Get the associated fit and return it's Chi2.
    If no fit, available, return 50000
    """

    function_name = "FitFunc"

    fun = gr.GetFunction(function_name)
    if fun:
        return fun.GetChisquare()
    else:
        return 50000


###############################
# draw_fit
###############################

def draw_fit(gr, telescope_id, roc_id, canvas):
    """ Receive a TGraph and a telescope ID. Draw the fit result and save to a file.
    """

    gr.Draw("AP*")

    t = ROOT.TText()
    t.DrawTextNDC(0.2, 0.7, "chi2 = {0}".format(extract_fit_chi2(gr)))

    canvas.Print("plots/Telescope{0}_C{1}_{2}.png".format(telescope_id, roc_id, gr.GetName()))


###############################
# main
###############################

def main():

    # Prepare ROOT
    can = ROOT.TCanvas("", "", 800, 800)
    #can.SetLogx(1)

    li_calibration_filename_shortname = get_calibration_filenames(input_directory)

    # Loop over input files
    for input_filename, input_shortname in li_calibration_filename_shortname:

        if not input_shortname == "5":
            continue

        # Get all the TGraph objects with a name containing "phCal_"
        # from the file
        f = ROOT.TFile.Open(input_filename)
        li_keys = f.GetListOfKeys()
        li_graphs = []
        for k in li_keys:
            if "phCal_" in k.GetName() and k.GetClassName() == "TGraph":
                li_graphs.append(f.Get(k.GetName()))

        print "Read {0} graphs from file {1}".format(len(li_graphs), input_filename)

        h_chi2 = ROOT.TH1F("h_chi2_Telescope{0}_C{1}".format(telescope, input_shortname), "", 200, 0, 10000)
        h_chi2_wide = ROOT.TH1F("h_chi2_wide_Telescope{0}_C{1}".format(telescope, input_shortname), "", 200, 0, 100000)

        for gr in li_graphs:
            chi2 = extract_fit_chi2(gr)

            h_chi2.Fill(chi2)
            h_chi2_wide.Fill(chi2)

            # draw_fit(gr, telescope, input_shortname, can)

        for h in [h_chi2, h_chi2_wide]:
            h.Draw()
            can.Print("plots/" + h.GetName() + ".png")
    # End loop over filenames


# Only execute when called directly
if __name__ == '__main__':
    main()
