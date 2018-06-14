#!/bin/bash

source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.36/x86_64-centos7-gcc48-opt/root/bin/thisroot.sh # set up ROOT 5.34
# /afs becomes inaccessible once the kerberos ticket expires!
# if [[ $( echo ${PATH} | grep "brilconda" > /dev/null 2>&1; echo $? ) = 1 ]]; then export PATH=$HOME/.local/bin:/afs/cern.ch/cms/lumi/brilconda-1.1.7/bin:$PATH; fi # https://cms-service-lumi.web.cern.ch/cms-service-lumi/#prerequisite

localPLTOffline=/localdata/PLT-EndOfFill/PLTOffline
localPLTOnline=/localdata/PLT-EndOfFill/PLTOnline

# https://linuxtidbits.wordpress.com/2008/08/11/output-color-on-bash-scripts/
red=$(tput setaf 1);
normal=$(tput sgr0);

# set group permissions to the same as owner permissions
# chmod -R g=u                     /localdata/201*/ /localdata/PLT-*/ /localdata/miniconda3/ # https://superuser.com/a/83621
# set rw permissions to owner and group for any new files (needs to be run at least once for each user)
setfacl -Rdm u::rwX,g::rwX,o::rX /localdata/201*/ /localdata/PLT-*/ /localdata/miniconda3/ # https://unix.stackexchange.com/a/75977 # recursive: https://serverfault.com/q/2736 # capital X makes directories executable

analyze(){
  # ONLY MODIFY THIS FUNCTION IF THIS IS A LOCAL COPY (DO NOT CHANGE /localdata/PLT-EndOfFill/PLT-EndOfFill.sh SINCE IT RUNS AUTOMATICALLY)
  # Select which processes to run.
  printf "%s\n\n" "${red}PROCESSING FILL ${fill}...${normal}"
  \mkdir -p ${outputDir}/logs
  # https://github.com/cmsplt/PLTOffline/blob/master/README.md
  # processGainCal                   &   
  processBXpattern                 &
  processDumpErrors                &
  # processAlignment                 &
  processPulseHeights              &
  # processClusterSizeDistribution   &
  processTrackOccupancy            &
  # processTrackDistributions        &
  # processAccidentals               &
  # processROC_Efficiency            &
  touch ${outputDir}/DoNotOverwrite
}

