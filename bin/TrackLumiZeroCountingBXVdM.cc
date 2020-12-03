////////////////////////////////////////////////////////////////////
//
//  TrackLumiZeroCountingBXVdM.cc -- a modification of
//    TrackLumiZeroCounting.cc to do bunch-by-bunch measurements
//    during the VdM scan. It drops the dropout and error check
//    functionality entirely (since if you have that in your scan
//    you're already in trouble). It also doesn't need to check which
//    bunches are filled (since that concept isn't necessary when
//    you're going bunch by bunch), but will just write out for all
//    bunches which are triggered (although if you want to select a
//    particular subset you can do that below).
//
//  Paul Lujan, June 2016
//  Edited by Daniel Gift, July 14 2016
//  updated for 2020 studies, PJL Oct. 2020
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <iomanip>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TFile.h"
#include "TProfile.h"

int TrackLumiZeroCounting(const std::string, const std::string, const std::string, const std::string);

std::vector<Int_t> FindActiveBunches(const std::string);

const Int_t NBX = 3564;
const Int_t nChannels = 24;                    
const Int_t consecutiveZeros = 3000000; // Number of events with no hits before declaring channel dead
const Int_t nValidChannels = 16; // Channels that could in theory have hits.
const Int_t validChannels[nValidChannels] = {1, 2, 4, 5, 7, 8, 10, 11, 13, 14, 
					     16, 17, 19, 20, 22, 23};
std::vector<int> filledChannels = {2, 4, 5, 7, 8, 10, 11, 13, 14, 16, 17, 19, 20};

// Number of filled bunches. Note: this doesn't actually get used for the per-BX analysis, but since the
// things that read these files expect a number here, we need to at least put something.
const int nFilledBunches = 32;
// If true, will only read the bunches from the vector below. If false, will use all bunches with data.
const bool useSpecificBX = false;
// If true, will warn you if it finds a BX not in the list below. Obviously this shouldn't be true if you're
// selecting only a subset of the triggered bunches.
const bool warnUnexpectedBX = false;
// If useSpecificBX is true, these are the bunches to use. Note: counting starts at 1 here
const std::vector<int> selectedBX =  {1, 41, 81, 110, 121, 161, 201, 241, 281, 591,   // 2016 triggered bunches
				      872, 912, 952, 992, 1032, 1072, 1112, 1151, 1152, 
				      1682, 1783, 1823, 1863, 1903, 1943, 1983, 2023, 
				      2063, 2654, 2655, 2694, 2734, 2774, 2814, 2854, 
				      2894, 2934};

