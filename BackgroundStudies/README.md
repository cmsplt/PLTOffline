# PLT background studies

This directory contains the scripts and plots for the studies to do a background measurement using PLT. These were originally done by Thoth in 2016, but since the original scripts for these had disappeared when we were writing the paper, I (Paul) recreated them in 2021 and added some improvements.

Despite being in the PLTOffline repository, these don't actually use the Slink data like most scripts here. Rather, they are designed to run on the HD5 files, with some information from Timber for the vacuum gauges.

These scripts are currently designed to run on fill 5005, a special fill in 2016 in which gas was deliberately introduced into the beam pipe in order to artificially create a higher background level. If you want to run these on more recent fills, some changes may be necessary; these are documented in the scripts.

The scripts consist of two pieces: `computePLTBackground.py` runs over the HD5 files, extracts the necessary beam and BCM1F information, and does the background computation on the raw PLT data. The result is two CSV files, one for the BCM1F background and one for the (un-normalized) PLT background. The second part, `PlotPLTBackground.C`, takes these two CSV files along with a third that you will need to provide from Timber, and plots them all.

## Basic concepts

Some more detail is provided in `computePLTBackground.py`, but the basic idea is to use the rate of triple coincidences in PLT to measure the beam-induced background so that it may be compared with BCM1F. In colliding bunches, the background is very small compared to the collision, so the trick is to find places to measure where we expect background to be present but without collisions. There are thus two diferent methods that we can use:
* "Method A", or "noncolliding", looks for BXs where only one beam is filled, and uses the rate measured in that BX as an estimate of the background in that beam
* "Method B", or "precolliding", looks for unfilled BXs immediately preceding filled bunches, and uses the rate measured in that BX on the incoming side as an estimate of the background in that beam

## Computing the background

The script `computePLTBackground.py` is responsible for actually computing the background. It runs over a list of HD5 files specified in `input_files` and produces two output files. One is simply a dump of the BCM1F background in the HD5 file. The other is the PLT background, as computed via the two methods described above. Note that the PLT background is normalized by the beam currents, to correspond to the BCM1F background definition, but otherwise no normalization is applied.

Note that the HD5 files for this fill have a weird error where sometimes earlier data is duplicated later in the file. The script will automatically detect and ignore these. I'm not sure what caused this for these files, but hopefully it's not a problem for future running.

Because this script needs access to the HD5 files, you'll presumably need to run it on one of the online BRIL machines.

## Plotting the results

In order to plot the results using `PlotPLTBackground.C`, you will need three files. The first two are `background_bcm1f.csv` and `background_pltz.csv` produced by `computePLTBackground.py`. The plotter also needs the readings from the vacuum gauges to plot the pressure. You will have to get these from Timber at <https://timber.cern.ch/> (note that Timber can only be accessed from within the CERN network, so you'll need a proxy otherwise). Follow the instructions in `PlotPLTBackground.C` to see what variables you need, and then export them as a CSV for the time period you're interested in as `background_vacuum.csv`.

Once all of the input files are set up, simply run `PlotPLTBackground.C` (any machine with a sufficiently recent version of ROOT should work) to produce the two plots, one for each background method.

Note that the script normalizes the PLT background so that the integral over the whole time period is equal to the integral to BCM1F. This seems to produce good results for fill 5005 but for other fills you may want to adopt a different method. No attempt has been made to derive a normalization factor using only PLT data.

## Results here

Also in this directory are the three CSV files and the plots for fill 5005, so you can see what the final products look like for this case.

--Paul Lujan, November 2021