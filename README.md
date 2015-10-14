# PLTOffline
PLT Offline Software
For PLT pixel data analysis

To use the package, _source_ the appropriate setup_* file for the machine you are using, or open on and export your PATH
and LD_LIBRARY_PATH to point to your local installation of root. 

# compile with _make_
.exe are made from the .cc files in ./bin/ and placed in the "home" directory of the package
.h files are in ./include/
source files are in ./src/

 run an exe with no arguments to print out a list of files that are required as input
 ```
 ./MakeTracks
 Usage: ./MakeTracks [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]
 ```
<b> DataFileName </b> 
means a binary Slink file. names are of the form Slink_DATE_TIME.dat the current datasets (2015) are stored on april.cern.ch:/data2/Data_2015/

<b> GainCalFileName </b>
means a fitted gain calibration file. these are created by the user by running ./GainCalFastFits on a gaincal_DATE_TIME.avg.txt file.  the *.avg.txt files are stored in april.cern.ch:/data2/GainCal/  it is recomended to rename the GainCalFits.dat which is created by this process to GainCalFits_DATE_TIME.dat (do the same for the root file) so that the user knows which gain cal it was fitted from.  this might eventually become automatic

<b> AlignmentFileName </b>
is an alignment file from the ./ALIGNMENT/ directory.  The alignment file used most frequently and for is called Alignment_IdealInstall.dat this assumes an ideal hardware configuration and works for any sort of study except track slope, and residual stuides. of course the hardware configuration is not perfect, so one can calculate the actual alignment by running ./CalculateAlignment and passing it IdealInstall.dat as an input. this will output a corrected alignment file into the "home" directory of the package which can be used for track slope studies.
  
some exe's output root files and all exe's output plots to the ./plots/ directory

#Program Listing
| Executable Name | Inputs | Outputs | Short Description|
|:--------|:---------|:-------|:--------|
|  |   |   |   |
| GainCalFastFits | gaincal_*.avg.txt | GainCalFits.dat plots/GainCal/*.gif | Uses a gaincal_DATETIME.avg.txt file to fit the saturation curves for each pixel. for a good gain cal, as of oct2015 will take 15-20 mins. The output, GainCalFits.dat is used as input for other executables in the package |
| CalculateAlignment | Data, GainCal, Alignment  | TransAlignment.dat, ROTAlignment.dat, plots/Alignment/*.gif, calculatealignment.root  | lines up pixel hits from tracks behind each other by moving the planes. fixes rotational offsets first, outputs ROTAlignment.dat. Then fixes translational misalignment and ouptus TRANSAlignment.dat. I would suggest using ALIGNMENT/Alignment_IdealInstall.dat as the input alignment file at first, until you have a better starting point guess.  Make sure the alignment file has the same channel numbers as the gain cal was made with. |
|  |   |   |   |
|  |   |   |   |
|  |   |   |   |


