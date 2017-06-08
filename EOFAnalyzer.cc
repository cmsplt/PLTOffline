////////////////////////////////////////////////////////////////
/// This file is used to analyze the Slink data automatically after a fill is dumped.
/// The analyzer consists of three basic sub analyzers: TrackLumiZeroCounting, MeasureAccidentals, and ROCEfficiency
///   Joseph Heideman, Thoth Gunter, Paul Lujan, Andres Delannoy
///
///////////////////////////////////////////////////////////////



#include <iostream>
#include <string>
#include <iomanip>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TFile.h"
#include "TProfile.h"
#include "PLTBinaryFileReader.h"

class HitCounter
{
  public:
    HitCounter () {
      NFiducial[0] = 0;
      NFiducial[1] = 0;
      NFiducial[2] = 0;
      NFiducialAndHit[0] = 0;
      NFiducialAndHit[1] = 0;
      NFiducialAndHit[2] = 0;
    }
    ~HitCounter () {}

    int NFiducial[3];
    int NFiducialAndHit[3];
};

int EOFAnalyzer(const std::string, const std::string, const std::string, const std::string);
int TrackingEfficiency (std::string const, std::string const, std::string const);
std::vector<Int_t> FindActiveBunches(const std::string);

const Int_t NBX = 3564;
const Int_t nChannels = 24;
const Int_t consecutiveZeros = 150000; // Number of events with no hits before declaring channel dead
const Int_t nValidChannels = 16; // Channels that could in theory have hits.
const Int_t validChannels[nValidChannels] = {1, 2, 4, 5, 7, 8, 10, 11, 13, 14,
                                             16, 17, 19, 20, 22, 23};


int EOFAnalyzer(const std::string DataFileName, const std::string GainCalFileName, const std::string AlignmentFileName,
		const std::string TrackDistributionFileName, const std::string TimestampFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;
  std::cout << "TrackQualityFileName: " << TrackDistributionFileName << std::endl;
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

  // note: bunches internally are in 0-counting convention
  std::vector<Int_t> filledBunches = FindActiveBunches(DataFileName);
  
  // FindActiveBunches returns the number of active bunches as the last element
  int nFilledBunches = filledBunches[NBX];
  filledBunches.pop_back();
  std::cout << "Total of " << nFilledBunches << " bunches filled" << std::endl;


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
//      slope_y_low[ch] = mean-sigma*5.0;
//      slope_y_high[ch] =  mean+sigma*5.0;
    }
    for (int i=0; i<nch; ++i) {
      fscanf(qfile, "SlopeX_Ch%d %f %f\n", &ch, &mean, &sigma);
      MeanSlopeX[ch] = mean;
      SigmaSlopeX[ch] = sigma;
//      slope_x_low[ch] = mean-sigma*5.0;
//      slope_x_high[ch] =  mean+sigma*5.0;
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
    std::cout << nSteps << " timestamps" << std::endl;
    // for (unsigned int i=0; i<timestamps.size(); ++i) {
    //   std::cout << "start: " << timestamps[i].first << " end: " << timestamps[i].second << std::endl;
    // }
  }

/////////// MeasureAcc and TLZC Setup
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.ReadOnlinePixelMask("Mask_2017_24x36center_26x38outer.txt");
//  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_FullSensor;
//  Event.SetPlaneFiducialRegion(FidRegionHits);
//  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);
//  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_01to2_AllCombs);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, FidRegionHits);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_01to2_AllCombs);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);


//  std::vector<double> accidentalRate;
//  std::vector<double> accidentalRateErr;
  std::vector<double> trackLumiGood, trackLumiErr;
  std::map<int, int> NTrkEvMap;


  std::vector<float> nAllTriple(nSteps);
  std::vector<float> nGoodTriple(nSteps);
  std::vector<int> nEvents(nSteps);
  std::vector<int> nEventsFilledBX(nSteps);
  std::vector<float> nEmptyEventsFilledBX(nSteps);
  std::vector<float> nNonEmptyFilledBX(nSteps);

  int nEventsByBX[NBX] = {0};
  float nEmptyEventsByBX[NBX] = {0.};

  std::vector<int> zeroVector(nChannels);

  std::vector<std::vector<int> > nAllTripleByChannel(nSteps, nChannels);
  std::vector<std::vector<int> > nGoodTripleByChannel(nSteps, nChannels);
  std::vector<std::vector<int> > nEmptyEventsFilledBXByChannel(nSteps, nChannels);
  std::vector<std::vector<int> > nNonEmptyFilledBXByChannel(nSteps, nChannels);
  std::vector<std::vector<int> > nEmptyEventsByBXByChannel(3564, nChannels);

  // Variables to keep track of dead channels
  std::map<Int_t, Int_t> startEvent;
  std::map<Int_t, std::string> startTime;
  std::map<Int_t, Int_t> endEvent;
  std::map<Int_t, std::string> endTime;
  Int_t offset = 1;
  Int_t zerosInARow[nChannels] = {0};
