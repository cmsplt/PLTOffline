#!/usr/bin/env python3

import os, sys, csv

# This is a simple script to get the beam intensities using brilcalc beam and then put them into a format that
# can be used by PlotTrackLumiVdM.C. You'll need to fill in three things below:
# 1) The fill number.
# 2) The target timestamp. Basically it will take the first timestamp that matches this string, so you can be
# vague (08:00) or specific (08:00:21) or really specific ("05/27/16 08:00:21"), as you desire.
# 3) The list of bunches that we're interested in.

#fill_number = "4954"
#target_time = "08:00"
fill_number = "4945"
target_time = " 19:00"

bunches = [1, 41, 81, 110, 121, 161, 201,
           241, 281, 591, 872, 912, 952,
           992, 1032, 1072, 1112, 1151, 1152,
           1682, 1783, 1823, 1863, 1903, 1943,
           1983, 2023, 2063, 2654, 2655, 2694,
           2734, 2774, 2814, 2854, 2894, 2934]

fill_number = str(fill_number)
# 1) Invoke brilcalc to get the beam intensities
beam_file = "beam_"+fill_number+".csv"
os.system("brilcalc beam -f "+fill_number+" --xing -o "+beam_file)

# 2) Read through the file until we get the time we want.
found_target = False
with open(beam_file) as beam_data:
    reader = csv.reader(beam_data, delimiter=',')
    for row in reader:
        # Skip comments
        if row[0][0] == '#':
            continue
        if row[3].find(target_time) >= 0:
            found_target = True
            # drop beginning and ending brackets, then split
            bxdata = row[4][1:-1].split(" ")
            beam_intensity = {}
            for i in range(0, len(bxdata), 3):
                beam_intensity[int(bxdata[i])] = [bxdata[i+1], bxdata[i+2]]
            break

# 3) Clean up after ourselves.
os.unlink(beam_file)

if not found_target:
    print("Sorry, failed to find the target timestamp in the file!")
    sys.exit(1)

# 4) Get the actual bunches that we're interested in.
beam1_results = []
beam2_results = []
for i in bunches:
    if i in beam_intensity:
        beam1_results.append(beam_intensity[i][0])
        beam2_results.append(beam_intensity[i][1])
    else:
        beam1_results.append("0.0")
        beam2_results.append("0.0")
print("const std::vector<float> beam1Intensity = {"+", ".join(beam1_results)+"};")
print("const std::vector<float> beam2Intensity = {"+", ".join(beam2_results)+"};")