int TrackLumiZeroCounting(const std::string DataFileName, const std::string GainCalFileName, const std::string AlignmentFileName,
		       const std::string TrackDistributionFileName, const std::string TimestampFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;
  std::cout << "TrackDistributionFileName: " << TrackDistributionFileName << std::endl;
  if (TimestampFileName != "")
    std::cout << "TimestampFileName: " << TimestampFileName << std::endl;

  std::cout << "Total of " << nFilledBunches << " bunches filled" << std::endl;
  
  // Set some basic style for plots
  PLTU::SetStyle();
  gStyle->SetOptStat(1111);

  // read the track quality vars
  std::map<int, float> MeanSlopeY;
  std::map<int, float> SigmaSlopeY;
  std::map<int, float> MeanSlopeX;
  std::map<int, float> SigmaSlopeX;
  std::map<int, float> MeanResidualY;
  std::map<int, float> SigmaResidualY;
  std::map<int, float> MeanResidualX;
  std::map<int, float> SigmaResidualX;

  FILE *qfile = fopen(TrackDistributionFileName.c_str(), "r");
  if (qfile == NULL) {
    std::cout << "Track quality file not found; cannot proceed without this data" << std::endl;
    return(1);
  } else {
    int nch, ch, roc;
    float mean, sigma;
    fscanf(qfile, "%d\n", &nch);
    for (int i=0; i<nch; ++i) {
      fscanf(qfile, "SlopeY_Ch%d %f %f\n", &ch, &mean, &sigma);
      MeanSlopeY[ch] = mean;
      SigmaSlopeY[ch] = sigma;
    }
    for (int i=0; i<nch; ++i) {
      fscanf(qfile, "SlopeX_Ch%d %f %f\n", &ch, &mean, &sigma);
      MeanSlopeX[ch] = mean;
      SigmaSlopeX[ch] = sigma;
    }
    for (int i=0; i<nch*3; ++i) {
      fscanf(qfile, "ResidualY%d_ROC%d %f %f\n", &ch, &roc, &mean, &sigma);
      MeanResidualY[10*ch+roc] = mean;
      SigmaResidualY[10*ch+roc] = sigma;
    }
    for (int i=0; i<nch*3; ++i) {
      fscanf(qfile, "ResidualX%d_ROC%d %f %f\n", &ch, &roc, &mean, &sigma);
      MeanResidualX[10*ch+roc] = mean;
      SigmaResidualX[10*ch+roc] = sigma;
    }
    
    fclose(qfile);
  } // read track quality file

  // Read timestamp file, if it exists.
  bool useTimestamps = false;
  int nSteps = 1;
  std::vector<std::pair<uint32_t, uint32_t> > timestamps;
  if (TimestampFileName != "") {
    FILE *tfile;
    tfile = fopen(TimestampFileName.c_str(), "r");
    if (tfile == NULL) {
      std::cerr << "Couldn't open timestamp file " << TimestampFileName << "!" << std::endl;
      return(1);
    }
    int tBegin, tEnd;
    fscanf(tfile, "%d", &nSteps);
    for (int i=0; i<nSteps; ++i) {
      fscanf(tfile, "%d %d", &tBegin, &tEnd);
      timestamps.push_back(std::make_pair(tBegin, tEnd));
    }
    fclose(tfile);
    useTimestamps = true;
    // std::cout << nSteps << " timestamps" << std::endl;
    // for (unsigned int i=0; i<timestamps.size(); ++i) {
    //   std::cout << "start: " << timestamps[i].first << " end: " << timestamps[i].second << std::endl;
    // }
  }

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, FidRegionHits);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_01to2_AllCombs);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  std::vector<std::vector<int> > nEventsBX(nSteps, std::vector<int>(NBX));

  typedef std::vector<std::vector<int> > vv;

  // Arrays to keep track of anaysis by channel and per bunch
  std::vector<vv> nAllTripleChBX(nSteps, vv(nChannels, std::vector<int>(NBX)));
  std::vector<vv> nGoodTripleChBX(nSteps, vv(nChannels, std::vector<int>(NBX)));
  std::vector<vv> nEmptyEventsChBX(nSteps, vv(nChannels, std::vector<int>(NBX)));
  std::vector<vv> nNonEmptyEventsChBX(nSteps, vv(nChannels, std::vector<int>(NBX)));
  
  int stepNumber = 0;
  uint32_t currentStepStart = 0;
  // const uint32_t stepLength = 60000; // 1 minute
  const uint32_t stepLength = 300000; // 5 minutes

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {

    //   if (ientry > 163000000) break;

    if (ientry % 100000 == 0) {
      int nsec = Event.Time()/1000;
      int hr = nsec/3600;
      int min = (nsec-(hr*3600))/60;
      int sec = nsec % 60;
      std::cout << "Processing event: " << ientry << " at " << Event.Time() << " ("
		<< std::setfill('0') << std::setw(2) << (hr%24) << ":" << std::setw(2) << min
		<< ":" << std::setw(2) << sec << "." << std::setw(3) << Event.Time()%1000
		<< ")" << std::endl;
      
      FILE *f = fopen("abort.txt", "r");
      if (f != NULL) {
	fclose(f);
	break;
      }
    }
    int BX = Event.BX();
    // Note -- Event.BX() is start-at-0 while selectedBX is start-at-1, so don't forget to adjust for that!
    if (useSpecificBX && std::find(selectedBX.begin(), selectedBX.end(), BX+1) == selectedBX.end()) {
      if (warnUnexpectedBX)
	std::cout << "Warning: found data with unexpected BX " << BX+1 << std::endl;
      continue;
    }
    
    if (useTimestamps) {
      stepNumber = -1;
      if (Event.Time() > timestamps[nSteps-1].second) {
	std::cout << "Reached end of last step, ending..." << std::endl;
	break;
      }
      for (int iStep = 0; iStep < nSteps; ++iStep) {
	if (Event.Time() >= timestamps[iStep].first && Event.Time() <= timestamps[iStep].second) {
	  stepNumber = iStep;
	  break;
	}
      }
      if (stepNumber == -1) continue;
    } else {
      if (stepNumber == 0 && currentStepStart == 0) currentStepStart = Event.Time();
      if ((Event.Time() - currentStepStart) > stepLength) {
	//This should never be needed with this file. Always run with time stamps
	std::cout << "warning: not supported" << std::endl;
      }
    }

    nEventsBX[stepNumber][BX]++; // Count number of events
    
    std::vector<int> hasGoodTracksInEvent(nChannels);

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      
      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);
      int ch = Telescope->Channel();
           
      if (Telescope->NTracks() > 0) {
	nAllTripleChBX[stepNumber][ch][BX]++; // Count number of triple coincidences
	
	bool foundOneGoodTrack = false;
	for (size_t itrack = 0; itrack < Telescope->NTracks(); ++itrack) {
	  PLTTrack *tr = Telescope->Track(itrack);
	  
	  // apply track quality cuts
	  float slopeY = tr->fTVY/tr->fTVZ;
	  float slopeX = tr->fTVX/tr->fTVZ;
	 
	  if (fabs((slopeY-MeanSlopeY[ch])/SigmaSlopeY[ch]) > 5.0) continue;
	  if (fabs((slopeX-MeanSlopeX[ch])/SigmaSlopeX[ch]) > 5.0) continue;
	  if (fabs((tr->LResidualY(0)-MeanResidualY[10*ch])/SigmaResidualY[10*ch]) > 5.0) continue;
	  if (fabs((tr->LResidualY(1)-MeanResidualY[10*ch+1])/SigmaResidualY[10*ch+1]) > 5.0) continue;
	  if (fabs((tr->LResidualY(2)-MeanResidualY[10*ch+2])/SigmaResidualY[10*ch+2]) > 5.0) continue;
	  if (fabs((tr->LResidualX(0)-MeanResidualX[10*ch])/SigmaResidualX[10*ch]) > 5.0) continue;
	  if (fabs((tr->LResidualX(1)-MeanResidualX[10*ch+1])/SigmaResidualX[10*ch+1]) > 5.0) continue;
	  if (fabs((tr->LResidualX(2)-MeanResidualX[10*ch+2])/SigmaResidualX[10*ch+2]) > 5.0) continue;
	  foundOneGoodTrack = true;
	  break; // We found a valid track!
	}
	if (foundOneGoodTrack) { 
	  nGoodTripleChBX[stepNumber][ch][BX]++; //Count the number of valid triple coincidences
	  hasGoodTracksInEvent[ch]++; // Note that this channel had a good track
	}
      }
    } // telescope loop
    
    for (Int_t index = 0; index < nValidChannels; ++index) {
      Int_t channel = validChannels[index];

      if (hasGoodTracksInEvent[channel] > 0) {
	nNonEmptyEventsChBX[stepNumber][channel][BX]++; // There were tracks!
      }      
      else {
	nEmptyEventsChBX[stepNumber][channel][BX]++; // No tracks 
      }
    }

  } // event loop
  
  // properly catch the last step
  timestamps.push_back(std::make_pair(currentStepStart, Event.Time()));

  int nFilledChannels = filledChannels.size();
  for (int iBX = 0; iBX < NBX; ++iBX) {
    // If we are using specific BXs, skip ones that aren't in the list. If we're not, skip ones that don't have any triggers.
    if (useSpecificBX && std::find(selectedBX.begin(), selectedBX.end(), iBX+1) == selectedBX.end()) continue;
    if (!useSpecificBX && nEventsBX[0][iBX] == 0) continue;

    std::stringstream buf;
    buf << "TrackLumiZC" << std::setfill('0') << std::setw(4) << iBX+1 << std::setw(0) << ".txt";
    std::string name = buf.str();
    FILE *outf = fopen(name.c_str(), "w");
    fprintf(outf, "%d %d\n", nSteps, nFilledBunches);

    for (int iStep=0; iStep<nSteps; ++iStep) {
      // compute channel-averaged quantities
      float allTripleTot = 0.;
      float goodTripleTot = 0.;
      float emptyEventsTot = 0.;
      float nonEmptyEventsTot = 0.;
      for (std::vector<int>::iterator it = filledChannels.begin(); it != filledChannels.end(); ++it) {
	int iChan = *it;
	allTripleTot += nAllTripleChBX[iStep][iChan][iBX];
	goodTripleTot += nGoodTripleChBX[iStep][iChan][iBX];
	emptyEventsTot += nEmptyEventsChBX[iStep][iChan][iBX];
	nonEmptyEventsTot += nNonEmptyEventsChBX[iStep][iChan][iBX];
      }

      fprintf(outf, "%d %d %d %lf %lf %d %lf %lf", timestamps[iStep].first, timestamps[iStep].second, nEventsBX[iStep][iBX],
	      allTripleTot/nFilledChannels, goodTripleTot/nFilledChannels, nEventsBX[iStep][iBX],
	      emptyEventsTot/nFilledChannels, nonEmptyEventsTot/nFilledChannels);
      for (auto ch: filledChannels) {
	fprintf(outf, " %d", nNonEmptyEventsChBX[iStep][ch][iBX]);
      }
      fprintf(outf, "\n");

    }
    fclose(outf);
  }
  
return 0;
}

int main (int argc, char* argv[])
{
  if (argc < 5 || argc > 6) {
    std::cerr << "Usage: " << argv[0] << " DataFile.dat GainCal.dat AlignmentFile.dat TrackDistributions.txt [TimestampFile.dat]" << std::endl;
    return 1;
  }

  const std::string DataFileName = argv[1];
  const std::string GainCalFileName = argv[2];
  const std::string AlignmentFileName = argv[3];
  const std::string TrackDistributionFileName = argv[4];
  const std::string TimestampFileName = (argc == 6) ? argv[5] : "";

  TrackLumiZeroCounting(DataFileName, GainCalFileName, AlignmentFileName, TrackDistributionFileName, TimestampFileName);

  return 0;
}