//  Int_t blanks = 0;

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
  //const uint32_t stepLength = 3000; //30sec// 1 minute
  const uint32_t stepLength = 300000; // 5 minutes
  //const uint32_t lsLength= 23.31; // 1 lumisection


  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry > 10000) break;
//    if (ientry % 100000 == 0) std::cout << "Processing event " << ientry << std::endl;
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
    for (int i = 0; i < nChannels; ++i)   newHits[i] = 0;

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      
      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      int ch = Telescope->Channel();
/////////////////////////////////////////////////////////////////////////////

      if (Telescope->NTracks() > 0) {
	nAllTripleByChannel[stepNumber][ch]++; // Count number of triple coincidences
        nAllTriple[stepNumber]++; // increment overall track number	

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
          nGoodTriple[stepNumber]++;
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
 
      // Loop through all channels and note how many hits in each   // Keep track of how many times in a row a channel has had 0 hits
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
   if (0) { 
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
	if (problems[errorCh] >= 2000) { //now identify it as having consistent problems
	  errorChannels.push_back(errorCh);
	  errorTimes.push_back(Event.ReadableTime());
	  std::cout<<"Channel "<<errorCh<<" very very very very very very desynced at "<<Event.ReadableTime()<<"    :"<<ientry<<std::endl;
	  channelHasIssuesNow[errorCh] = 1;
	}
      }
      if (problems[errorCh] < 2000)
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
   }//end channel dropout & desync check

  } // event loop
    // properly catch the last step
  timestamps.push_back(std::make_pair(currentStepStart, Event.Time()));

  std::vector<Int_t> functionalChannels;
  for (int index = 0; index < nValidChannels; ++index) {
    if (startEvent.count(validChannels[index]) == (Int_t)0) {
      functionalChannels.push_back(validChannels[index]);
    }
  }


///// Find averages over all valid telescopes

//  float allTripleTot;
//  float goodTripleTot;
  float emptyEventsFilledBXTot;
  float nonEmptyFilledBXTot;
  float emptyEventsByBXTot;
//  float nFunctionalChannels = (float)functionalChannels.size();
  float nFunctionalChannels = 16;
  for (int stepNum = 0; stepNum < nSteps; ++stepNum) {
//    allTripleTot = 0.;
//    goodTripleTot = 0.;
    emptyEventsFilledBXTot = 0.;
    nonEmptyFilledBXTot = 0.;
      for (int it = 0; it < nChannels; it++){
      int channel = it;
//      allTripleTot += nAllTripleByChannel[stepNum][channel];
//      goodTripleTot += nGoodTripleByChannel[stepNum][channel];
      emptyEventsFilledBXTot += nEmptyEventsFilledBXByChannel[stepNum][channel];
      nonEmptyFilledBXTot += nNonEmptyFilledBXByChannel[stepNum][channel];
    }
    // For these we want the average 
//    nAllTriple[stepNum] = allTripleTot/nFunctionalChannels;
//    nGoodTriple[stepNum] = goodTripleTot/nFunctionalChannels;
    nEmptyEventsFilledBX[stepNum] = emptyEventsFilledBXTot/nFunctionalChannels;
    nNonEmptyFilledBX[stepNum] = nonEmptyFilledBXTot/nFunctionalChannels;
  }

  for (int i = 0; i < NBX; ++i) {
    emptyEventsByBXTot = 0.;    
    for (int it = 0; it < nChannels; it++){
      int channel = it;
      emptyEventsByBXTot += nEmptyEventsByBXByChannel[i][channel];
    }
    nEmptyEventsByBX[i] = emptyEventsByBXTot/nFunctionalChannels;
  }

//  std::cout << "==Zero counting by BX==" << std::endl;
//  for (int index = 0; index < NBX; ++index) {
//    if (filledBunches[index] == 1)
//      std::cout << index+1 << " " << nEventsByBX[index] << " " << nEmptyEventsByBX[index] << std::endl;
//  }

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

