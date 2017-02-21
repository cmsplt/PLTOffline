This directory contains all the results from measuring the accidentals for the 2015 and 2016 runs and the
scripts used to plot these results. Here's a quick rundown of the basic procedure.

1) Run MeasureAccidentals. You need at least five arguments: the slink file, the gain cal fits file, the
alignment file, the track distributions file, and the fill number.

For the 2015 accidental analysis, the gain cal file used was GainCal/GainCalFits_20150923.225334.dat, the
alignment file used was ALIGNMENT/Trans_Alignment_4449.dat, and the track distributions file was
TrackDistributions/TrackDistributions_MagnetOn.txt. (For magnet-off calibrations, those latter two files were
ALIGNMENT/Trans_Alignment_4341.dat and TrackDistributions/TrackDistributions_MagnetOff_4341.txt respectively.)

For the 2016 accidental analysis, the files used are GainCal/GainCalFits_20160501.155303.dat,
ALIGNMENT/Trans_Alignment_4892.dat, and TrackDistributions/TrackDistributions_MagnetOn2016_4892.txt.

If this is a regular fill, you don't need a sixth argument; it will measure the accidental rate in
five-minute steps. If this is a VdM or other special fill, specify a timestamp file as the sixth argument, so
that it will measure the accidental rate for each step of the scan. You can make this timestamp file from a
VdM csv file by using scripts/extractVDMTimestamps.py on the online machines (although you'll need to edit to
remove the separation column, which is included for reference), or just make it by hand (fun).

Sample invocation for a regular 2016 fill (4990 in this example):

./MeasureAccidentals /raid/PLT/SlinkData_2016/Slink_20160514.200310.dat GainCal/GainCalFits_20160501.155303.dat
ALIGNMENT/Trans_Alignment_4892.dat TrackDistributions/TrackDistributions_MagnetOn2016_4892.txt 4990

Sample invocation for a regular 2015 fill (4444 in this example):
./MeasureAccidentals /raid/PLT/SlinkData_2015/Slink_20150930.234122.dat GainCal/GainCalFits_20150923.225334.dat
ALIGNMENT/Trans_Alignment_4449.dat TrackDistributions/TrackDistributions_MagnetOn.txt 4444

Sample invocation for a VdM scan (scan Y2 from fill 4266 in this example):
./MeasureAccidentals /raid/PLT/SlinkData_2015/Slink_20150825.003211.dat GainCal/GainCalFits_20150923.225334.dat
ALIGNMENT/Trans_Alignment_4449.dat TrackDistributions/TrackDistributions_MagnetOn.txt 4266
AccidentalStudies/Timestamp_Fill4266_VdMScanY2.txt

Running this produces AccidentalRates_NNNN.txt, where NNNN is the fill number you specified, as well as a
bunch of plots in the plots/NNNN directory.

The format of this file is as follows. First, the total number of data points in the file. Then, for each
point, a line of the form
begin timestamp,end timestamp,number of triggers,number of total tracks,number of good tracks

The program will also provide at the end some information about what caused failing tracks to fail. I've
saved this information for a few fills in the AccidentalFails_NNNN.txt files for reference.

2) Get the CSV file containing the online luminosity from the Conditions DB: go to the FillReport page for the
fill, click the ConditionBrowser link at the top, change the start time to the beginning of stable beams, and
then plot CMS_BEAM_COND/CMS_BRIL_LUMIPLTZ/LUMI_TOTINST. Save the result as a csv file. If the fill is long, you
may need to do this in two separate pieces and join them later.

3) Next, run ParseCondDBData, giving the accidental rate file and the CSV file as arguments. Sample
invocation:

../ParseCondDBData AccidentalRates_4444.txt Fill4444.csv

This will produce an output file CombinedRates.txt. You will also want to rename it to something like
CombinedRates_4444.txt. You will also need to make at least one modification: on the first line, add the
number of bunches in the fill after the number of data points. There might also be some extra lines after the
end of the fill that you may want to trim out (if you do this, don't forget to also change the number of data
points).

The format of this file is similar to the AccidentalRates file, except there are two more fields: the number
of online lumi points in this time period, and the total of the online lumi numbers in the time period.

For some studies I've also made an additional version of the CombinedRates file (the version with _clean at
the end) in which I've gone through and removed points affected by optimization scans. Similarly, for the VdM
and mu scan files, there's also a _Central version which only includes the central points of the scans (since
the accidental rate measurement becomes unreliable for large beam separation).

4) Finally, plot. To plot a single fill, use PlotAccidentalRates.C. To plot a bunch of fills on the same plot,
use PlotAccidentalRatesAllScans.C. These have been updated to PlotAccidentalRates2016Single.C and
PlotAccidentalRates2016.C, respectively, for the 2016 versions. See those files for specific documentation
(there are also a few other plot scripts for miscellaneous purposes).
