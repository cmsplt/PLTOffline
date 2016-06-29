# VdMStudies
This directory contains results from the 2016 VdM track Analysis (Fill 4954) and also serves as a template for making similar studies for other fills.

## Setup
1) Slink_File and Gaincal_File for a given fill
2) Generate Alignment file using ./CalculateAlignment Slink_File Gaincal_File Alignment_IdealInstall.dat
3) Change Trans_Alignment.dat generated from (2) to Trans_AlignmentFillNo.dat


# Steps
We would like to investigate the track parameters. To that end, we first generate the TrackParams.root file by invoking the following:

## Get TrackParams
`./TrackParams Slink_File Gaincal_File Trans_AlignmentFillNo`
Default track TrackParameters are Slopes, Residuals, BX, Channels. You would need to change TrackParams.cc if you are interested in other variables (intercepts to Z=0 plane,X-Y at local telescope coordinates per ROC,etc). Fun!

### VdM Specific
For VdM, we are interested in trackas as a function of separation between beams. Need scanpoint information scanpointY1.dat (pointNum, timeFrom, timeTo, separation). Now we ttree per separation point by invoking the following:

`root -l
.L VdMClass.C+
VdMClass m
m.Loop("scanpointY1")`

Boom!

### Generate text file for 0 separation
root -l t4954scanpointY1.root
perSCP->SetScanField(0)
perSCP->Scan("sx:sy","pt==13"); >> xyVdM.txt

remove the *'s and column names from the text file. We would like to eventually use this file to get parameters for the likelyhood fit.