// USED FOR THE VDM SCAN, BUNCH-BY-BUNCH ANALYSIS
// HAS EXCESSIVELY LONG NUMBERS TO ACCOUNT FOR THIS
// AND PRODUCES MANY OUTPUT FILES, ONE FOR EACH BUNCH
////////////////////////////////////////////////////////////////////
//
//  TrackLumiZeroCounting -- a program to derive a luminosity
//    using the rate of good tracks
//    Based on MeasureAccidentals to derive good tracks, 
//    PlotActiveBunches to determine filled bunches,
//    and DetectFailure.cc to determine when a channel drops out.
//
//  Paul Lujan, June 2016
//  Edited by Daniel Gift, July 14 2016
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
// Trigger Bunches
const int nFilledBunches = 32;
const int numTriggers = 37;
// This counting scheme starts at 1
const int triggerInit[numTriggers] = {1, 41, 81, 110, 121, 161, 201, 241, 281, 591, 
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

  // Make a vector to indicate which BXs should be considered. A -1 indicates
  // it is not a trigger, a 0 or higher indicates it is a trigger
  std::vector<int> triggers(NBX); //The counting scheme here starts at 0
  for (int bx = 0; bx < NBX; ++bx) {
    for (int trig = 0; trig < numTriggers; ++trig) {
      if (triggerInit[trig] == bx+1) triggers[bx] = 1;
    }
  }
  for (int bx = 0; bx < NBX; ++bx){
    triggers[bx]--;
  }

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

  bool useTrackQuality = true;
  FILE *qfile = fopen(TrackDistributionFileName.c_str(), "r");
  if (qfile == NULL) {
    std::cout << "Track quality file not found; accidental fraction will not be measured" << std::endl;
    useTrackQuality = false;
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
    
//     for (std::map<int, float>::iterator it = MeanSlopeY.begin(); it != MeanSlopeY.end(); ++it) {
//     ch = it->first;
//     std::cout << ch << " sX " << MeanSlopeX[ch] << "+/-" << SigmaSlopeX[ch]
// 	      << " sY " << MeanSlopeY[ch] << "+/-" << SigmaSlopeY[ch]
// 	      << " rX0 " << MeanResidualX[10*ch] << "+/-" << SigmaResidualX[10*ch]
// 	      << " rX1 " << MeanResidualX[10*ch+1] << "+/-" << SigmaResidualX[10*ch+1]
// 	      << " rX2 " << MeanResidualX[10*ch+2] << "+/-" << SigmaResidualX[10*ch+2]
// 	      << " rY0 " << MeanResidualY[10*ch] << "+/-" << SigmaResidualY[10*ch]
// 	      << " rY1 " << MeanResidualY[10*ch+1] << "+/-" << SigmaResidualY[10*ch+1]
// 	      << " rY2 " << MeanResidualY[10*ch+2] << "+/-" << SigmaResidualY[10*ch+2]
// 	      << std::endl;
//     }
//    return(0);

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

  std::map<int, int> NTrkEvMap;

  TH2F* HistBeamSpot[4];
  HistBeamSpot[0] = new TH2F("BeamSpotX", "BeamSpot X=0;Y;Z;NTracks", 25, -25, 25, 25, -540, 540);
  HistBeamSpot[1] = new TH2F("BeamSpotY", "BeamSpot Y=0;X;Z;NTracks", 25, -25, 25, 25, -540, 540);
  HistBeamSpot[2] = new TH2F("BeamSpotZ", "BeamSpot Z=0;X;Y;NTracks", 30, -10, 10, 15, -10, 10);
  HistBeamSpot[3] = new TH2F("BeamSpotZzoom", "BeamSpotZoom Z=0;X;Y;NTracks", 30, -5, 5, 30, -5, 5);

  std::map<int, TH1F*> MapSlopeY;
  std::map<int, TH1F*> MapSlopeX;
  std::map<int, TH2F*> MapSlope2D;
  std::map<int, TH1F*> MapResidualY;
  std::map<int, TH1F*> MapResidualX;

  // Arrays to keep track of variables
  std::vector<int> nTotTracks(nSteps);
  std::vector<int> nGoodTracks(nSteps);
  std::vector<std::vector<float> > nAllTriple(nSteps, std::vector<float>(NBX));
  std::vector<std::vector<float> > nGoodTriple(nSteps, std::vector<float>(NBX));
  std::vector<std::vector<float> > nEmptyEventsFilledBX(nSteps, std::vector<float>(NBX));
  std::vector<std::vector<float> > nNonEmptyFilledBX(nSteps, std::vector<float>(NBX));
  std::vector<std::vector<int> > nEvents(nSteps, std::vector<int>(NBX));
  std::vector<std::vector<int> > nEventsFilledBX(nSteps, std::vector<int>(NBX));

  typedef std::vector<std::vector<int> > vv;

  // Arrays to keep track of anaysis by channel and per bunch
  std::vector<vv> nAllTripleBX(nSteps, vv(nChannels, std::vector<int>(NBX)));
  std::vector<vv> nGoodTripleBX(nSteps, vv(nChannels, std::vector<int>(NBX)));
  std::vector<vv> nEmptyEventsFilledBXBX(nSteps, vv(nChannels, std::vector<int>(NBX)));
  std::vector<vv> nNonEmptyFilledBXBX(nSteps, vv(nChannels, std::vector<int>(NBX)));

  int nEventsByBX[NBX] = {0};
  float nEmptyEventsByBX[NBX] = {0.};

  std::vector<int> zeroVector(nChannels);

  // Double arrays to keep track channel-by-channel
  std::vector<std::vector<int> > nAllTripleByChannel(nSteps, zeroVector);
  std::vector<std::vector<int> > nGoodTripleByChannel(nSteps, zeroVector);
  std::vector<std::vector<int> > nEmptyEventsFilledBXByChannel(nSteps, zeroVector);
  std::vector<std::vector<int> > nNonEmptyFilledBXByChannel(nSteps, zeroVector);
  std::vector<std::vector<int> > nEmptyEventsByBXByChannel(3564, zeroVector);

  // Variables to keep track of dead channels
  std::map<Int_t, Int_t> startEvent;
  std::map<Int_t, std::string> startTime;
  std::map<Int_t, Int_t> endEvent;
  std::map<Int_t, std::string> endTime;
  Int_t offset = 1;
  Int_t zerosInARow[nChannels] = {0};

  // Variables to keep track of channels with desync errors
  std::vector<int> b;
  std::vector<int>& wrong = b; //Initialization
  std::vector<int> problems(nChannels);
  std::vector<int> channelHasIssuesNow(nChannels); //to indicate that a channel had many desync errors and hasn't recovered
  std::vector<int> hasProblemThisEvent(nChannels); //to indicate a channel had a Desyn error this event
  std::vector<int> errorChannels; //Keep track of which channels had problems
  std::vector<std::string> errorTimes; // and when
  Int_t newHits[nChannels] = {0};
    
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
    if (triggers[BX] < 0) continue;
    
    if (useTimestamps) {
      stepNumber = -1;
      if (Event.Time() > timestamps[nSteps-1].second) break;
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
	//This should never be needed with thsi file. Always run with time stamps
	/*	//std::cout << "starting new step @" << Event.Time() << " from " << currentStepStart << std::endl;
	timestamps.push_back(std::make_pair(currentStepStart, Event.Time()-1));
	nTotTracks.push_back(0);
	nGoodTracks.push_back(0);
	nAllTriple.push_back(0);
	nGoodTriple.push_back(0);
	nEvents.push_back(0);
	nEventsFilledBX.push_back(0);
	nEmptyEventsFilledBX.push_back(0);
	nNonEmptyFilledBX.push_back(0);
       
	nTotTracksByChannel.push_back(zeroVector);
        nGoodTracksByChannel.push_back(zeroVector);
        nAllTripleByChannel.push_back(zeroVector);
        nGoodTripleByChannel.push_back(zeroVector);
        nEmptyEventsFilledBXByChannel.push_back(zeroVector);
	nNonEmptyFilledBXByChannel.push_back(zeroVector);
  
	currentStepStart = Event.Time();
	stepNumber++;
	nSteps++;
	*/
      }
    }

    nEvents[stepNumber][BX]++; // Count number of events
    
    std::vector<int> hasGoodTracksInEvent = zeroVector;
    for (int i = 0; i < nChannels; ++i)
      newHits[i] = 0;
    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      
      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);
      int ch = Telescope->Channel();
           
      if (!MapSlopeY[Telescope->Channel()]) {
        TString Name = TString::Format("SlopeY_Ch%i", Telescope->Channel());
        MapSlopeY[Telescope->Channel()] = new TH1F(Name, Name, 40, -0.02, 0.06);
        MapSlopeY[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
        Name = TString::Format("SlopeX_Ch%i", Telescope->Channel());
        MapSlopeX[Telescope->Channel()] = new TH1F(Name, Name, 40, -0.04, 0.04);
        MapSlopeX[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
        Name = TString::Format("Slope2D_Ch%i", Telescope->Channel());
        MapSlope2D[Telescope->Channel()] = new TH2F(Name, Name, 100, -0.1, 0.1, 100, -0.1, 0.1);
        MapSlope2D[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
        MapSlope2D[Telescope->Channel()]->SetYTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
        Name = TString::Format("ResidualY%i_ROC0", Telescope->Channel());
        MapResidualY[Telescope->Channel()*10+0] = new TH1F(Name, Name, 40, -0.02, 0.06);
        MapResidualY[Telescope->Channel()*10+0]->SetXTitle("Local Telescope Residual-Y (cm)");
        Name = TString::Format("ResidualX%i_ROC0", Telescope->Channel());
        MapResidualX[Telescope->Channel()*10+0] = new TH1F(Name, Name, 40, -0.04, 0.04);
        MapResidualX[Telescope->Channel()*10+0]->SetXTitle("Local Telescope Residual-X (cm)");
        Name = TString::Format("ResidualY%i_ROC1", Telescope->Channel());
        MapResidualY[Telescope->Channel()*10+1] = new TH1F(Name, Name, 40, -0.02, 0.06);
        MapResidualY[Telescope->Channel()*10+1]->SetXTitle("Local Telescope Residual-Y (cm)");
        Name = TString::Format("ResidualX%i_ROC1", Telescope->Channel());
        MapResidualX[Telescope->Channel()*10+1] = new TH1F(Name, Name, 40, -0.04, 0.04);
        MapResidualX[Telescope->Channel()*10+1]->SetXTitle("Local Telescope Residual-X (cm)");
        Name = TString::Format("ResidualY%i_ROC2", Telescope->Channel());
        MapResidualY[Telescope->Channel()*10+2] = new TH1F(Name, Name, 40, -0.02, 0.06);
        MapResidualY[Telescope->Channel()*10+2]->SetXTitle("Local Telescope Residual-Y (cm)");
        Name = TString::Format("ResidualX%i_ROC2", Telescope->Channel());
        MapResidualX[Telescope->Channel()*10+2] = new TH1F(Name, Name, 40, -0.04, 0.04);
        MapResidualX[Telescope->Channel()*10+2]->SetXTitle("Local Telescope Residual-X (cm)");
      }

      if (Telescope->NTracks() > 0) {
	nAllTripleBX[stepNumber][ch][BX]++; // Count number of triple coincidences
	
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
	  nGoodTripleBX[stepNumber][ch][BX]++; //Count the number of valid triple coincidences
	  hasGoodTracksInEvent[ch]++; // Note that this channel had a good track
	}
      }
      newHits[ch] += (Int_t)Telescope->NTracks(); // count number of hits, to determine if channel is dead
    } // telescope loop
    
    nEventsByBX[Event.BX()]++; // Count number of events, sorted by BX

    nEventsFilledBX[stepNumber][BX]++; // Count number of events that happened in filled bunches
    
    for (Int_t index = 0; index < nValidChannels; ++index) {
      Int_t channel = validChannels[index];
      if (hasGoodTracksInEvent[channel] == 0) {
      	nEmptyEventsByBXByChannel[Event.BX()][channel]++; // Count number of events with no good tracks
      }
      if (hasGoodTracksInEvent[channel] > 0) {
	nNonEmptyFilledBXBX[stepNumber][channel][BX]++; // There were tracks!
      }      
      else {
	nEmptyEventsFilledBXBX[stepNumber][channel][BX]++; // No tracks 
      }
      
      ////////////
      // Begin checking for dead channels
      ////////////
      
      // Loop through all channels and note how many hits in each  
      // Keep track of how many times in a row a channel has had 0 hits
      // Increment the number of 0's in a row seen
      if (newHits[channel] == 0) {
	++zerosInARow[channel];
      }

      // Or reset that count to 0        
      else {
	if (zerosInARow[channel] > consecutiveZeros) {
	  std::cout << "Channel " << channel << " has recovered at event " << ientry << " at time " << Event.ReadableTime() << std::endl;
	  // Account for the possibility of multiple failures in the same channel
          // Store the errors as the number plus mutiples of 100
	  for (Int_t possibleOffset = 0; possibleOffset < offset; ++possibleOffset) {
	    if (endEvent[100 * possibleOffset + channel] == -1) {
	      endEvent[100 * possibleOffset + channel] = ientry;
	      endTime[100 * possibleOffset + channel] = Event.ReadableTime();
	    }
	  }
	}
	zerosInARow[channel] = 0;
      }

      // If a channel has had 0 hits for long enough, declare it dead
      if (zerosInARow[channel] == consecutiveZeros) {
	int nsec = Event.Time()/1000;
	int hr = nsec/3600;
	int min = (nsec-(hr*3600))/60;
	
	std::stringstream buf;
	buf << std::setfill('0') << std::setw(2) << hr << ":" << std::setw(2) << min;
	std::string failureTime = buf.str();
	std::cout << "Failure in Channel "  << channel << " at event " << 
	  ientry - consecutiveZeros + 1 << " at approximately  " << failureTime << std::endl;
	// If there's already a failure with this channel, the index is the offset plus the channel
	if (startEvent.count(channel)) {
	  startEvent.insert(std::pair<Int_t, Int_t>(offset*100 + channel, ientry - consecutiveZeros + 1));
	  startTime.insert(std::pair<Int_t, std::string> (offset * 100 + channel, failureTime));
	  endEvent.insert(std::pair<Int_t, Int_t> (offset * 100 + channel, -1));
	  endTime.insert(std::pair<Int_t, std::string> (offset * 100 + channel, "End"));
	  ++offset;
	}
	// If there isn't yet a failure in this channel, do it normally
	else {
	  startEvent.insert(std::pair<Int_t, Int_t>(channel, ientry - consecutiveZeros + 1));
	  startTime.insert(std::pair<Int_t, std::string> (channel, failureTime));
	  endEvent.insert(std::pair<Int_t, Int_t> (channel, -1));
	  endTime.insert(std::pair<Int_t, std::string> (channel, "End"));
	}
      }
    }

    ////////////
    // End checking for dead channels
    ////////////

    ////////////
    // Begin checking for desync errors
    ////////////

    // Keep track of desync events
    wrong = Event.getDesyncChannels(); //List of channels with desync error
    for (size_t index = 0; index < wrong.size(); ++index) {
      int errorCh = wrong[index];
      hasProblemThisEvent[errorCh] = 1; // show this channel has a problem this event
      if (!channelHasIssuesNow[errorCh]) { // if we haven't identified this as having consistent problems
	if (problems[errorCh] == 0)
	  std::cout<<"Channel "<<errorCh<<" temporarily desynced at "<<Event.ReadableTime()<<"   :"<<ientry<<std::endl;
	if (problems[errorCh] >= 20000) { //now identify it as having consistent problems
	  errorChannels.push_back(errorCh);
	  errorTimes.push_back(Event.ReadableTime());
	  std::cout<<"Channel "<<errorCh<<" very very very very very very desynced at "<<Event.ReadableTime()<<"    :"<<ientry<<std::endl;
	  channelHasIssuesNow[errorCh] = 1;
	}
      }
      if (problems[errorCh] < 20000)
        problems[errorCh]++;
    }
    //decrement the count until its zero. This is to make sure that the channel really is done 
    // having problems, and that it doesn't just temporarily become good but then go bad again
    for (int channel = 0; channel < nChannels; ++channel) {
      if (channelHasIssuesNow[channel]) { 
	if (!hasProblemThisEvent[channel]) {
	  problems[channel]--;
	  if (problems[channel] < 1) {
	    std::cout<<"Channel "<<channel<<" resynced at "<<Event.ReadableTime()<<"     :"<<ientry<<std::endl;
	    channelHasIssuesNow[channel] = 0;
	  }
	}
	else 
	  hasProblemThisEvent[channel] = 0;
      }
    }

    ////////////
    // End check for desync errors
    ////////////
    
  } // event loop
    // properly catch the last step
  timestamps.push_back(std::make_pair(currentStepStart, Event.Time()));

  //Print out the desync errors
  for (size_t i = 0; i < errorChannels.size(); ++i) 
    std::cout<<"There was an desync error in channel "<<errorChannels[i]<<" at time "<<errorTimes[i]<<std::endl;

  // Print out the channel failures
  for (std::map<Int_t, Int_t>::iterator it = startEvent.begin(); it != startEvent.end(); it++) {
    Int_t channel = (it->first) % 100;
    if ((startEvent[channel] == 0) && (endEvent[channel] == -1))
      std::cout << "Channel " << channel << " was dead for the full fill" << std::endl;
    else {                                                               
      std::cout << "Error in channel " << channel << " at event " << startEvent[channel] <<
	" at approximately " << startTime[channel] << std::endl;
      if (endEvent[channel] == -1)
	std::cout << "\tFailure lasted through the end of the fill" << std::endl;
      else
	std::cout << "\tFailure ended at event " << endEvent[channel] << " at time " <<
	  endTime[channel] << std::endl;
    }
  }
  
  // Make a vector of the good channels (that didn't die in the fill)
  // These we will use to calculate luminosities
  // Desync errors are ignored
  std::vector<Int_t> functionalChannels;
  for (int index = 0; index < nValidChannels; ++index) {
    if (startEvent.count(validChannels[index]) == (Int_t)0) {
      functionalChannels.push_back(validChannels[index]);
    }
  }
 
  TFile *f = new TFile("histo_slopes.root","RECREATE");
  FILE *textf = fopen("TrackDistributions.txt", "w");

  TCanvas Can("BeamSpot", "BeamSpot", 900, 900);
  Can.Divide(3, 3);
  Can.cd(1);
  HistBeamSpot[0]->SetXTitle("X(cm)");
  HistBeamSpot[0]->SetYTitle("Y(cm)");
  HistBeamSpot[0]->Draw("colz");
  Can.cd(2);
  HistBeamSpot[1]->SetXTitle("X(cm)");
  HistBeamSpot[1]->SetYTitle("Y(cm)");
  HistBeamSpot[1]->Draw("colz");
  Can.cd(3);
  HistBeamSpot[2]->SetXTitle("X(cm)");
  HistBeamSpot[2]->SetYTitle("Y(cm)");
  HistBeamSpot[2]->Draw("colz");

  HistBeamSpot[3]->SetXTitle("X (cm)");
  HistBeamSpot[3]->SetYTitle("Y (cm)");
  HistBeamSpot[3]->Draw("colz");

  Can.cd(1+3);
  HistBeamSpot[0]->ProjectionX()->Draw("ep");
  Can.cd(2+3);
  HistBeamSpot[1]->ProjectionX()->Draw("ep");
  Can.cd(3+3);
  HistBeamSpot[2]->ProjectionX()->Draw("ep");

  Can.cd(1+6);
  HistBeamSpot[0]->ProjectionY()->Draw("ep");
  Can.cd(2+6);
  HistBeamSpot[1]->ProjectionY()->Draw("ep");
  Can.cd(3+6);
  HistBeamSpot[2]->ProjectionY()->Draw("ep");
  Can.SaveAs("plots/BeamSpot.gif");
  
  HistBeamSpot[0]->Write();
  HistBeamSpot[1]->Write();
  HistBeamSpot[2]->Write();
  HistBeamSpot[3]->Write();

  fprintf(textf, "%d\n", (int)MapSlopeY.size());

  for (std::map<int, TH1F*>::iterator it = MapSlopeY.begin(); it != MapSlopeY.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH1F*>::iterator it = MapSlopeX.begin(); it != MapSlopeX.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH2F*>::iterator it = MapSlope2D.begin(); it != MapSlope2D.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = MapResidualY.begin(); it != MapResidualY.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = MapResidualX.begin(); it != MapResidualX.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  
  f->Close();
  fclose(textf);
  FILE *outf = fopen("TrackLumiZC.txt", "w");
  fprintf(outf, "%d %d\n", nSteps, nFilledBunches);

  // Find averages over all valid telescopes
  float allTripleTot;
  float goodTripleTot;
  float emptyEventsFilledBXTot;
  float nonEmptyFilledBXTot;
  float emptyEventsByBXTot;
  float nFunctionalChannels = (float)functionalChannels.size();
  for (int stepNum = 0; stepNum < nSteps; ++stepNum) {
    for (int BXnum = 0; BXnum < NBX; ++BXnum) {
      allTripleTot = 0.;
      goodTripleTot = 0.;
      emptyEventsFilledBXTot = 0.;
      nonEmptyFilledBXTot = 0.;
      for (std::vector<int>::iterator it = functionalChannels.begin(); it != functionalChannels.end(); ++it) {
	int channel = *it;
	allTripleTot += nAllTripleBX[stepNum][channel][BXnum];
	goodTripleTot += nGoodTripleBX[stepNum][channel][BXnum];
	emptyEventsFilledBXTot += nEmptyEventsFilledBXBX[stepNum][channel][BXnum];
	nonEmptyFilledBXTot += nNonEmptyFilledBXBX[stepNum][channel][BXnum];
      }
      // For these we want the average 
      nAllTriple[stepNum][BXnum] = allTripleTot/nFunctionalChannels;
      nGoodTriple[stepNum][BXnum] = goodTripleTot/nFunctionalChannels;
      nEmptyEventsFilledBX[stepNum][BXnum] = emptyEventsFilledBXTot/nFunctionalChannels;
      nNonEmptyFilledBX[stepNum][BXnum] = nonEmptyFilledBXTot/nFunctionalChannels;
    }
  }


  for (int i = 0; i < NBX; ++i) {
    emptyEventsByBXTot = 0.;    
    for (std::vector<int>::iterator it = functionalChannels.begin(); it != functionalChannels.end(); ++it) {
      int channel = *it;
      emptyEventsByBXTot += nEmptyEventsByBXByChannel[i][channel];
    }
    nEmptyEventsByBX[i] = emptyEventsByBXTot/nFunctionalChannels;
  }

  for (int BX = 0; BX < NBX; ++BX) {
    if (triggers[BX] < 0) continue;
    std::stringstream buf;
    buf << "TrackLumiZC" << std::setfill('0') << std::setw(4) << BX+1 << std::setw(0) << ".txt";
    std::string name = buf.str();
    FILE *outf = fopen(name.c_str(), "w");
    fprintf(outf, "%d %d\n", nSteps, nFilledBunches);

    for (int i=0; i<nSteps; ++i) {
    // Used if you don't want all the channel data
      fprintf(outf, "%d %d %d %lf %lf %d %lf %lf\n", timestamps[i].first, timestamps[i].second, nEvents[i][BX], nAllTriple[i][BX], nGoodTriple[i][BX], nEventsFilledBX[i][BX], nEmptyEventsFilledBX[i][BX], nNonEmptyFilledBX[i][BX]);
    }
    fclose(outf);
  }
  /*
  std::cout << "==Zero counting by BX==" << std::endl;
  for (int index = 0; index < NBX; ++index) {
    if (filledBunches[index] == 1)
      std::cout << index+1 << " " << nEventsByBX[index] << " " << nEmptyEventsByBX[index] << std::endl;
      }*/
  
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
