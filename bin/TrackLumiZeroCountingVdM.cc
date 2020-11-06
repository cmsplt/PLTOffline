// USED FOR THE VDM SCAN, HAS EXCESSIVELY LONG NUMBERS TO ACCOUNT FOR THIS
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

int TrackLumiZeroCounting(const std::string DataFileName, const std::string GainCalFileName, const std::string AlignmentFileName,
		       const std::string TrackDistributionFileName, const std::string TimestampFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;
  std::cout << "TrackDistributionFileName: " << TrackDistributionFileName << std::endl;
  if (TimestampFileName != "")
    std::cout << "TimestampFileName: " << TimestampFileName << std::endl;

  // note: bunches internally are in 0-counting convention
  std::vector<Int_t> filledBunches = FindActiveBunches(DataFileName);
  
  // FindActiveBunches returns the number of active bunches as the last element
  int nFilledBunches = filledBunches[NBX];
  filledBunches.pop_back();
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
  std::vector<float> nAllTriple(nSteps);
  std::vector<float> nGoodTriple(nSteps);
  std::vector<int> nEvents(nSteps);
  std::vector<int> nEventsFilledBX(nSteps);
  std::vector<float> nEmptyEventsFilledBX(nSteps);
  std::vector<float> nNonEmptyFilledBX(nSteps);
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
	//std::cout << "starting new step @" << Event.Time() << " from " << currentStepStart << std::endl;
	timestamps.push_back(std::make_pair(currentStepStart, Event.Time()-1));
	nAllTriple.push_back(0);
	nGoodTriple.push_back(0);
	nEvents.push_back(0);
	nEventsFilledBX.push_back(0);
	nEmptyEventsFilledBX.push_back(0);
	nNonEmptyFilledBX.push_back(0);
       
        nAllTripleByChannel.push_back(zeroVector);
        nGoodTripleByChannel.push_back(zeroVector);
        nEmptyEventsFilledBXByChannel.push_back(zeroVector);
        nNonEmptyFilledBXByChannel.push_back(zeroVector);
  
	currentStepStart = Event.Time();
	stepNumber++;
	nSteps++;
      }
    }

    nEvents[stepNumber]++; // Count number of events
    
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
	nAllTripleByChannel[stepNumber][ch]++; // Count number of triple coincidences
	
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
	  nGoodTripleByChannel[stepNumber][ch]++; //Count the number of valid triple coincidences
	  hasGoodTracksInEvent[ch]++; // Note that this channel had a good track
	}
      }
      newHits[ch] += (Int_t)Telescope->NTracks(); // count number of hits, to determine if channel is dead
    } // telescope loop
    
    nEventsByBX[Event.BX()]++; // Count number of events, sorted by BX

    if (filledBunches[Event.BX()])
      nEventsFilledBX[stepNumber]++; // Count number of events that happened in filled bunches
    
    for (Int_t index = 0; index < nValidChannels; ++index) {
      Int_t channel = validChannels[index];
      if (hasGoodTracksInEvent[channel] == 0)
      	nEmptyEventsByBXByChannel[Event.BX()][channel]++; // Count number of events with no good tracks
      if (filledBunches[Event.BX()]) { // Look now only at events from filled bunches
	if (hasGoodTracksInEvent[channel] > 0)
	  nNonEmptyFilledBXByChannel[stepNumber][channel]++; // There were tracks!
	else
	  nEmptyEventsFilledBXByChannel[stepNumber][channel]++; // No tracks 
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
    /*    for (int channel = 0; channel < nChannels; ++channel) {
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
    */

    for (int channel = 0; channel < nChannels; ++channel) {
      if (channelHasIssuesNow[channel]) {
        if (!hasProblemThisEvent[channel]) {
          // problems[channel]--;                                               
          if (problems[channel] < 1) {
	    std::cout<<"Channel "<<channel<<" resynced at "<<Event.ReadableTime\
	      ()<<"     :"<<ientry<<std::endl;
            channelHasIssuesNow[channel] = 0;
          }
        }
        else
          hasProblemThisEvent[channel] = 0;
      }
      if (!hasProblemThisEvent[channel]) {
        if (problems[channel] > 0)
          problems[channel]--;
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
    allTripleTot = 0.;
    goodTripleTot = 0.;
    emptyEventsFilledBXTot = 0.;
    nonEmptyFilledBXTot = 0.;
    for (std::vector<int>::iterator it = functionalChannels.begin(); it != functionalChannels.end(); ++it) {
      int channel = *it;
      allTripleTot += nAllTripleByChannel[stepNum][channel];
      goodTripleTot += nGoodTripleByChannel[stepNum][channel];
      emptyEventsFilledBXTot += nEmptyEventsFilledBXByChannel[stepNum][channel];
      nonEmptyFilledBXTot += nNonEmptyFilledBXByChannel[stepNum][channel];
    }
    // For these we want the average 
    nAllTriple[stepNum] = allTripleTot/nFunctionalChannels;
    nGoodTriple[stepNum] = goodTripleTot/nFunctionalChannels;
    nEmptyEventsFilledBX[stepNum] = emptyEventsFilledBXTot/nFunctionalChannels;
    nNonEmptyFilledBX[stepNum] = nonEmptyFilledBXTot/nFunctionalChannels;
  }

  for (int i = 0; i < NBX; ++i) {
    emptyEventsByBXTot = 0.;    
    for (std::vector<int>::iterator it = functionalChannels.begin(); it != functionalChannels.end(); ++it) {
      int channel = *it;
      emptyEventsByBXTot += nEmptyEventsByBXByChannel[i][channel];
    }
    nEmptyEventsByBX[i] = emptyEventsByBXTot/nFunctionalChannels;
  }

  for (int i=0; i<nSteps; ++i) {
    // Used if you don't want all the channel data
     //  fprintf(outf, "%d %d %d %lf %lf %d %lf %lf\n", timestamps[i].first, timestamps[i].second, nEvents[i], nAllTriple[i], nGoodTriple[i], nEventsFilledBX[i], nEmptyEventsFilledBX[i], nNonEmptyFilledBX[i]);
  
    //Used for if you want all the channel data
    fprintf(outf, "%d %d %d %lf %lf %d %lf %lf    %d %d %d %d %d %d %d %d %d %d %d %d %d\n", timestamps[i].first, timestamps[i].second, nEvents[i], nAllTriple[i], nGoodTriple[i], nEventsFilledBX[i], nEmptyEventsFilledBX[i], nNonEmptyFilledBX[i], nNonEmptyFilledBXByChannel[i][2], nNonEmptyFilledBXByChannel[i][4], nNonEmptyFilledBXByChannel[i][5], nNonEmptyFilledBXByChannel[i][7], nNonEmptyFilledBXByChannel[i][8], nNonEmptyFilledBXByChannel[i][10], nNonEmptyFilledBXByChannel[i][11], nNonEmptyFilledBXByChannel[i][13], nNonEmptyFilledBXByChannel[i][14], nNonEmptyFilledBXByChannel[i][16], nNonEmptyFilledBXByChannel[i][17], nNonEmptyFilledBXByChannel[i][19], nNonEmptyFilledBXByChannel[i][20]);
}
  fclose(outf);

  std::cout << "==Zero counting by BX==" << std::endl;
  for (int index = 0; index < NBX; ++index) {
    if (filledBunches[index] == 1)
      std::cout << index+1 << " " << nEventsByBX[index] << " " << nEmptyEventsByBX[index] << std::endl;
  }
  
return 0;
}

std::vector<int> FindActiveBunches(const std::string DataFileName) {

  // Set some basic style
  PLTU::SetStyle();
  std::string DataFileName2 = DataFileName; //"/raid/PLT/SlinkData_2016/Slink_20160527.124811.dat";
  // Grab the plt event reader
  PLTEvent Event(DataFileName2);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  TProfile *avg = new TProfile("avg", "Average Number of Hits;Bunch ID;Average Hits per Bin", NBX, 0.5, NBX + .5);
  // Loop over enough events in the file to determine which are filled 
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 50000 == 0) {
      std::cout << "Processing event: " << ientry << " at " << Event.ReadableTime() << " to determine active bunches" << std::endl;
    }
    if (ientry > 10000000)
      break;
    // +1 because BX starts counting at 0, but LHC convention is to start at 1  
    avg->Fill(Event.BX()+1, Event.NHits());  
  }
  std::vector<Double_t> num;
  
  // Make a vector for all the bin values and sort it                           
  for (Int_t index = 1; index <= avg->GetXaxis()->GetNbins(); ++index) {
    num.push_back(avg->GetBinContent(index));
  }
  std::sort(num.begin(), num.end());
  
  // Find a large jump in the occupancy of each bin                             
  Double_t runningAvg = 0;
  Int_t breakPoint = 0;
  for (Int_t index = 1; index < NBX; ++index) {
    Double_t diff = num[index] - num[index - 1];
    
    // 6000 is somewhat arbitrary, but it worked. Adjust as need be.             
    // Also wait a few events (here 4) to build up a running average first
    if (diff > 6000*runningAvg && num[index-1] != 0 && (index > 4)) {
      breakPoint = index;
      break;
    }
    // Keep track of the average difference so far. 
    // We want this average to be greatly exceeded at the jump                                                    
    runningAvg = ((Double_t)(runningAvg*(index - 1) + diff))/((Double_t)index);
  }

  // Error handling. If this happens try changing the 6000 above                
  if (breakPoint == 0)
    std::cout << "Did not find large gap in data; break between active and" <<
      " inactive may be unclear" << std::endl;
  
  // Make a vector of the active bunches. Note this counting convention starts at 0. 
  // The last element will be the number of active bunches
   std::vector<Int_t> activeBunches(NBX + 1);
   for (int index = 1; index < avg->GetXaxis()->GetNbins(); ++index) {
     if (avg->GetBinContent(index) >= num[breakPoint]) {
       activeBunches[index - 1] = 1;
       activeBunches[NBX]++;
     }
     else
       activeBunches[index - 1] = 0;
   }
   
  return activeBunches;
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
