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
+ there are a few subdirectories (e.g., AccidentalStudies) devoted to particular analyses; these have further documentation inside

If you run an executable with no arguments, it should tell you what it needs as input. Example:

 ```
 ./MakeTracks
 Usage: ./MakeTracks [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]
 ```
The most common inputs are:

**DataFile** is the binary Slink data file. The 2016 data files are stored on the RAID on april.cern.ch in /raid/PLT/SlinkData_2016. The 2015 data files are also stored in SlinkData_2015 but they have been gzipped to save space, so you'll need to unzip them somewhere before use.

**GainCal** is the gain calibration fit file. This is used to translate the raw ADC counts in the pulse height data to a given charge. The GainCal/ directory has the standard versions of these files as well as a README with more information about which ones you should use (for instance, the standard file for 2016 is GainCal/GainCalFits_20160501.155303.dat). See the description of **GainCalFastFits** below for more information about how to create these fits.

**AlignmentFile** is an alignment file; these are normally stored in the the ALIGNMENT/ directory. See the README file there for recommendations about which to use (for example, the standard 2016 alignment file is ALIGNMENT/Trans_Alignment_4892.dat). There are also a few special files, such as Alignment_IdealInstall.dat, which assumes a "perfect" installation with the nominal PLT alignment. See the description of **CalculateAlignment** below for more information on how to create a new alignment file.

**TrackDistributions** is a distribution of track parameters. Some standard files are stored in TrackDistributions/, and TrackDistributions/TrackDistributions_MagnetOn2016_4892.txt is the standard for 2016 running. See the description of **MeasureAccidentals** below for more about these files.

**VERY IMPORTANT:** Since running over an entire Slink file takes a very long time, many executables will only run on the first N events before stopping. Make sure to have a look and make sure that this limit (if any) is useful for your purposes!

## Code Basics

This section describes how the code works so you can use it to write your own analysis. The heart is the class **PLTEvent**. This takes care of reading events from files, applying the gain calibration and alignment, and performing clustering and tracking. Once this is done you can then do whatever you want with the processed event.

Basic usage:
```c++
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    // do your analysis here. for example, loop over telescopes or planes,
    // and get hits, clusters, or tracks from them
  }
```

You can omit the GainCal and/or Alignment if they are not used in your analysis (but you need a gain calibration in order to do clustering, and alignment in order to do tracking). When constructed, **PLTEvent** applies certain defaults for the fiducial region, clustering algorithm, and tracking algorithm. You can change these with the methods `SetPlaneFiducialRegion`, `SetPlaneClustering`, and `SetTrackingAlgorithm` respectively. (Note that the default is to not do tracking, so if you want tracks, you should change this!)

By default, **PLTEvent** will read from a binary file whose name is specified in `DataFileName`. You can also specify `kTextFile` as the last argument to read a text file, or `kBuffer` to read from a buffer (which you supply to `Event.GetNextEvent()` as `Event.GetNextEvent(buf, bufSize)`).

* Supported fiducial regions: `kFiducialRegion_All` (default) is the whole plane, `kFiducialRegion_Diamond` (obsolete) is the diamond sensor area, `kFiducialRegion_m1_m1` is the whole plane minus the outermost row and column, `kFiducialRegion_m2_m2` is the same except with 2 rows and columns, and similarly for `_m3_m3`, `_m4_m4`, and `_m5_m5`. Since the advent of masking the concept of FiducialRegion is a bit obsolete -- for most purposes it's recommended just to stick with `kFiducialRegion_All` and mask as necessary.

* Supported clustering algorithms: `kClustering_AllTouching` (default) clusters all adjacent hits into a single cluster, `kClustering_NoClustering` doesn't do any clustering, `kClustering_OnePixOneCluster` takes each individual pixel and builds a cluster for that pixel, `kClustering_Seed_3x3` clusters using a seed which is the largest in a 3x3 area, similarly for `_5x5` and `_9x9`. `kClustering_AllTouching` is the most stable and is recommended for most uses.