/////////////////////////////////////////////////////////////////////////////////////
///    for (int i=0; i<nSteps; ++i) {
///      // Used if you don't want all the channel data
///       //  fprintf(outl, "%d %d %d %lf %lf %d %lf %lf\n", timestamps[i].first, timestamps[i].second, nEvents[i], nAllTriple[i], nGoodTriple[i], nEventsFilledBX[i], nEmptyEventsFilledBX[i], nNonEmptyFilledBX[i]);
///    
///      //Used for if you want all the channel data
///      fprintf(outl, "%d %d %d %lf %lf %d %lf %lf    %d %d %d %d %d %d %d %d %d %d %d %d %d\n", timestamps[i].first, timestamps[i].second, nEvents[i], nAllTriple[i], nGoodTriple[i], nEventsFilledBX[i], nEmptyEventsFilledBX[i], nNonEmptyFilledBX[i], nNonEmptyFilledBXByChannel[i][2], nNonEmptyFilledBXByChannel[i][4], nNonEmptyFilledBXByChannel[i][5], nNonEmptyFilledBXByChannel[i][7], nNonEmptyFilledBXByChannel[i][8], nNonEmptyFilledBXByChannel[i][10], nNonEmptyFilledBXByChannel[i][11], nNonEmptyFilledBXByChannel[i][13], nNonEmptyFilledBXByChannel[i][14], nNonEmptyFilledBXByChannel[i][16], nNonEmptyFilledBXByChannel[i][17], nNonEmptyFilledBXByChannel[i][19], nNonEmptyFilledBXByChannel[i][20]);
///  }
///    fclose(outl);
//////////////////////// End of old Output ///////////////////////


////////////////Combined Accidentals and TrackLumiZC////////////////////////////
//double fitSlope[17], fitOffset[17], fitSlopeErr[17], fitOffsetErr[17];

//const int nS = nSteps;
// std::vector<double> accidentalRate(nSteps,0.0); //= {0};
// std::vector<double> accidentalRateErr(nSteps,0.0);// = {0};
double accidentalRate, accidentalRateErr;
 static const float calibration = 3.0;
 double accrate;
 float nAll;
 Int_t iCh;
  std::cout << "Printing accidentals and error by channel" << std::endl;
  for (int i=0; i<nSteps; ++i) {
   std::cout << "Step " << i << std::endl;
   for (int ichan = 0; ichan < 17; ichan++){
////    if ((nMeas == 0) || (nFilledTrig == (int)0) || (nBunches == 0) || (tracksAll == 0) || (tBegin == tEnd) (nFull == nFilledTrig) || (tracksAll == nTrig) || (nFull == 0.0))      continue;  /////// update to arrays for acc/ZC
//   if((double)(totLumi[i]/(n[i]*nBunches)) < 1.0) continue ; // SBIL to low for Accidental Statistics
   if (ichan == 0 || ichan == 4) continue;
   std::cout << ichan << " ";
   iCh = validChannels[ichan];

   if (ichan != 16){
    std::cout << iCh << " " << nAllTripleByChannel[i][iCh] << " " << nGoodTripleByChannel[i][iCh] << std::endl;
    accrate = (double)(nAllTripleByChannel[i][iCh]-nGoodTripleByChannel[i][iCh])/nAllTripleByChannel[i][iCh];
    nAll = nAllTripleByChannel[i][iCh];
      }
	else{
    std::cout << nAllTriple[i] << " " << nGoodTriple[i] << std::endl;
    accrate = (double)(nAllTriple[i]-nGoodTriple[i])/nAllTriple[i];
    nAll = nAllTriple[i];
	}

    accidentalRate = (double)(100.0*accrate);
    accidentalRateErr = (double)(100.0*sqrt(accrate*(1-accrate)/nAll));
//    std::cout << accidentalRate << " " << accidentalRateErr << " ";
    }

   std::cout << std::endl;
   }
  std::cout << "Printing TrackLumiZC" << std::endl;
  for (int i=0; i<nSteps; ++i) {
    ////////TrackLumiZC calculation///////
//    trackLumiAll.push_back(-log(1.0-(double)tracksAll/nTrig));
    int nFilledTrig = nEventsFilledBX[i]; 
    double nFull = nNonEmptyFilledBX[i];
    double f0 = 1.0-(double)nFull/nFilledTrig;
    // It's unclear whether the error should include 1/sqrt(n_channels) or not. If I do include it
    // the errors look too small, so I'm leaving it out for the moment.
    double f0err = sqrt(f0*(1-f0)/nFilledTrig);///sqrt(13.);
    double goodTrackZC = -log(f0)*calibration;
    double goodTrackZCPlus = -log(f0-f0err)*calibration;
    trackLumiGood.push_back(goodTrackZC);
    trackLumiErr.push_back(fabs(goodTrackZCPlus-goodTrackZC));
   std::cout << goodTrackZC << std::endl;
   
    }
   std::cout << "\n";  
