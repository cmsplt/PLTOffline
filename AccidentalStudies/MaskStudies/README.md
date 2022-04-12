# MaskStudies

This directory contains the data for studying the optimum mask size. This study was originally performed by Joe in May 2016, but since I (Paul) was not able to find the original results and scripts anywhere, I recreated this study in 2022 for the PLT paper (producing slightly different results for whatever reason).

### Overview

The study here uses the data from fill 4892. This was an early 2016 fill (May 2016) when we were still using a mask with the same active area as 2015 (28x41 in the center plane and 34x50 in the outer planes). The basic procedure of the study is simple: take the input data from the fill, try a smaller mask size, reject any points lying outside the new mask size, rerun the accidental calculation, and measure the resulting accidental rate as a function of mask size.

### Code

I implemented the original code for appying a mask in `PLTBinaryFileReader::ReadOnlinePixelMask()` and this was committed in 2016; the code that Joe wrote to update `MeasureAccidentals` to allow specifying a mask file was unfortunately not, but this was an easy addition.

The original mask file used in May 2016 (prior to the optimization from this study) is found in the `archive/` directory in the `cmsplt` repository as `cmsplt/archive/interface/conf/Mask_May2016_v1.txt`; I've copied it here. I've also written a script to reduce the active area, `reduce_active_area.py`. This was used to produce the other masks used in this study; for example, the command to reduce this mask to the final selected 2016 size was `reduce_active_area.py 24 36 26 38` (the output should be redirected to a file).

Then, I ran `MeasureAccidentals` with the new masks, using a command of this form:
`./MeasureAccidentals /localdata/2016/SLINK/Slink_20160507.165735.dat GainCal/GainCalFits_20160501.155303.dat ALIGNMENT/Trans_Alignment_4892.dat TrackDistributions/TrackDistributions_MagnetOn2016_4892.txt 4892 - AccidentalStudies/MaskStudies/Mask_Resized_20x30_24x36.txt`
using the standard gaincal, alignment, and track distribution for 2016, along with `-` to skip the timestamp file and the mask file that I wanted to use.

This produced an `AccidentalRates_4892.txt` file which I then renamed to include the mask size, and then (following the normal procedure) used `ParseCondDBData` to attach the online fast-or luminosity from `AccidentalStudies/Fill4892.csv` to produce `CombinedRates_4892.txt`. I then also renamed this file, deleted the last line (because it was empty), and updated the number of lines in the header and added the number of colliding bunches.

### Data files

The files here include the following:
* The regular files without any additional mask applied, `AccidentalRates_4892.txt` and `CombinedRates_4892.txt`. These are simply copies of the same files in the parent directory.
* The base mask file, `Mask_May2016_v1.txt`, the script for resizing the active area, `reduce_active_area.py`, and the various resized masks, `Mask_Resized_AAxBB_CCxDD.txt`.
* The results of running `MeasureAccidentals` using each of the mask files, `AccidentalRates_4892_AAxBB_CCxDD.txt` and `CombinedRates_4892_AAxBB_CCxDD.txt`.
* The plotting script and resulting plots (see below).

### Plots

The plotting script used to plot these results is `PlotAccidentalRatesMasks.C`. This is derived from `PlotAccidentalRates2016.C` in the parent directory and works very much the same way. It produces both a PDF and PNG output version as `AccidentalRate_MaskCalibration.{pdf,png}`.
