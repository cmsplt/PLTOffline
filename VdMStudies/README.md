## PLT lumi correction via pixel data analysis


 Firstly, you will need track parameters for a given fill. bin/TrackParams.cc makes a ttree with slopes, residuals and groups tracks into 1 minute buckets. 
 We will eventually use this minute (trackID) to get the SBIL value to a group of tracks. Output is saved to TrackParams.root file.
 
 ```
 ./TrackParams
 Usage: ./TrackParams [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]
 ```
 
 
## Program Listing
| Code | Inputs | Outputs | Short Description|
|:--------|:---------|:-------|:--------|
|  |   |   |   |
| VdMClass.C | scanY1.txt,scanx1.txt,... | t4954scanY1.root| Organizes tracks per scan point|
| VdMSlopeXModel.C | Slope-x Data file from VdM 0mm separation  | plots, parameters  | fits data with slope-x model |
| VdMSlopeXModel.C | Slope-y Data file from VdM 0mm separation  | plots, parameters  | fits data with slope-y model |
|  |   |   |   |
| VdMfreezeSxSy.C | Slope-x, Slope-y data from various fills  | plots, parameters, frac3  | fits data with combined model  |
|  |   |   |   |
| trackClass.C | trackParams.root, dipFillNo.txt | trackSBIL.txt| Assigns SBIL value to each trackID|
|  |   |   |   |

We grab tracks from TrackParams.root file (dump them to text file, strip *'s) for a given trackID for likelihood fits. 
Since we would like to know the "extra stuff" as a function of SBIL, we need to associate the tracks with trackID to some SBIL value.
You do that via trackClass.C (edit the pointer to root file in trackClass.h, number of colliding bunches, edit epoch time under trackClass.Ce,and also get lumi values from dip). 
Rinse and repeat for many SBIL/Fills.

Example usage--2028 is the number of colliding bunches for fill 5013:
```
root -l
.L trackClass.C+
trackClass m
m.Loop("dip5013.txt",2028) 
```
Refer to READMEmore.md for a detailed description.
 
