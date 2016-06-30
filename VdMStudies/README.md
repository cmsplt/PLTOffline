# VdMStudies
This directory contains results from the 2016 VdM track Analysis (Fill 4954) and also serves as a template for making similar studies for other fills.

##### Basics
1. `Slink_File` and `Gaincal_File` for a given fill
2. Generate Alignment_File using `./CalculateAlignment Slink_File Gaincal_File Alignment_IdealInstall.dat`
3. Change Trans_Alignment.dat generated from (2) to `Trans_Alignment_FillNo.dat`


## Track Parameters
We would like to investigate the track parameters. For a given fill, one may be interested in knowing how things scale during scans or if there's some weird thing happening to a partiular channel/quadrant at partiuclar time, etc. To that end, we first generate the `TrackParams.root` file by invoking the following:

`./TrackParams Slink_File Gaincal_File Trans_Alignment_FillNo.dat

Default track parameters are Beamspot, Slopes, Residuals, BX, Channels, TrackID. You would need to change `bin/TrackParams.cc` if you are interested in other variables (intercepts to Z=0 plane,X-Y at local telescope coordinates per ROC,etc). Fun!

### VdM Specific

For VdM scans, we may be interested in knowing tracks as a function of separation between Beam1 and Beam2. So, we will need scanpoint information--`scanY1.txt` (pointNum, timeFrom, timeTo, Separation). For further analysis of track parameters, we generate ttree per separation point by invoking the following:

```
root -l
.L VdMClass.C+
VdMClass m
m.Loop("scanY1")
```

Boom! You now have t4954scanY1.root file. TTree now has "pt" branch that tells you the separation between beams for each track.

#### Usage
Let's say you would like to look at SlopeY when beams were not separated i.e. 0 mm separation (pt==13) vs when they were separated by 0.344454 mm (pt ==20). 

```
root -l  t4954scanY1.root
perSCP->Draw("sy","pt==13")
perSCP->Draw("sy","pt==20","same")
```

# Likelyhood Fits
SBIL for VdM is close to 0. As we increase SBIL, what "extra" things do we see? Does luminosity scale linearly? What about second-order terms?...

Requirements: 
1. RooFit package
2. Track parameters at VdM, and at different SBIL

#### Generate text file for VdM 0 mm separation

```
root -l t4954scanpointY1.root
perSCP->SetScanField(0)
perSCP->Scan("sx:sy","pt==13"); >> xyVdM.txt
```

remove the *'s and column names from the text file. We would like to eventually use xyVdM.txt to get parameters for the likelyhood fit.


Similarly, for other fills::

```
root -l TrackParms.root
Tracks->SetScanField(0)
Tracks->Scan("SlopeX:SlopeY","TrackID==idnum"); >> xyFillNo_idnum.txt
```

#### TODO
Add Fit code
Add Toy_MC code
Add Plots

## Other fills
For other fills, we are interested in seeing how track parameters vary as a function of SBIL. The goal is to assign SBIL value to these tracks--trackClass.C and trackClass.h are for that purpose. 

We would need few things---TrackParams.root, DipFillNo.txt, the day for the fill, the number of colliding bunches.

1. TrackParamsFillNo.root:::./TrackParams Slink_File Gaincal_File TransAlignment_File
2. dipFillNo.txt::: Go to the page `ConditionBrowser/CMS_BEAM_COND/CMS_BRIL_LUMINOSITY/PLTZERO_INSTLUMI/` and make a plot. Click on text and save the resulting text file, remove the header lines and save as `dipFillNo.txt`.

 Before anything, you will need to change the pointer to the root file at ` TFile *f =... ` @ trackClass.h. You will also need to know the Epoch time from the start day of the Fill that you are inspecting. For example, Fill 5013 is from 2016-06-11. The 0th hour, 0th minute, 0th sec Epoch timestamp turns out to be 465603200 for that day. Change the timeOffset on trackClass.C to that value. We also know the number of colliding bunches in this fill is 2048.

Example usage

```
root -l
.L trackClass.C+
trackClass m
m.Loop("dip5013.txt",2028) 
```

This gives trackSBIL.txt file which tells us what the SBIL value was for each TrackID. Now, you're ready to make some likelyhood fits!

1. We have xyVdM.txt
2. xyFill_TrackID.txt