//  Event.Clear();
 
return 0;
  
}

std::vector<int> FindActiveBunches(const std::string DataFileName) {

  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  TProfile *avg = new TProfile("avg", "Average Number of Hits;Bunch ID;Average Hits per Bin", NBX, 0.5, NBX + .5);
  // Loop over enough events in the file to determine which are filled 
  //  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {}
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 50000 == 0) {
      std::cout << "Processing event: " << ientry << " at " << Event.ReadableTime() << " to determine active bunches" << std::endl;
    }
    if (ientry > 1000000)      break;
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

int TrackingEfficiency (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();
  // Grab the plt event reader
  PLTEvent EventTE(DataFileName, GainCalFileName, AlignmentFileName);
  EventTE.ReadOnlinePixelMask("Mask_2016_VdM_v1.txt");
  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_FullSensor;
  EventTE.SetPlaneFiducialRegion(FidRegionHits);
  EventTE.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);
  EventTE.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_01to2_AllCombs);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Map for tracking efficiency
  std::map<uint32_t, std::map<int, HitCounter> > HC;
  
  float const PixelDist = 5;//<-Original
  float slope_x_low = 0.0 - 0.01;
  float slope_y_low = 0.027 - 0.01;
  float slope_x_high = 0.0 + 0.01;
  float slope_y_high = 0.027 + 0.01;

  uint32_t date = 0;
  uint32_t five_min = 5 * 60 * 1000; 

  // Loop over all events in file
  for (int ientry = 0; EventTE.GetNextEvent() >= 0; ++ientry) {
    if (ientry > 20000 * 50) break;
    if (ientry == 0){
      date = EventTE.Time();
    }

    if (ientry % 10000000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    // Loop over all planes with hits in event
    for (size_t it = 0; it != EventTE.NTelescopes(); ++it) {
      PLTTelescope* Telescope = EventTE.Telescope(it);

      int    const Channel = Telescope->Channel();
      size_t const NPlanes = Telescope->NPlanes();

      // make them clean events
      if (Telescope->NHitPlanes() < 2 || (unsigned)(Telescope->NHitPlanes()) != Telescope->NClusters()) {
        continue;
      }



      PLTPlane* Plane[3] = {0x0, 0x0, 0x0};
      for (size_t ip = 0; ip != NPlanes; ++ip) {
        Plane[ Telescope->Plane(ip)->ROC() ] = Telescope->Plane(ip);
      }

      // To construct 3 tracks.. one testing each plane
      PLTTrack Tracks[3];

      // 2-plane tracks
      if (Plane[0]->NClusters() && Plane[1]->NClusters()) {
        Tracks[0].AddCluster(Plane[0]->Cluster(0));
        Tracks[0].AddCluster(Plane[1]->Cluster(0));
        Tracks[0].MakeTrack(Alignment);
      }
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        Tracks[1].AddCluster(Plane[0]->Cluster(0));
        Tracks[1].AddCluster(Plane[2]->Cluster(0));
        Tracks[1].MakeTrack(Alignment);
      }
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        Tracks[2].AddCluster(Plane[1]->Cluster(0));
        Tracks[2].AddCluster(Plane[2]->Cluster(0));
        Tracks[2].MakeTrack(Alignment);
      }


      // 5 minute interval selection 
      if (EventTE.Time() > date + five_min*2 ) date = EventTE.Time();  
      // Test of plane 2
      if (Plane[0]->NClusters() && Plane[1]->NClusters()) {
        if (Tracks[0].IsFiducial(Channel, 2, Alignment, EventTE.PixelMask()) && Tracks[0].NHits() == 2 && Tracks[0].fTVY/Tracks[0].fTVZ < slope_y_high && Tracks[0].fTVY/Tracks[0].fTVZ > slope_y_low && Tracks[0].fTVX/Tracks[0].fTVZ < slope_x_high && Tracks[0].fTVX/Tracks[0].fTVZ > slope_x_low ) {
          ++HC[date][Channel].NFiducial[2];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 2);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[0].TX( CP->LZ ), Tracks[0].TY( CP->LZ ), Channel, 2);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
	  float cluster_charge = 0;
	  if(Plane[2]->NClusters()) cluster_charge=Plane[2]->Cluster(0)->Charge();
          if (Plane[2]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[0].LResiduals( *(Plane[2]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (fabs(RPXY.first) <= PixelDist && fabs(RPXY.second) <= PixelDist) {
              ++HC[date][Channel].NFiducialAndHit[2];
            } 
          }
        }
      }

      // Test of plane 1
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[1].IsFiducial(Channel, 1, Alignment, EventTE.PixelMask()) && Tracks[1].NHits() == 2 && Tracks[1].fTVY/Tracks[1].fTVZ < slope_y_high && Tracks[1].fTVY/Tracks[1].fTVZ > slope_y_low && Tracks[1].fTVX/Tracks[1].fTVZ < slope_x_high && Tracks[1].fTVX/Tracks[1].fTVZ > slope_x_low) {
          ++HC[date][Channel].NFiducial[1];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 1);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[1].TX( CP->LZ ), Tracks[1].TY( CP->LZ ), Channel, 1);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
	  float cluster_charge = 0;
	  if(Plane[1]->NClusters()) cluster_charge=Plane[1]->Cluster(0)->Charge();
          if (Plane[1]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[1].LResiduals( *(Plane[1]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (fabs(RPXY.first) <= PixelDist && fabs(RPXY.second) <= PixelDist) {
              ++HC[date][Channel].NFiducialAndHit[1];
            } 
          }
        }
      }

      // Test of plane 0
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[2].IsFiducial(Channel, 0, Alignment, EventTE.PixelMask()) && Tracks[2].NHits() == 2 && Tracks[2].fTVY/Tracks[2].fTVZ < slope_y_high && Tracks[2].fTVY/Tracks[2].fTVZ > slope_y_low && Tracks[2].fTVX/Tracks[2].fTVZ < slope_x_high && Tracks[2].fTVX/Tracks[2].fTVZ > slope_x_low) {
          ++HC[date][Channel].NFiducial[0];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 0);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[2].TX( CP->LZ ), Tracks[2].TY( CP->LZ ), Channel, 0);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
	  float cluster_charge = 0;
	  if(Plane[0]->NClusters()) cluster_charge=Plane[0]->Cluster(0)->Charge();
          if (Plane[0]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[2].LResiduals( *(Plane[0]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (fabs(RPXY.first) <= PixelDist && fabs(RPXY.second) <= PixelDist) {
              ++HC[date][Channel].NFiducialAndHit[0];
            } 
          }
        }
      }
    }
  }