firstRunSetup(){
  # This should only need to be done once
  # PLTOFFLINE
  mkdir -p ${localPLTOffline}
  cd ${localPLTOffline%/*}
  git clone https://github.com/cmsplt/PLTOffline.git
  cd ${localPLTOffline}
  make
  git pull
  # PLTONLINE
  mkdir -p ${localPLTOnline}
  cd ${localPLTOnline}
  read -p "Please enter your svn username: " svnuser
  export SVN_SSH="ssh -l $svnuser"
  svn co https://svn.cern.ch/reps/cmsplt/trunk cmsplt
  cd ${localPLTOnline}/cmsplt/
  svn status
  svn update
  # BRILCALC # https://cms-service-lumi.web.cern.ch/cms-service-lumi/#brilcondainstallation
  mkdir -p ${localBRILCALC}
  cd       ${localBRILCALC}
  wget https://cern.ch/cmslumisw/installers/linux-64/Brilconda-1.1.7-Linux-x86_64.sh
  bash Brilconda-1.1.7-Linux-x86_64.sh -b -p ${localBRILCALC}/Brilconda-1.1.7
  Brilconda-1.1.7/bin/./pip show brilws | grep "Location:"
  # Location: /afs/cern.ch/user/a/adelanno/.local/lib/python2.7/site-packages
  Brilconda-1.1.7/bin/./pip uninstall brilws
  # Uninstalling brilws-3.1.3:
  # ...
  Brilconda-1.1.7/bin/./pip install   brilws
  # Collecting brilws
  # ...
  Brilconda-1.1.7/bin/./brilcalc --version
  # 3.3.0
}

selectFill(){
  if [[ $1 =~ [0-9]{4,5} ]];
    # If 4-digit argument is provided then use input as fill number
    then
      fill=$1
    # If no argument is provided, prompt user for input fill during 60 seconds
    else
      printf "%s\n" "Usage: . /localdata/PLT-EndOfFill/PLT-EndOfFill.sh <fillNumber>";
      printf "%s\n" "Please provide a fill number or wait 10 seconds to analyze the most recent fill"
      # for (( time=60; time>0; time=$((time-1)) )); do printf "${red}${time}${normal} \r"; read -t1 -s fill; if [[ ! -z $fill ]]; then break; fi; done;
      read -t10 -p "select fill: " userFill
      if [[ ! -z "${userFill}" ]]; 
        # If user provides fill number then use input as fill number
        then
          fill=${userFill}
        # Otherwise, select most recent fill in /localdata/PLT-timestamps/TimeStamps.StableBeams
        else
          if [[ $( awk -F '[ .]' 'NR==1{print NF}' /localdata/PLT-timestamps/TimeStamps.StableBeams ) -ge 7 ]]; then
            fill=$( awk -F '[ .]' 'NR==1{print $1}' /localdata/PLT-timestamps/TimeStamps.StableBeams )       
          fi
          printf "\n%s\n" "Checking most recent fill: ${fill}"
      fi
  fi
}

updateTimestamps(){
  # Update /localdata/PLT-timestamps/PLT-timestamps.txt which lists SLINK and WORKLOOP timestamps corresponding to every fill (fill|fillDeclared|fillEnd|slinkTS|workloopTS)
  printf '\n%s\n' "UPDATING TIMESTAMPS..."
  /localdata/PLT-timestamps/./PLT-timestamps.sh  > /dev/null
  # Find the SLINK timestamp(s) corresponding to the given fill
  slinkTS=( $( grep "^${fill}" /localdata/PLT-timestamps/PLT-timestamps.txt | awk -F '|' '{print $4}' ) )
  # Determine the year based on the SLINK timestamp
  year=$( echo ${slinkTS} | cut -c 1-4 )
  # Set the directory contaning the SLINK files based on ${year}
  slinkDir=/localdata/${year}/SLINK
  # Clear ${slinkFiles} array
  slinkFiles=()
  # Loop through SLINK timestamp(s) and assign full paths to the SLINK files
  for ts in ${slinkTS[@]};
    do
    slinkFiles+=( ${slinkDir}/Slink_${ts}.dat );
  done
}

checkLoadedInBRILCALC(){
  # Check if selected fill is loaded in BRILCALC by parsing its output
  # mostRecentFill=$( awk -F '[ .]' 'NR==1{print $1}' /localdata/PLT-timestamps/TimeStamps.StableBeams )
  /localdata/brilconda/Brilconda-1.1.7/bin/./brilcalc lumi -f ${fill} -o /localdata/PLT-EndOfFill/brilcalc/${fill}.summary
  numberOfLines=$( awk 'END{print NR}' /localdata/PLT-EndOfFill/brilcalc/${fill}.summary )
  # fillLoadedInBRILCALC=$( awk -F '[| ]' 'NR=='"$(( ${numberOfLines} -2 ))"'{print $3}' "/localdata/PLT-EndOfFill/brilcalc/${fill}.summary" )
  fillLoadedInBRILCALC=$( awk -F '[#,]' 'NR=='"$(( ${numberOfLines}    ))"'{print $2}' "/localdata/PLT-EndOfFill/brilcalc/${fill}.summary" )
}

inputFiles(){
  # Select input files based on the ${year}
  # https://github.com/cmsplt/PLTOffline/blob/master/AccidentalStudies/README
  if   [[ ${year} == 2015 ]]; then
    gaincalFile=${localPLTOffline}/GainCal/GainCalFits_20150923.225334.dat
    alignmentFile=${localPLTOffline}/ALIGNMENT/Trans_Alignment_4449.dat
    trackDistributionsFile=${localPLTOffline}/TrackDistributions/TrackDistributions_MagnetOn.txt
    # magnetOFFalignmentFile=${localPLTOffline}/ALIGNMENT/Trans_Alignment_4341.dat
    # magnetOFFtrackDistributionsFile=${localPLTOffline}/TrackDistributions/TrackDistributions_MagnetOff_4341.txt
  elif [[ ${year} == 2016 ]]; then
    gaincalFile=${localPLTOffline}/GainCal/GainCalFits_20160501.155303.dat
    alignmentFile=${localPLTOffline}/ALIGNMENT/Trans_Alignment_4892.dat
    trackDistributionsFile=${localPLTOffline}/TrackDistributions/TrackDistributions_MagnetOn2016_4892.txt
  elif [[ ${year} == 2017 ]]; then
    gaincalFile=/localdata/2017/CALIB/GainCal/GainCalFits_20171126.141225.dat
    alignmentFile=${localPLTOffline}/ALIGNMENT/Trans_Alignment_6110.dat
    trackDistributionsFile=${localPLTOffline}/TrackDistributions/TrackDistributions_MagnetOn2017_5718.txt
  fi
  if [[ ${year} == 2018 ]]; then
    gaincalFile=/localdata/2018/CALIB/GainCal/20180305.200728/GainCalFits_20180305.200728.dat
    alignmentFile=${localPLTOffline}/ALIGNMENT/Trans_Alignment_2017MagnetOn_Prelim.dat
    trackDistributionsFile=${localPLTOffline}/TrackDistributions/TrackDistributions_MagnetOn2017_5718.txt
  fi
  # Parse the most recent timestamps for WORKLOOP and GAINCAL files
  workloopDir=/localdata/${year}/WORKLOOP
  mostRecentWorkloopTS=$( ls -t ${workloopDir}/Data_Scaler_* | head -1 | grep -o '[0-9]\+[.][0-9]\+' );
  gaincalDir=/localdata/${year}/CALIB/GainCal
  mostRecentGaincalTS=$(  ls -t ${gaincalDir}/gaincalfast_*  | head -1 | grep -o '[0-9]\+[.][0-9]\+' );
}

runBRILCALC(){
  printf '%s\n' "RUNNING BRILCALC..."
  # Run BRILCALC twice to get AVGPU for every lumisection and the number of colliding bunches (required for Accidentals)
  # https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalc
  # Run BRILCALC LUMI to get AVGPU for every lumisection
  brilcalcLumiFile=/localdata/PLT-EndOfFill/brilcalc/${fill}.lumi
  # brilcalc lumi -f ${fill} --byls --xingTr 0.85 --type PLTZERO --tssec -o ${brilcalcLumiFile} # https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalclumi
  /localdata/brilconda/Brilconda-1.1.7/bin/./brilcalc lumi -f ${fill} --byls --type PLTZERO --tssec               -o ${brilcalcLumiFile} # https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalclumi
  dos2unix -q ${brilcalcLumiFile}
  # Run BRILCALC BEAM to get number of colliding bunches
  brilcalcBeamFile=/localdata/PLT-EndOfFill/brilcalc/${fill}.beam
  /localdata/brilconda/Brilconda-1.1.7/bin/./brilcalc beam -f ${fill}                                             -o ${brilcalcBeamFile} # https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#brilcalcbeam
  dos2unix -q ${brilcalcBeamFile}
  # https://stackoverflow.com/a/13380679 # https://unix.stackexchange.com/a/37791
  IFS=$'\n' # allow whitespace in array elements
  # run=(        $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $1}'  ) )
  # fill=(       $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $2}'  ) )
  # LSstart=(    $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $3}'  ) )
  # LSend=(      $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $4}'  ) )
  LStstamp=(     $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $5}'  ) )
  # beamStatus=( $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $6}'  ) )
  # energy=(     $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $7}'  ) )
  delivered=(    $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $8}'  ) )
  recorded=(     $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $9}'  ) )
  avgPU=(        $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $10}' ) )
  # lumiSource=( $( head -n -3 ${brilcalcLumiFile} | sed '1,3d' | awk -F '[,:]' '{if ($6=="STABLE BEAMS") print $11}' ) )

  # Parse the number of colliding bunches in the fill
  nBX=$( sed -n '3p' ${brilcalcBeamFile} | awk -F '[,]' '{print $NF}' )
  # stableBeamsDeclared=$(date -d @${LStstamp[0]} +"%Y%m%d.%H%M%S")

  # Recreate csv file exported from CMSWBM Condition Browser 
  # https://github.com/cmsplt/PLTOffline/blob/master/AccidentalStudies/README#L45
  # TO DO: Convert AVG PILE UP -> SBIL
  
  SBILfile=${localPLTOffline}/AccidentalStudies/${fill}.brilcalcSBIL.csv
  printf "%s\n%s\n%s\n"  "cms_omds_lb.CMS_BEAM_COND.CMS_BRIL_LUMIPLTZ.LUMI_TOTINST,,,"  "$( echo ${#LStstamp[@]} ),,,"  "ROW,WEIGHT,DIPTIME,LUMI_TOTINST" > ${SBILfile}
  for (( i=1; i<"${#LStstamp[@]}"; i++)); do
    SBIL=$( bc -l <<< "( "${delivered[i]}/${nBX}" )" )
    printf "%s\n" "${i},1,${LStstamp[i]},${SBIL}" >> ${SBILfile}
  done

}

processGainCal(){
  # https://github.com/cmsplt/PLTOffline/blob/master/bin/GainCalFastFits.cc
  printf '%s\n' "RUNNING GAINCAL FAST FITS..."
  ./GainCalFastFits                            ${gaincalDir}/gaincalfast_${mostRecentGaincalTS}.avg.txt   > ${outputDir}/logs/GainCalFastFits.txt 2>&1
  mkdir  -p                                    ${gaincalDir}/${mostRecentGaincalTS}/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/GainCalFits_*   ${gaincalDir}/${mostRecentGaincalTS}/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/plots/GainCal/* ${gaincalDir}/${mostRecentGaincalTS}/ > /dev/null 2>&1
}

processBXpattern(){
  # https://github.com/cmsplt/PLTOffline/blob/master/bin/TestBXPattern.cc
  # TestBXPattern (D) will read a binary data file and print out the rate of triggers for each BX and the average number of hits in each BX. Can be used to make sure that the trigger is functioning as desired (especially useful for special VdM triggers). Also present in the online archive.
  printf '%s\n' "RUNNING TEST BXPATTERN..."
  ./TestBXPattern ${slinkFile}                                                                            > ${outputDir}/logs/TestBXPattern.txt 2>&1
}

processDumpErrors(){
  # https://github.com/cmsplt/PLTOffline/blob/master/bin/DumpErrors.cc
  printf '%s\n' "RUNNING DUMP ERRORS..."
  ./DumpErrors    ${slinkFile}                                                                            > ${outputDir}/logs/DumpErrors.txt 2>&1
}

processPulseHeights(){
  # https://github.com/cmsplt/PLTOffline/blob/master/bin/PulseHeights.cc
  # PulseHeights (DG) makes a plot of the pulse heights using the gain calibration to translate the raw ADC counts into a charge. It produces two sets of plots: plots/ClusterSize_ChXX.gif simply shows the distribution of number of pixels per cluster in each ROC, and plots/PulseHeight_ChXX.gif shows the distribution of charge for single-pixel, two-pixel, and three-or-more-pixel clusters, the average pulse height as a function of time, and a 2-D histogram showing the average charge per pixel. PulseHeightsBorder is a variant which looks at the pulse heights only for pixels on the border of the active area or not on the border, given an input mask.
  printf '%s\n' "RUNNING PULSE HEIGHTS..."
  ./PulseHeights  ${slinkFile}  ${gaincalFile}                                                            > ${outputDir}/logs/PulseHeights.txt 2>&1
  mkdir  -p                                                                                                 ${outputDir}/PulseHeights/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/plots/PulseHeight*.gif                                                       ${outputDir}/PulseHeights/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/plots/ClusterSize*.gif                                                       ${outputDir}/PulseHeights/ > /dev/null 2>&1
}

processClusterSizeDistribution(){
  printf '%s\n' "RUNNING CLUSTER SIZE DISTRIBUTION..."
  mkdir -p                                                                                                  ${localPLTOffline}/plots/${fill}/ > /dev/null 2>&1
  ./MakeClusterSizeDistribution ${slinkFile} ${gaincalFile} ${fill}                                       > ${outputDir}/logs/MakeClusterSizeDistribution.txt 2>&1
  mkdir -p                                                                                                  ${outputDir}/MakeClusterSizeDistribution/ > /dev/null 2>&1
  cp    -f  ${localPLTOffline}/plots/${fill}/cluster_size_distr_output_*                                    ${outputDir}/MakeClusterSizeDistribution/ > /dev/null 2>&1
}

processAlignment(){
  # https://github.com/cmsplt/PLTOffline/blob/master/bin/CalculateAlignment.cc
  # CalculateAlignment (DGA) is used to generate the alignment used by the PLT tracking. It proceeds in three passes: first it corrects the rotational offset of the second and third planes relative to the first, and then it runs a second pass applying that rotation to correct the translational offset of the second and third planes relative to the first. Finally in the third pass it applies all the corrections to examine the remaining residuals. It produces ROTATED_Alignment.dat, which is a .dat file containing the results of the alignment after the first pass, and Trans_Alignment.dat, a .dat file containing the results of the alignment after the second pass. The latter is the file that can be used as the alignment input for further analyses (again, if you want to save it for further use, it's recommended that you save it as ALIGNMENT/Trans_Alignment_[FillNumber].dat). The plots are saved in plots/Alignment/ and the ROOT file containing the plot data is saved in histo_calculatealignment.root. The input alignment file is used as a first guess for starting the alignment process -- you can use the previous alignment file or ALIGNMENT/Alignment_IdealInstall.dat. The script plotScripts/PlotAlignmentVsTime.C can be used to compare different alignment files so you can observe changes over time.
  printf '%s\n' "RUNNING CALCULATE ALIGNMENT..."
  ./CalculateAlignment  ${slinkFile}  ${gaincalFile}  ${alignmentFile}                                    > ${outputDir}/logs/CalculateAlignment.txt 2>&1
  mkdir  -p                                                                                                 ${outputDir}/CalculateAlignment/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/*_Alignment.dat                                                              ${outputDir}/CalculateAlignment/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/histo_calculatealignment.root                                                ${outputDir}/CalculateAlignment/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/plots/Alignment/*                                                            ${outputDir}/CalculateAlignment/ > /dev/null 2>&1
}

processTrackOccupancy(){
  # https://github.com/cmsplt/PLTOffline/blob/master/bin/TrackOccupancy.cc
  # TrackOccupancy (DGA) makes occupancy plots similar to those in OccupancyPlots, but only considers hits which are part of reconstructed tracks using the tracking. This can be used to study the alignment and the mask alignment. IMPORTANT: The mask for the central plane is hard-coded: if the position of the hit on the central plane falls outside the mask the track will be discarded. Please be aware of this if the mask position changes! Produces plots in plots/TrackOccupancy*.gif (the actual occupancy plots), Tracks_ChXX_EvYY.gif (the raw hit data for the first few individual tracks in the file), and histo_track_occupancy.root, which contains the plot data.
  printf '%s\n' "RUNNING TRACK OCCUPANCY..."
  ./TrackOccupancy  ${slinkFile}  ${gaincalFile}  ${alignmentFile}                                        > ${outputDir}/logs/TrackOccupancy.txt 2>&1
  mkdir  -p                                                                                                 ${outputDir}/TrackOccupancy/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/histo_track_occupancy.root                                                   ${outputDir}/TrackOccupancy/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/plots/TrackOccupancy*.gif                                                    ${outputDir}/TrackOccupancy/ > /dev/null 2>&1
}

processTrackDistributions(){
  # https://github.com/cmsplt/PLTOffline/tree/master/TrackDistributions
  printf '%s\n' "RUNNING TRACK DISTRIBUTIONS..."
  mkdir  -p                                                                                                 ${localPLTOffline}/plots/${fill}/ > /dev/null 2>&1
  ./MeasureAccidentals ${slinkFile} ${gaincalFile} ${alignmentFile} ${trackDistributionsFile} dummy.txt   > ${outputDir}/logs/MeasureAccidentals.txt 2>&1
  mkdir  -p                                                                                                 ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1
  cp     -f ${localPLTOffline}/TrackDistributions_${fill}.txt                                               ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1

}

processAccidentals(){
  # https://github.com/cmsplt/PLTOffline/blob/master/AccidentalStudies/README
  # MeasureAccidentals (DGAT) is the main utility for measuring the accidental rate. It performs the tracking using all possible combinations of hits with no quality cuts, and then looks to see if it can find at least one "good" track among the combinations. If so, the telescope is marked as good for that event; otherwise, the hits are taken to be accidental. A good track is defined as one for which all of the track parameters (slope X/Y, and residual X/Y in planes 0/1/2) are within 5 sigma of the mean values. These parameters are defined in a track distributions file, which is an additional input to the script and can be found in the TrackDistributions/ directory. You can also run MeasureAccidentals without an input track distributions file, in which case it won't compute the accidental rates but will produce an output TrackDistributions_[FillNumber].txt file which can be used for further running. The main output of the script is AccidentalRates_[FillNumber].txt, which contains the accidental data by time period. It also creates a histo_slopes.root file and plots like MakeTracks. Note: While the accidental rate is computed using all tracks, the track distribution file is derived only from the "pure" track sample; i.e., events where there is exactly one cluster in each plane in the telescope.
  printf '%s\n' "RUNNING MEASURE ACCIDENTALS..."
  mkdir  -p                                                                                                 ${localPLTOffline}/plots/${fill}/ > /dev/null 2>&1
  ./MeasureAccidentals ${slinkFile} ${gaincalFile} ${alignmentFile} ${trackDistributionsFile} ${fill}     > ${outputDir}/logs/MeasureAccidentals.txt 2>&1
  mkdir  -p                                                                                                 ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/AccidentalRates_${fill}.txt                                                  ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/TrackDistributions_${fill}.txt                                               ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/histo_slopes_${fill}.root                                                    ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/plots/${fill}/Slope*.gif                                                     ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/plots/${fill}/Residual*.gif                                                  ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1
  mv     -f ${localPLTOffline}/plots/${fill}/BeamSpot.gif                                                   ${outputDir}/MeasureAccidentals/ > /dev/null 2>&1
  # https://github.com/cmsplt/PLTOffline/blob/master/AccidentalStudies/README#L38
  numLinesAccidentalRates=$(         cat ${outputDir}/MeasureAccidentals/AccidentalRates_${fill}.txt | sed '1,1d' | wc -l ) 
  # TSbeginAccidentalRates=( $(        cat ${outputDir}/MeasureAccidentals/AccidentalRates_${fill}.txt | sed '1,1d' | awk -F '[ ]' '{print $1}' ) )
  # TSendAccidentalRates=( $(          cat ${outputDir}/MeasureAccidentals/AccidentalRates_${fill}.txt | sed '1,1d' | awk -F '[ ]' '{print $2}' ) )
  # numTriggersAccidentalRates=( $(    cat ${outputDir}/MeasureAccidentals/AccidentalRates_${fill}.txt | sed '1,1d' | awk -F '[ ]' '{print $3}' ) )
  # numTotalTracksAccidentalRates=( $( cat ${outputDir}/MeasureAccidentals/AccidentalRates_${fill}.txt | sed '1,1d' | awk -F '[ ]' '{print $4}' ) )
  # numGoodTracksAccidentalRates=( $(  cat ${outputDir}/MeasureAccidentals/AccidentalRates_${fill}.txt | sed '1,1d' | awk -F '[ ]' '{print $5}' ) )
  
  # https://github.com/cmsplt/PLTOffline/blob/master/AccidentalStudies/README#L50
  ./ParseCondDBData  ${outputDir}/MeasureAccidentals/AccidentalRates_${fill}.txt  ${SBILfile}             > ${outputDir}/MeasureAccidentals/ParseCondDBData.txt 2>&1
  mv  -f ${localPLTOffline}/CombinedRates.txt                                                               ${outputDir}/MeasureAccidentals/CombinedRates_${fill}.txt > /dev/null 2>&1
  sed -i "1 s|.*|${numLinesAccidentalRates} ${nBX}|"                                                        ${outputDir}/MeasureAccidentals/CombinedRates_${fill}.txt > /dev/null 2>&1 # https://stackoverflow.com/a/9309986 
}

processROC_Efficiency(){
  # https://github.com/cmsplt/PLTOffline/blob/master/bin/ROC_Efficiency.cc
  printf '%s\n' "RUNNING ROC EFFICIENCY..."
  ./ROC_Efficiency ${slinkFile} ${gaincalFile} ${alignmentFile} ${trackDistributionsFile} ${fill}         > ${outputDir}/logs/ROC_Efficiency.txt 2>&1
}

EOFmain(){
  # Proceed only if the current fill is loaded in BRILCALC
  if [[ ${fillLoadedInBRILCALC} = 1 ]];
  then
    if [[ $1 =~ [0-9]{4,5} || ! -z ${userFill} ]];
      then # If script is run manually (fill is provided as a 4- or 5-digit argument or in prompt) then set output directory to user's home directory and allow overwritting of results # https://stackoverflow.com/a/806923
        outputDir="${HOME}/PLTANALYSIS/Fill${fill}-Slink${slinkTS}";
        rm ${outputDir}/DoNotOverwrite > /dev/null 2>&1;
      else # If fill is run automatically, set outputDir=/localdata/${year}/ANALYSIS/Fill${fill}-Slink${slinkTS} 
        outputDir=/localdata/${year}/ANALYSIS/Fill${fill}-Slink${slinkTS} 
    fi
    # Proceed only if ${slinkFiles} exist(s) (guarantees most recent SLINK has been transferred) AND if the ${outputDir}/DoNotOverwrite directory DOES NOT exist (guarantees this fill hasn't been already analyzed)
    if [[ -e ${slinkFiles} && ! -e ${outputDir}/DoNotOverwrite ]];
      then
        inputFiles
        runBRILCALC
        cd ${localPLTOffline}/
        for slinkFile in ${slinkFiles[@]}; do
          analyze
        done
        cd ~-
      else
        printf "%s\n%s\n" "No SLINK files found for fill ${fill} :( Please double-check that stable beams were declared for this fill." "OR fill was already analyzed automatically (remove ${outputDir}/DoNotOverwrite to OVERWRITE results)"
    fi
  else
    printf "%s\n" "Fill ${fill} has not yet been loaded into BRILCALC. Please double-check that stable beams were declared for this fill and try again later."
fi
}

selectFill $1
updateTimestamps
checkLoadedInBRILCALC
EOFmain

# TO DO:
# Get most recent fill from BRILCALC once it is available 
  # https://github.com/CMS-LUMI-POG/Normtags/blob/master/get_recentfill.py
# Convert avgPU -> SBIL
