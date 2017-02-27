# PLTOffline: PLT offline software for pixel data analysis

## Getting started

To clone this package, run `git clone https://github.com/cmsplt/PLTOffline.git` (https) or `git clone git@github.com:cmsplt/PLTOffline.git` (SSH). The latter requires you to have your SSH keys properly set up but once they are, you don't have to keep typing your password, which is rather convenient.

Once checked out, `source` the appropriate setup_* file for the machine and shell flavor you are using, or alternatively just set your PATH and LD_LIBRARY_PATH to point to your local installation of root.

Once you're set up, just compile with `make` (or `gmake`).

For this repository, the git policy is pretty simple. If you're making small changes, then it's OK to push directly to master (make your changes, commit them, then `git push origin master`). If you're making more substantial changes, please make a branch: `git checkout -b NewBranchName`, make your changes, commit them, `git push origin NewBranchName`, then go to the [PLTOffline web page on GitHub](https://github.com/cmsplt/PLTOffline) and make a pull request from your new branch. You can request a review from someone else so they can look over your change. When the pull request has been merged, switch back to master (`git checkout master`), pull your changes, and delete the branch that you don't need any more.

## Organization

This repository is organized somewhat unconventionally:
+ src/ contains the source files for the common classes that are used by the executables.
+ include/ contains the header files for the above.
+ bin/ contains the source files for the executables that perform a specific analysis
+ the actual executables themselves are compiled into the base directory of the package
+ plots (and sometimes ROOT files) produced by the executables are generally placed into plots/
+ plotScripts/ contains some scripts for making more plots
+ ALIGNMENT/, GainCal/, and TrackDistributions/ contain input files for analysis (further described below)
+ there are a few subdirectories (e.g. AccidentalStudies) devoted to particular analyses; these have further documentation inside

If you run an executable with no arguments, it should tell you what it needs as input. Example:

 ```
 ./MakeTracks
 Usage: ./MakeTracks [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]
 ```
**DataFile** is the binary Slink data file. The 2016 data files are stored on the RAID on april.cern.ch in /raid/PLT/SlinkData_2016. The 2015 data files are also stored in SlinkData_2015 but they have been gzipped to save space, so you'll need to unzip them somewhere before use.

**GainCal** is the gain calibration fit file. This is used to translate the raw ADC counts in the pulse height data to a given charge. The GainCal/ directory has the standard versions of these files as well as a README with more information about which ones you should use (for instance, the standard file for 2016 is GainCal/GainCalFits_20160501.155303.dat). See the description of GainCalFastFits below for more information about how to create these fits.

**AlignmentFileName** is an alignment file from the ALIGNMENT/ directory. See the README file there for recommendations about which to use (for example, the standard 2016 alignment file is ALIGNMENT/Trans_Alignment_4892.dat). There are also a few special files, such as Alignment_IdealInstall.dat, which assumes a "perfect" installation with the nominal PLT alignment. See the description of CalculateAlignment below for more information on how to create a new alignment file.

Some executables (e.g. MeasureAccidentals) may require other inputs, which are described below.

**VERY IMPORTANT:** Since running over an entire Slink file takes a very long time, many executables will only run on the first N events before stopping. Make sure to have a look and make sure that this limit (if any) is useful for your purposes!

## Code Basics

This section describes how the code works so you can use it to write your own analysis. The heart is the class **PLTEvent**. This takes care of reading events from files, applying the gain calibration and alignment, and performing clustering and tracking. Once this is done you can then do whatever you want with the processed event.

Basic usage:
```c++
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    // do your analysis here. for example, loop over telescopes or planes, and get hits, clusters, or tracks from them
  }
```

You can omit the GainCal and/or Alignment if they are not used in your analysis. When constructed, **PLTEvent** applies certain defaults for the fiducial region, clustering algorithm, and tracking algorithm. You can change these with the methods `SetPlaneFiducialRegion`, `SetPlaneClustering`, and `SetTrackingAlgorithm` respectively. (Note that the default is to not do tracking, so if you want tracks, you should change this!)