//  ofstream file;
//  file.open("plots/roc_eff.csv");//needs additional formatting
//  file << "Time,Channel,ROC,NFiducial,NFiducialAndHit,Efficiency\n";


  //Write CSV file
  for (std::map<uint32_t, std::map<int, HitCounter> >::iterator timestamp_iter = HC.begin(); timestamp_iter != HC.end(); ++timestamp_iter) {
    for (std::map<int, HitCounter>::iterator channel_iter = timestamp_iter->second.begin(); channel_iter != timestamp_iter->second.end(); ++channel_iter) {
      //printf("Efficiencies for Channel %2i:\n", timestamp_iter->first);
      for (int roc_number = 0; roc_number != 3; ++roc_number) {
        std::cout << timestamp_iter->first << "," << channel_iter->first << "," << roc_number << "," << channel_iter->second.NFiducial[roc_number] << "," << channel_iter->second.NFiducialAndHit[roc_number] << "," << float(channel_iter->second.NFiducialAndHit[roc_number]) / float(channel_iter->second.NFiducial[roc_number]) << "\n";
      } 
    }
  }

//  file.close();
  return 0;
}

int main (int argc, char* argv[])
{
  if (argc < 5 || argc > 6) {
    std::cerr << "Usage: " << argv[0] << " DataFile.dat GainCal.dat AlignmentFile.dat TrackDistribution.txt [TimestampFile.dat]" << std::endl;
    return 1;
  }

  const std::string DataFileName = argv[1];
  const std::string GainCalFileName = argv[2];
  const std::string AlignmentFileName = argv[3];
  const std::string TrackDistributionFileName = argv[4];
  const std::string TimestampFileName = (argc == 6) ? argv[5] : "";

  TrackingEfficiency(DataFileName, GainCalFileName, AlignmentFileName);
  EOFAnalyzer(DataFileName, GainCalFileName, AlignmentFileName, TrackDistributionFileName, TimestampFileName);

  return 0;
}

