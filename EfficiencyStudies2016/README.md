# EfficiencyStudies2016

This directory contains the results of Thoth's studies from April 2017 using the track efficiency method on the 2016 data. The files within are:

* `efficiency_linearity_2016_v1.csv.gz` (zipped because it's quite a large file) -- the original results from Thoth formatted in a CSV file, containing the efficiency for each ROC for each time interval.
* `process_eff.pl` -- a very simple Perl script I wrote to extract the efficiency for a specific ROC (channel 4 ROC 1 is currently selected) for each fill (only the efficiency of at the beginning of the fill is considered).
* `eff_by_fill.csv` -- the results of the above script.

That's all!
--Paul Lujan, August 2023
