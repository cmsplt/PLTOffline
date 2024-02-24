# FermilabTalk2015

This directory contains the plots and scripts from my [Fermilab Talk of the Week](https://indico.cern.ch/event/401748/) in July 2015 -- I later cleaned up the scripts a bit to make output for the [approved plots page](https://twiki.cern.ch/twiki/bin/view/CMSPublic/PLTFirstResultsJuly2015) as well in August. This also includes the plots and scripts for the two fine timing scans in 2015 (although those weren't in the talk).

* `MakeNiceOccupancy.C` makes the occupancy plots using the data for Fill 3851 stored in `histo_occupancy.root`. The output plot is stored in `occupancies3851.*` in EPS, PDF, and PNG format.

* `MakeNiceTrackOccupancy.C` makes the track occupancy plots using the data for Fill 3850 stored in `histo_track_occupancy_new.root`. The output plot is stored in `trackoccupancies3850.*` in EPS, PDF, and PNG format. There's also an older ROOT file, `histo_track_occupancy.root`; I'm not sure what fill this one is from but presumably it had results that did not look quite as nice as the newer one. `run_number.txt` contains the details of how the track occupancy histogram was created.

* `PlotFineTimingScan.C` makes the plot showing the results of the fine timing scan in 2015. There are two input data files, `FineTimingScan.txt` for the first scan and `FineTimingScanAug23.txt` for the second. The script is currently set up to plot the latter, but can also be used to plot the former by changing the input file and the two constants at the top of the script.

* `PlotWBM.C` makes the plot of the beam optimization scan using the data from WBM stored in `ConditionBrowser_1435569109207.root`. The output plot is stored in `BeamOptimization.*` in EPS, PDF, and PNG format.

* `ScanXPlane`, `ScanYPlane` are the plots of the vdM scan analysis in PS, EPS, PDF, PNG, and ROOT format.

* The actual code for creating the vdM plots is in `vdm_for_talk`. This is a version of the old 2015 vdM framework with some changes to `vdmUtilities.py` for the necessary style updates and `env.sh` to run on the BRIL online machines, and adding `vdm_runII_plt.py` as the overall driver file (and `vdm_runII_test.py` as a test file). Note that the plot output names are `1_X_1.*` and `1_Y_1.*` but I renamed them as above. (I was planning to propagate the style changes back into the main framework, but this never happened before it became obsolete anyway.)

Also included are `tdrstyle.C` for providing the standard CMS plot style and `CMS_lumi.C` for providing the standard CMS lumi designation on plots.