* Supported tracking algorithms: `kTrackingAlgorithm_NoTracking` (default) does no tracking. `kTrackingAlgorithm_01to2_All` ("standard" tracking) looks for pairs in the first two planes and then searches for a hit in the third plane within a (very loose) distance from the extrapolated stub. Overlaps are removed from the resulting set of tracks by selecting the one with the best chi-squared. `kTrackingAlgorithm_01to2_AllCombs` (tracking for accidental measurement) usese the same algorithm, but there is no distance cut applied on the third plane and all combinations are kept.

* Masking: To apply a specific mask, use `ReadPixelMask()` or `ReadOnlinePixelMask()`. `ReadPixelMask()` reads a mask in the offline format (lines of the form `[pixFEDChannel] [ROC] [column] [row]`). `ReadOnlinePixelMask()` reads a mask in the same format as used for the online (lines of the form `[mFEC] [mFECChan] [hubId] [ROC] [col] [row] [enablePix]`, and also supports ranges in the column and row number fields). It uses the gain calibration to translate the hardware address to the FED channel, so you'll need to use a form of **PLTEvent** which uses the gain calibration. In either case, the mask operates simply by discarding any hits read in the masked-out area.

The standard classes that do the work for PLTEvent are pretty self-explanatory, so here's a quick rundown:
* **PLTHit** contains the information for a single hit: location (in local and global coordinates) and charge.
* **PLTCluster** similarly contains the information for a single cluster, including pointers to the component hits.
* **PLTPlane** is the class for a single plane, and contains all the hits and clusters on that plane. The actual clusterization of hits is done in this class as well.
* **PLTTelescope** is the class for a single telescope (arrangement of three planes). It contains pointers to the individual planes, and to the tracks that have been found in that telescope (once tracking has been run).
* **PLTTrack** is the class for a single track (a line passing through three clusters, one on each plane in a telescope). It contains the individual clusters and the track parameters.
* **PLTBinaryFileReader** does the actual work of reading and parsing the input Slink binary file. It parses a single event, extracts the timestamp, event number, bunch crossing number, and individual hits, and passes them to **PLTEvent** to perform the rest of the processing. Despite the name, it can also read text files or from buffers.
* **PLTGainCal** handles the work of the gain calibration. It can read in GainCal files in a variety of formats and, once the gain calibration has been loaded, it can be used to translate a raw pulse height into a charge.
* **PLTAlignment** handles the work of the alignment. It reads in the alignment files, and once they have been loaded, can be used to align hits and clusters from local coordinates to global coordinates.
* **PLTTracking** does the actual tracking. It finds tracks using the specified tracking algorithm and creates a set of **PLTTrack** objects in the event that can then be studied.
* **PLTU** contains various utility functions. This is also where the various constants defining the properties of the PLT are declared.
* **PLTTesterEvent** and **PLTTesterReader** are special forms of **PLTEvent** and **PLTBinaryFileReader** for reading the data from the teststand.
* **PLTHistFileReader** is for reading certain histogram files -- I don't know where these histogram files come from however!

# Program Listing

This section lists the various executables in the bin/ directory that can be used for various analyses. The parentheses show what arguments are necessary -- (D) denotes just a Slink data file, (DG) means a Slink data file and a gain calibration, (DGA) means a Slink data file, gain calibration, and alignment file, and (DGAT) means the preceding plus a track distribution file.

## Core Utilities

These are the most frequently used analysis programs.

* **OccupancyPlots** (D) is the simplest way of looking at the data. It just plots the occupancy for each pixel using the hit data from the Slink file. It makes a larger number of plots in plots/Occupancy_* of the raw occupancy histograms, the occupancy by quantiles (to reduce the effect of hot pixels), the 3x3 occupancy, 1D projections, and so forth. The ROOT data file with the information for the plots is saved as histo_occupancy.root. The script **plotScripts/MakeNiceOccupancy.C** can be used to make somewhat nicer plots from this ROOT file (suitable for approvals).