* Supported fiducial regions: kFiducialRegion_All (default) is the whole plane, kFiducialRegion_Diamond (obsolete) is the diamond sensor area, kFiducialRegion_m1_m1 is the whole plane minus the outermost row and column, kFiducialRegion_m2_m2 is the same except with 2 rows and columns, similarly for _m3_m3, _m4_m4, and _m5_m5. Since the advent of masking the concept of FiducialRegion is a bit obsolete -- for most purposes it's recommended just to stick with kFiducialRegion_All and mask as necessary.

* Supported clustering algorithms: kClustering_AllTouching (default) clusters all adjacent hits into a single cluster, kClustering_NoClustering doesn't do any clustering, kClustering_OnePixOneCluster takes each individual pixel and builds a cluster for that pixel, kClustering_Seed_3x3 clusters using a seed which is the largest in a 3x3 area, similarly for _5x5 and _9x9. AllTouching is the most stable and is recommended for most uses.

* Supported tracking algorithms: kTrackingAlgorithm_NoTracking (default) does no tracking. kTrackingAlgorithm_01to2_All ("standard" tracking) looks for pairs in the first two planes and then searches for a hit in the third plane within a (very loose) distance from the extrapolated stub. Overlaps are removed from the resulting set of tracks by selecting the one with the best chi-squared. kTrackingAlgorithm_01to2_AllCombs (tracking for accidental measurement) usese the same algorithm, but there is no distance cut applied on the third plane and all combinations are kept.

* Masking: To apply a specific mask, use ReadPixelMask() or ReadOnlinePixelMask(). ReadPixelMask() reads a mask in the offline format (lines of the form `[pixFEDChannel] [ROC] [column] [row]`). ReadOnlinePixelMask() reads a mask in the same format as used for the online (lines of the form `[mFEC] [mFECChan] [hubId] [ROC] [col] [row] [enablePix]`, and also supports ranges in the column and row number fields). It uses the gain calibration to translate the hardware address to the FED channel, so you'll need to use a form of **PLTEvent** which applies the gain calibration. In either case, the mask operates simply by discarding any hits read in the masked-out area.

The standard classes that do the work for PLTEvent are pretty self-explanatory, so here's a quick rundown:
* **PLTHit** contains the information for a single hit: location (in local and global coordinates) and charge.
* **PLTCluster** similarly contains the information for a single cluster, including pointers to the component hits.
* **PLTPlane** is the class for a single plane, and contains all the hits and clusters on that plane. The actual clusterization of hits is done in this class as well.
* **PLTTelescope** is the class for a single telescope (arrangement of three planes). It contains pointers to the individual planes, and to the tracks that have been found in that telescope (once tracking has been run).
* **PLTTrack** is the class for a single track (a line passing through three clusters, one on each plane in a telescope). It contains the individual clusters and the track parameters.
* **PLTBinaryFileReader** does the actual work of reading and parsing the input Slink binary file. It parses a single event, extracts the timestamp, event number, bunch crossing number, and individual hits, and passes them to **PLTEvent** to perform the rest of the processing. Despite the name, it can also read text files for debugging purposes.
* **PLTGainCal** handles the work of the gain calibration. It can read in GainCal files in a variety of formats and, once the gain calibration has been loaded, it can be used to translate a raw pulse height into a charge.
* **PLTAlignment** handles the work of the alignment. It reads in the alignment files, and once they have been loaded, can be used to align hits and clusters from local coordinates to global coordinates.
* **PLTTracking** does the actual tracking. It finds tracks using the specified tracking algorithm and creates a set of **PLTTrack** objects in the event that can then be studied.
* **PLTU** contains various utility functions. This is also where the various constants defining the properties of the PLT are declared.
* **PLTTesterEvent** and **PLTTesterReader** are special forms of PLTEvent and PLTBinaryFileReader for reading the data from the teststand.
* **PLTHistFileReader** is for reading certain histogram files -- I don't know where these histogram files come from however!

# Program Listing

This section lists the various executables in the bin/ directory that can be used for various analyses. The parentheses show what arguments are necessary -- (D) denotes just a Slink data file, (DG) means a Slink data file and a gain calibration, (DGA) means a Slink data file, gain calibration, and alignment file, and (DGAT) means the preceding plus a track distribution file.

## Core Utilities

* **OccupancyPlots** (D) is the simplest way of looking at the data. It just plots the occupancy for each pixel using the hit data from the Slink file. It makes a larger number of plots in plots/Occupancy_* of the raw occupancy histograms, the occupancy by quantiles (to reduce the effect of hot pixels), the 3x3 occupancy, 1D projections, and so forth. The ROOT data file with the information for the plots is saved as histo_occupancy.root.

