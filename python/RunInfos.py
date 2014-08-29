#!/usr/bin/env python

"""
Information on the runs taken with the different telescopes.

Flux numbers are in kHz and taken from Steve's spreadsheet.
"""

# All dictionaries have the telescopeID as key
# di_di_runs has dictionaries with run:flux matching as values
# di_li_runs_up has lists of runs while increasing the flux
# di_li_runs_down has lists of runs while decreasing the flux
# di_li_runs_final lists of runs at maximum flux, taken in the end
# di_di_rocs has dictionaries with roc-number:name matching as values
di_di_runs = {}
di_li_runs_up = {}
di_li_runs_down = {}
di_li_runs_final = {}
di_di_rocs = {}


# Telescope 0: For testing
di_di_runs[0] = {316: 1.6}
di_li_runs_up[0] = [316]
di_li_runs_down[0] = []
di_li_runs_final[0] = []
di_di_rocs[0] = {0 : "Si",
                 1 : "PolyA", 
                 2 : "PolyD", 
                 3 : "S86", 
                 4 : "S105", 
                 5 : "Si"}
                 

# Telescope 1
di_di_runs[1] = {
    322: 1.6,
    325: 12.9,
    327: 130.6,
    330: 1167.,
    333: 20809.,
    338: 1137.,
    340: 125.4,
    343: 10.8,
    347: 1.4,
    348: 1.5,
    350: 20809.2,
    352: 21387.3,
}

di_li_runs_up[1] = [322, 325, 327, 330, 333]
di_li_runs_down[1] = [338, 340, 343, 347, 348]
di_li_runs_final[1] = [350, 352]

di_di_rocs[1] = {0 : "Si",
                 1 : "PolyA", 
                 2 : "PolyD", 
                 3 : "S86", 
                 4 : "S105", 
                 5 : "Si"}



# Telescope 2
di_di_runs[2] = {
    466: 2.,
    467: 18.2,
    469: 1313.9,
    470: 9445.1,
    471: 1269.4,
    472: 1272.8,
    473: 143.4,
    474: 13.6,
    475: 2.0,
    476: 9398.8,
    478: 22138.7
}
di_li_runs_up[2] = [466, 467, 469, 470]
di_li_runs_down[2] = [471, 472, 473, 474, 475]
di_li_runs_final[2] = [476, 478]


di_di_rocs[2] = {0 : "Si",
                 1 : "PolyB", 
                 2 : "PolyD", 
                 3 : "S108", 
                 4 : "Si", 
                 5 : "Si"}