* **GainCalFastFits** is used to generate the GainCalFits.dat files used in nearly all analyses. You need as input a gaincalfast_YYYYMMDD.hhmmss.avg.txt file that was created by running the GainCal calibration online. Copy this to april (there are some 2015 and 2016 files in /data2/GainCal_2015 and /data2/GainCal_2016) and then run this utility. It will take about 30 minutes and then produce a GainCalFits_YYYYMMDD.hhmmss.dat file, a GainCalFits_YYYYMMDD.hhmmss.root file, and a large number of summary plots in the plots/GainCal directory. The .dat file is then used as input for the other executables that use the gain calibration; if you plan on using it further, it's recommended that you store it in GainCal/. The .root file contains all of the data about the calibration, including the specific fits per pixel and the overall summary plots.

* **CalculateAlignment** (DGA) is used to generate the alignment used by the PLT tracking. It proceeds in three passes: first it corrects the rotational offset of the second and third planes relative to the first, and then it runs a second pass applying that rotation to correct the translational offset of the second and third planes relative to the first. Finally in the third pass it applies all the corrections to examine the remaining residuals. It produces ROTATED_Alignment.dat, which is a .dat file containing the results of the alignment after the first pass, and Trans_Alignment.dat, a .dat file containing the results of the alignment after the second pass. The latter is the file that can be used as the alignment input for further analyses (again, if you want to save it for further use, it's recommended that you save it as ALIGNMENT/Trans_Alignment_[FillNumber].dat). The plots are saved in plots/Alignment/ and the ROOT file containing the plot data is saved in histo_calculatealignment.root. The input alignment file is used as a first guess for starting the alignment process -- you can use the previous alignment file or ALIGNMENT/Alignment_IdealInstall.dat. The script **plotScripts/PlotAlignmentVsTime.C** can be used to compare different alignment files so you can observe changes over time.

* **MakeTracks** (DGA) is used to study the tracking. It runs the tracking on a given data file and selects only "pure" tracks -- an event where there is exactly one cluster per telescope -- and then plots the resulting distribution of track parameters (slope for each telescope, and residuals in X and Y for each plane). Also plots the beam spot by projecting the tracks back to the origin. Other programs which study the tracks (like MeasureAccidentals) are based on this program. Creates histo_slopes.root and plots in plots/Slope\*.gif, plots/Residual\*.gif, and plots/BeamSpot.gif.

* **TrackOccupancy** (DGA) makes occupancy plots similar to those in OccupancyPlots, but only considers hits which are part of reconstructed tracks using the tracking. This can be used to study the alignment and the mask alignment. **IMPORTANT:** The mask for the central plane is hard-coded: if the position of the hit on the central plane falls outside the mask the track will be discarded. Please be aware of this if the mask position changes! Produces plots in plots/TrackOccupancy*.gif (the actual occupancy plots), Tracks_ChXX_EvYY.gif (the raw hit data for the first few individual tracks in the file), and histo_track_occupancy.root, which contains the plot data.

* **MeasureAccidentals** (DGAT) is the main utility for measuring the accidental rate. It performs the tracking using all possible combinations of hits with no quality cuts, and then looks to see if it can find at least one "good" track among the combinations. If so, the telescope is marked as good for that event; otherwise, the hits are taken to be accidental. A good track is defined as one for which all of the track parameters (slope X/Y, and residual X/Y in planes 0/1/2) are within 5 sigma of the mean values. These parameters are defined in a track distributions file, which is an additional input to the script and can be found in the TrackDistributions/ directory. You can also run MeasureAccidentals without an input track distributions file, in which case it won't compute the accidental rates but will produce an output TrackDistributions_[FillNumber].txt file which can be used for further running. The main output of the script is AccidentalRates_[FillNumber].txt, which contains the accidental data by time period. It also creates a histo_slopes.root file and plots like **MakeTracks**. **Note:** While the accidental rate is computed using all tracks, the track distribution file is derived only from the "pure" track sample; i.e., events where there is exactly one cluster in each plane in the telescope.

  By default **MeasureAccidentals** will split the input file into five-minute chunks and compute the accidental rate for each. You can also specify a timestamp file as input (e.g., if you have a scan and want to measure the accidental rate step by step). See the AccidentalStudies/ directory for much more documentation, as well as scripts which process and plot the output text files.

  You can also specify a mask file to be applied by giving it as the last argument. If you want to specify a mask file without using a timestamp file, just use a minus sign "-" as the timestamp file argument. See AccidentalStudies/MaskStudies/ for more information on these studies.

  **MeasureAccidentalsTele** and **MeasureAccidentalsBX** are variants of **MeasureAccidentals** which measure the accidental rate on a per-telescope and per-BX basis, respectively. **MeasureAccidentalsTele** works pretty much the same, except that the output file contains individual data for each telescope instead of an overall rate. **MeasureAccidentalsBX** is also similar except that the Slink data does not have enough statistics to do a per-BX measurement at 5-minute granularity, so it instead averages over the entire fill.

* **TrackLumiZeroCounting** (DGAT) is a utility to calculate luminosity from the track counting. Like **MeasureAccidentals**, it starts out by performing the tracking and looking to see if one good combination is found. The number of events where at least one good track is found and the number of events where no good tracks are found is then written out, and these can be used to calculate the luminosity. Since we can't do a per-BX measurement using the pixel data, the script only considers BXes which are filled, and so first must determine the fill pattern from the data. Also the script needs to deal with cases where a telescope drops out, so it keeps track of the data from each telescope and if it doesn't receive any, that telescope is excluded from the luminosity calculations. It also accounts for cases where a telescope becomes desynchronized and so the BX data from that telescope is no longer reliable. It produces the same output files as **MeasureAccidentals**.

  The directory `TrackLumiZC/` contains further documentation and utilities for processing and plotting the results produced by this script. **TrackLumiZeroCountingVdM** is a variant with somewhat different thresholds for determining a channel dropout suitable to the lower luminosity of the VdM scan. **TrackLumiZeroCountingBXVdM** is a variant which calculates a per-BX luminosity (also using the VdM setup); like **MeasureAccidentalsBX**, it requires a full fill's worth of data. Note that you will need to modify the channels that are actually written out at the top of the script.

   In 2020, I did some additional work on this luminosity measurement. The scripts and results for this are in the directory `TrackLumi2020/`. See the README in that directory for more documentation and information on these scripts.

* **ParseCondDBData** is a utility for use with **MeasureAccidentals** and **TrackLumiZeroCounting**. It takes a CSV file of the online luminosity from the conditions browser and a text file output by one of the previous scripts, and combines the two so that you can look at the accidental rate or track lumi as a function of the online fast-or luminosity. See the documentation in MeasureAccidentals/README for more details on how to use this script. (Note that the script only cares about the timestamp in the text file -- all other data will simply be copied over to the output file.)

* **PulseHeights** (DG) makes a plot of the pulse heights using the gain calibration to translate the raw ADC counts into a charge. It produces two sets of plots: plots/ClusterSize_ChXX.gif simply shows the distribution of number of pixels per cluster in each ROC, and plots/PulseHeight_ChXX.gif shows the distribution of charge for single-pixel, two-pixel, and three-or-more-pixel clusters, the average pulse height as a function of time, and a 2-D histogram showing the average charge per pixel. **PulseHeightsBorder** is a variant which looks at the pulse heights only for pixels on the border of the active area or not on the border, given an input mask.

* **TrackingEfficiency** (DGA) measures the efficiency to reconstruct a track. Specifically, it looks for events with hits in at least two planes, and measures the rate with which a hit in the third plane consistent with a track through the first two hits is found. These plots are saved in plots/TrackingEfficiency\*.gif.

## Other Utilities

These are more special-purpose utilities, but they have been written or updated recently and should still work without additional changes.

* **CalculateEventSize** (D) is a simple script to calculate the average event size (total number of hits per event) both overall and per-channel.
* **checkOnlineMaskSize.pl** is a script which checks an online mask file and prints out the size of the masked region, so you can check that it is the desired size. **checkOfflineMaskSize.pl** is the same but for an offline format mask file.
* **ClusterLumi** (D) is an attempt to measure the luminosity using cluster counting (like the PCC technique). This makes a ntuple in the output file clusterLumi.root which can then be used as input to **ReadClusterLumi**, which reads the ntuple and does the actual luminosity calculation. It then produces the final plots in the output file readClusterLumi.root. These results can be plotted using the script **plotScripts/MakeClusterLumiPlots.C**. This technique ended up being not very successful and was largely superseded by **TrackLuminosityZeroCounting**.
* **DetectFailure** (D) looks for times in a data file when a telescope has dropped out of the Slink readout by looking for periods where no data is received from a scope. The logic from this script was later merged into **TrackLumiZeroCounting**.
* **DoubleColumnMC** generates a Monte Carlo simulation to try to measure the effect of double-column readout inefficiencies. Specifically, on average it takes 6 clock cycles to drain a single double column (DC). If the same DC is hit during the drain, the hit can be queued, but only 3 hits can be queued, so any further beyond that will be lost. This Monte Carlo attempts to simulate that effect. Based on **PLTMC**.
* **FindFilledBX** (D) looks at the BX data in the Slink to determine which bunches are filled in the data (by looking for the BX with the highest hit rate and then taking any BX with a hit rate > 0.05*max as "filled") and then shows the results, so we can see if the data is misaligned with our expectation.
* **FindLeadingBX** (D) is similar to the above but also looks for specifically leading bunches in the data, if you want to do an analysis that focuses on these bunches only.
* **HitsVsEvent** (D) reads a given segment of a data file and prints out the number of hits in each event over that time period.
* **MeasureMissRate** (D) is an attempt to simulate the "missing triplet" problem that we experienced in 2015 where planes with three or more hits would not be correctly recognized by the fast-or FED when looking for triple coincidences. This is difficult to simulate in the pixel data because the fast-or algorithm in the ROCs counts hits in adjacent columns as a single hit sometimes, but not always, so the script tries several different algorithms in the pixel data to try to match what the fast-or data produces. Produces MissingHitsPixel.txt, a set of rates for each of the different algorithms in five-minute intervals.
* **OccupancyVsTime** (D) makes a plot of the occupancy in each telescope and plane as a function of time. Like **MeasureAccidentals**, it defaults to 5-minute increments but you can also give it a timestamp file to run in specified intervals. Produces plots/occupancy_vs_time.root, which contains the target plots.
* **PlotActiveBunches** (D) makes a plot of the average number of hits vs. BX and uses this plot to determine the number of filled BXes in the fill.
* **PlotBXDistribution** (D) simply looks at the BX value for each event and makes a plot of the resulting distribution. This can be used to determine if the random/cycling trigger actually is uniform, or conversely to see if the fast-or coincidence trigger is correctly triggering on filled bunches.
* **PlotErrorRates** (D) makes a plot of the rates for each of the six basic types of errors per minute over the course of a fill.
* **PLTGeometryExport** (A) sets up the PLT geometry using the given alignment file and exports it as a ROOT and XML file, for use in generating Monte Carlo.
* **PLTHitDisplay** (DGA) produces a 3-D event display which shows the hits for a given event.
* **RawADCValues** (D) is a utility for looking at the distribution of the raw ADC values for the pulse heights. It works similarly to **OccupancyPlots** but instead of plotting the occupancy simply plots the average ADC value for each pixel.
* **RecenterMask.C** is a ROOT macro for performing the mask recentering at the beginning of the year (or any other time the alignment may have changed significantly). To use it, first run **TrackOccupancy**, and then run this script. It will attempt to find the best placement for the masks on the outer planes given the hit data, and you can also adjust it manually. When complete, it will produce Mask_Recentered.txt, which you can then use as a new mask (be aware that you will have to add things like masks for individual noisy pixels manually).
* **SelectTimeRange** (D) will take an input data file, an output file, a start time, and an end time, and write only the events from the input file between the star and end time (inclusive) to the output file.
* **TelescopeRotation** (DGA) makes plots of the slopes, X and Y residuals, and XdY and YdX slopes for tracks. I'm not sure if it does anything that's not already done by **CalculateAlignment**, however.
* **TrackDiagnostics** (DGA) makes some more specific tracking plots -- in addition to the standard slopes, it also makes slope plots for tracks containing 2 or 3 clusters, as well as the overall cluster distribution.
* **TestBinaryFileReader** (D) will read a binary data file and print out the decoded information. It can either print a brief summary for each event or the entire event information with the decoded value for each word. _Also present in the online archive._
* **TestBXPattern** (D) will read a binary data file and print out the rate of triggers for each BX and the average number of hits in each BX. Can be used to make sure that the trigger is functioning as desired (especially useful for special VdM triggers). _Also present in the online archive._
* **TrackParams** (DGA) runs the tracking on a given data file and saves the track parameters to a TTree for further analysis. Creates TrackParams.root, which contains this tree.
* **TrackMC** generates a Monte Carlo simulation to measure the effect of pixel inefficiency on the PLT efficiency. Specifically, it models the effect that if the same pixel is hit a second time before the hit information is copied to the periphery (about 6 clocks), the hits will be lost. The program uses **PLTMC** to generate some random tracks and then applies this effect to see how the triple coincidence rate is affected. Produces an output binary file in the Slink format, which can be used for further studies.
* **TrackStudies** (DGA) produces a file showing how different track quantities change over time -- number of tracks, number of clusters, number of extra clusters, and number of events with no tracks.

## Old Utilities

These utilities are from the pilot run and may not have been updated since then (many of them still use the diamond fiducial region, or still have some hard-coded set of channels, for example), but they may still contain useful code. There's no previously existing documentation so I've put my best guess here. Use at your own risk!

* **BasicResiduals** (DG) looks for events with one cluster per telescope and plots the difference in position between the clusters.
* **BinaryHitCounter** (D) dumps the number of hits found in each event in the input data file (at least it does if you uncomment the std::cout statement).
* **CheckAlignment** (DGA) applies the alignment to an input file and makes a bunch of plots showing the results.
* **CheckGainCalCoefs** (G) checks a gain calibration file to see if there are any pixels for the given channel which are missing a fitted calibration curve.
* **ConvertGainCalCoefs** (G) converts a gain calibration file with hardware addresses into one with pixel FED channel number. Note that the conversion is hard-coded in the script and not at all correct for production running.
* **DeanTextToBinary** converts a text event file (used for debugging) into a binary file with the same format as the regular Slink data.
* **dumpPixffil** (D) and **dumpPixffile_orig** (D) dump the contents of a raw data file. These are from the Pixel software.
* **FindNoisyPixels** (D) looks for noisy pixels by looking at the occupancy plots and identifying any pixel with an occupancy more than 5 sigma above the median as noisy.
* **FindTrims** takes an input file of efficiency vs. trim values per pixel and tries to find the ideal trim values. The online software probably does a better job of this nowadays.
* **GetParamsFromFile** (G) reads in a gain calibration file and prints out the coefficients of the fitted calibration curve for a given channel, ROC, and pixel.
* **HistNTP** reads an unknown histogram file and makes a graph of the entries by channel and BX.
* **MakeIdealAlignment** makes a dummy ideal alignment file for the actual PLT installation (identical to ALIGNMENT/Alignment_IdealInstall.dat).
* **MakeIdealGainCal** makes a dummy gain calibration file with the same calibration curve used for all pixels on all ROCs.
* **MakeLumiGraphs** makes plots of luminosity versus time from an unknown histogram file.
* **MakeStraightAlignment** makes a dummy alignment file with all telescopes aligned in the same place (identical to ALIGNMENT/Old/Alignment_Straight.dat).
* **MakeTrimCalFileFake** makes a dummy trim calibration results file with randomly generated efficiencies, which presumably can be used as input to **FindTrims**.
* **NumberOfHitsPerEvent** makes histograms of the number of hits per event, number of clusters per event, and number of hits per cluster for each plane.
* **OccupancyPlots3x3** (D) makes occupancy plots and the plots of 3x3 efficiency. Since **OccupancyPlots** also makes 3x3 efficiency plots I'm not quite sure what this script does that **OccupancyPlots** doesn't, though.
* **OverlayPHLumi** takes a pulse height histogram file and a lumi histogram file and plots them together. I don't know how you would make either of the two input files however.
* **Plot3ParamFit** will simply make a plot of a gain calibration curve using the provided constants. Despite the name it can plot either a 3-parameter or a 5-parameter fit curve.
* **PlotADCForVCal** plots the response in ADC counts for a given VCal given a text input file.
* **PlotBuckets** plots the luminosity as a function of BX given an input histogram file of unknown format (of the kind read by **PLTHistReader**).
* **PlotEvent** (DG) takes a single event from a Slink data file and makes a plot of the hit pixels for that event.
* **PlotFedTime** takes the output of the online FED time calibration and makes a plot of the results. Probably better to use the utilities in the online software for this.
* **PlotPixelVsTimeMagTest** takes an input file of unknown format and an input file of magnet data and plots the results as a function of time.
* **PlotTemperature** takes an input file of temperature against time and plots the results.
* **PlotThresholds** takes a file from the output of a threshold calibration and plots the results.
* **PlotTrims** takes a trim file and plots the trim values as a function of pixel.
* **PLTDatToWave** takes a pixel FED transparent buffer (from an ultrablack calibration, for example) and plots the resulting waveform. This is a little redundant with what the online software does.
* **PLTEventDisplay** (DGA) makes a 3-D event display of events in the PLT with their tracks.
* **PLTMC** creates simulated hits and/or tracks and generates a binary file (in the same format as the Slink data) with these simulated hits. It supports a wide variety of different kinds of tracks that can be generated (from the IP, parallel to the beamline, random slopes, etc.) but has not been updated since the pilot run, so it would probably need some cleaning up to make usable again.
* **PrintBXTracks** (DGA) prints out the number of tracks per BX for the whole PLT and for each individual channel.
* **PrintEventNumbers** (D) does what it says: it takes an input data file and, for every million events, prints out the event number and timestamp.
* **PrintTextEvents** (D) takes an input binary data file and makes an output file containing the hit information in the events converted to text.
* **ProcessHistograms** takes an input histogram file (of unknown format) and plots the resulting total and by-channel luminosity.
* **PulseHeightsTrack** (DGA) is like **PulseHeights** but only plots hits which make up a valid track. It probably still works fine but I haven't tested it at all. **PulseHeightsSignalTrack** (DGA) is similar but applies some cuts on the slope in order to select only good tracks. These are all hard-coded so it would definitely need some updating in order to be usable.
* **ReadEvent** (D) reads a binary file and dumps the event header and hit data to the screen.
* **RunBasicTeststandAnalysis** takes the teststand data and makes some basic occupancy, levels, and pulse height plots from it. Makes plots and a HTML file suitable for posting.
* **RunTeststandGainCal** creates a (3-parameter) gain calibration from teststand data.
* **ScalarTree** converts a scaler data file into a TTree for further analysis.
* **ShowEvent** (D) takes a single event number from a data file and makes a plot showing the hits in the event.
* **ShowGainCalPoints** takes the raw gain calibration data and plots the resulting ADC vs. VCal curve.
* **SkimEvents** (D) takes a binary data file and creates an output text file from the data (but, despite the name, doesn't actually skim anything).
* **splitintoev** (D) takes a binary data file and splits the different data (pixel data, TDC data, etc.) into different files. This seems to reflect an older design for how the data stream would be constructed, however.
* **TestAlignment** (A) tests the alignment reader/writer by reading in an alignment file and then writing it out again.
* **TestGainCal** (G) tests a gain cal file by reading it in, applying the gain calibration to a range of input ADC values, and plotting the resulting curve.
* **TrackTest** (DGA) tests the tracking algorithms by comparing the slopes and residuals using only a pair of hits to fit the track to those obtained by using hits on all three planes.
* **UltraBlackFinder** takes a pixel FED transparent buffer and finds the ultrablacks in it. Much the same as similar routines in the online software (and I'm not sure why one would need it in the offline software).
* **Verbose** (DGA) simply runs the clustering and tracking in PLTEvent and then prints out the resulting hit, cluster, and track information.

## Even Older Utilities

The Archive/ directory contains retired code which is no longer used. A lot of the code here also looks like it was never finished or used. There's no documentation, so you're on your own here.