* **GainCalFastFits** is used to generate the GainCalFits.dat files used in nearly all analyses. You need as input a gaincalfast_YYYYMMDD.hhmmss.avg.txt file that was created by running the GainCal calibration online. Copy this to april (there are some 2015 and 2016 files in /data2/GainCal_2015 and /data2/GainCal_2016) and then run this utility. It will take about 30 minutes and then produce a GainCalFits_YYYYMMDD.hhmmss.dat file, a GainCalFits_YYYYMMDD.hhmmss.root file, and a large number of summary plots in the plots/GainCal directory. The .dat file is then used as input for the other executables that use the gain calibration; if you plan on using it further, it's recommended that you store it in GainCal/. The .root file contains all of the data about the calibration, including the specific fits per pixel and the overall summary plots.

* **CalculateAlignment** (DGA) is used to generate the alignment used by the PLT tracking. It proceeds in three passes: first it corrects the rotational offset of the second and third planes relative to the first, and then it runs a second pass applying that rotation to correct the translational offset of the second and third planes relative to the first. Finally in the third pass it applies all the corrections to examine the remaining residuals. It produces ROTATED_Alignment.dat, which is a .dat file containing the results of the alignment after the first pass, and Trans_Alignment.dat, a .dat file containing the results of the alignment after the second pass. The latter is the file that can be used as the alignment input for further analyses (again, if you want to save it for further use, it's recommended that you save it as ALIGNMENT/Trans_Alignment_[FillNumber].dat). The plots are saved in plots/Alignment/ and the ROOT file containing the plot data is saved in histo_calculatealignment.root. The input alignment file is used as a first guess for starting the alignment process -- you can use the previous alignment file or ALIGNMENT/Alignment_IdealInstall.dat.

* **MakeTracks** (DGA) is used to study the tracking. It runs the tracking on a given data file and selects only "pure" tracks -- an event where there is exactly one cluster per telescope -- and then plots the resulting distribution of track parameters (slope for each telescope, and residuals in X and Y for each plane). Also plots the beam spot by projecting the tracks back to the origin. Other programs which study the tracks (like MeasureAccidentals) are based on this program. Creates histo_slopes.root and plots in plots/Slope*.gif, plots/Residual*.gif, and plots/BeamSpot.gif.

* **TrackOccupancy** (DGA) makes occupancy plots similar to those in OccupancyPlots, but only considers hits which are part of reconstructed tracks using the tracking. This can be used to study the alignment and the mask alignment. **IMPORTANT:** The mask for the central plane is hard-coded: if the position of the hit on the central plane falls outside the mask the track will be discarded. Please be aware of this if the mask position changes! Produces plots in plots/TrackOccupancy*.gif (the actual occupancy plots), Tracks_ChXX_EvYY.gif (the raw hit data for the first few individual tracks in the file), and histo_track_occupancy.root, which contains the plot data.

* **MeasureAccidentals** (DGAT)

## Other Utilities

* **TrackParams** (DGA) runs the tracking on a given data file and saves the track parameters to a TTree for further analysis. Creates TrackParams.root, which contains this tree.

## Old Utilities

These utilities are from the pilot run and may not have been updated since then (many of them still use the diamond fiducial region, for example). There's no documentation so I've put my best guess here. Use at your own risk!

* **dumpPixffil** (D) and **dumpPixffile_orig** (D) dump the contents of a raw data file. These are from the Pixel software.
* **MakeIdealAlignment** makes a dummy ideal alignment file for the actual PLT installation (identical to ALIGNMENT/Alignment_IdealInstall.dat).
* **MakeStraightAlignment** makes a dummy alignment file with all telescopes aligned in the same place (identical to ALIGNMENT/Old/Alignment_Straight.dat).
* **UltraBlackFinder** takes a pixel FED transparent buffer and finds the ultrablacks in it. Much the same as similar routines in the online software (and I'm not sure why one would need it in the offline software).
* **Verbose** (DGA) simply runs the clustering and tracking in PLTEvent and then prints out the resulting hit, cluster, and track information.

## Even Older Utilities

The Archive/ directory contains retired code which is no longer used. There's no documentation, so you're on your own here.
