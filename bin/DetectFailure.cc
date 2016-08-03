////////////////////////////////////////////////////////////////////
//
// DetectFailure
// Daniel Gift
// June 24, 2016
//
// Adapted from PlotActiveBunches
//
// A script that determines if there is a telescope that goes dead 
// for a signifiant section of a run. Assumes that channels 1, 22,
//  and 23 are not working.
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <numeric>

#include "PLTEvent.h"
#include "PLTU.h"
#include "PLTPlane.h"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE
const Int_t nChannels = 24; //Shouldn't need to change this.
const Int_t consecutiveZeros = 10000; //Number of events with no hits before declared dead

// Change this varible if other telescopes are revived or die.
const Int_t nValidChannels = 16;
const Int_t validChannels[nValidChannels] = {1, 2, 4, 5, 7, 8, 10, 11, 13, 14, 16, 17, 19, 20, 22, 23};


// CODE BELOW

int DetectFailure (std::string const DataFileName) {
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  
  // Arrays to story variables
  std::map<Int_t, Int_t> startEvent;
  std::map<Int_t, std::string> startTime;
  std::map<Int_t, Int_t> endEvent;
  std::map<Int_t, std::string> endTime;
  Int_t zerosInARow[nChannels] = {0};
  Int_t newHits[nChannels] = {0};
  
  Int_t offset = 1;
  Int_t lastEvent = 0;
  std::string lastTime = "";
  Int_t blanks = 0;

  // Loop over all events in file
  for (Int_t ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 200000 == 0) {
      std::cout << "Processing event: " << ientry << " at " << Event.ReadableTime() << std::endl;
    }
    
    // Stop a fill from going over the end of stable beams. Notes when there 
    // has been an extended period of time with no hits in all channels
    if (Event.NPlanes() == (size_t)0){
      ++blanks;
      if (blanks > 1000)
	break;
    }
    else 
      blanks = 0;
    
    // Loop through all channels and note how many hits in each
    for (size_t ip = 0; ip < Event.NPlanes(); ++ip) {
      PLTPlane* Plane = Event.Plane(ip);
      Int_t ch = Plane->Channel();
      newHits[ch] = (Int_t)Plane->NHits();
    }
    
    // Keep track of how many times in a row a channel has had 0 hits 
    for (Int_t index = 0; index < nValidChannels; ++index) {
      Int_t actCh = validChannels[index];
      // Increment the number of 0's in a row seen 
      if (newHits[actCh] == 0) {
     	++zerosInARow[actCh];
      }
      
      // Or reset that count to 0
      else {
     	if (zerosInARow[actCh] > consecutiveZeros) {
     	  std::cout << "Channel " << actCh << " has recovered at event " << ientry << " at time " << Event.ReadableTime() << std::endl;
	  
	  // Account for the possibility of multiple failures in the same channel
	  // Store the errors as the number plus mutiples of 100
	  for (Int_t possibleOffset = 0; possibleOffset < offset; ++possibleOffset) {
    	    if (endEvent[100 * possibleOffset + actCh] == -1) {
	      endEvent[100 * possibleOffset +actCh] = ientry;
    	      endTime[100 * possibleOffset +actCh] = Event.ReadableTime();
    	    }
	  }
	}

	zerosInARow[actCh] = 0;
      }

      // If a channel has had 0 hits for long enough, declare it dead      
      if (zerosInARow[actCh] == consecutiveZeros) {
	int nsec = Event.Time()/1000;
    	int hr = nsec/3600;
	int min = (nsec-(hr*3600))/60;
	
	std::stringstream buf;
	buf << std::setfill('0') << std::setw(2) << hr << ":" << std::setw(2) << min;
	std::string failureTime = buf.str();
	std::cout << "Failure in Channel "  << actCh << " at event " << 
	  ientry - consecutiveZeros + 1 << " at approximately  " << failureTime << std::endl;
	// If there's already a failure with this channel, the index is the offset plus the channel
	if (startEvent.count(actCh)) {
	  startEvent.insert(std::pair<Int_t, Int_t>(offset*100 + actCh, ientry - consecutiveZeros + 1));
	  startTime.insert(std::pair<Int_t, std::string> (offset * 100 + actCh, failureTime));
	  endEvent.insert(std::pair<Int_t, Int_t> (offset * 100 + actCh, -1));
	  endTime.insert(std::pair<Int_t, std::string> (offset * 100 + actCh, "End"));
	  ++offset;
	}
	// If there isn't yet a failure in this channel, do it normally
	else {
	  startEvent.insert(std::pair<Int_t, Int_t>(actCh, ientry - consecutiveZeros + 1));
	  startTime.insert(std::pair<Int_t, std::string> (actCh, failureTime));
	  endEvent.insert(std::pair<Int_t, Int_t> (actCh, -1));
    	  endTime.insert(std::pair<Int_t, std::string> (actCh, "End"));
	}
      }
    }
    // Keep track of the last event for reference. Mod 100 for speed (every event is too slow)
    if (ientry%100 ==0) {
      lastEvent = ientry;
      lastTime = Event.ReadableTime();
    }
  }

  std::cout << "Finished; last event is " << lastEvent << " at about time " << lastTime << std::endl;

  // Print out the failures
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
  
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }
  
  std::string const DataFileName = argv[1];
  DetectFailure(DataFileName);
  
  return 0;
}
