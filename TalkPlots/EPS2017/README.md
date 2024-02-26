# EPS2017

This directory contains some miscellaneous scripts for making the plots for my talk at EPS2017. Since in many cases the scripts for making the original plots were no longer available, many of these scripts instead make better legends which I then spliced into the original PNG.

Scripts within:

* legendXX.C -- scripts for making new legends for some plots
* makeRatioPlot.C -- takes the raw PLT/DT ratio data from Dan's spreadsheet and plots it in nicer form
* plotEffVsTime.C -- plots the efficiency vs. time using Thoth's efficiency data. This uses eff_by_fill.csv, which is a subset of Thoth's data that I extracted for a single scope and ROC at the beginning of each fill.
