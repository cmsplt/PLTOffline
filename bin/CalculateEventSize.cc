////////////////////////////////////////////////////////////////////
//
// CalculateEventSize
// Paul Lujan
// May 11, 2016
//
// A very simple script which calculates the average event size per
// event.
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <numeric>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TH1F.h"
#include "TFile.h"
#include "TCanvas.h"

// FUNCTION DEFINITIONS HERE

int FindFilledBX(const std::string DataFileName);

// CONSTANTS HERE

// total number of bunch crossings
const int NBX = 3564;

const int nFEDChannels = 36;
const int nChannels = 16;

// Translation of pixel FED channel number to readout channel number.
const int channelNumber[36] = {-1, 0, 1, -1, 2, 3, -1, 4, 5, -1, 6, 7,  // 0-11
			       -1, 8, 9, -1, 10, 11, -1, 12, 13, -1, 14, 15, // 12-23
			       -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

const int nevents = 100000;

// CODE BELOW

int CalculateEventSize(const std::string DataFileName) {
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  int nhits[nChannels];
  int ntothits = 0;

  for (int i=0; i<nChannels; ++i) {
    nhits[i] = 0;
  }

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 50000 == 0) {
      int nsec = Event.Time()/1000;
      int hr = nsec/3600;
      int min = (nsec-(hr*3600))/60;
      int sec = nsec % 60;
      std::cout << "Processing event: " << ientry << " at " << std::setfill('0') << std::setw(2)
		<< hr << ":" << std::setw(2) << min << ":" << std::setw(2) << sec << "."
		<< std::setw(3) << Event.Time()%1000 << std::endl;
    }
    if (ientry > nevents) break;

    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {
      PLTPlane* Plane = Event.Plane(ip);
      if (channelNumber[Plane->Channel()] == -1) {
	std::cout << "Bad channel number found: " << Plane->Channel() << std::endl;
      } else {
	nhits[channelNumber[Plane->Channel()]] += Plane->NHits();
	ntothits += Plane->NHits();
      }
    }
  }

  for (int i=0; i<nChannels; ++i) {
    std::cout << "Average for channel " << i << ": " << (float)nhits[i]/nevents << std::endl;
  }
  std::cout << "Average for all channels: " << (float)ntothits/(14*nevents) << std::endl;
  std::cout << "Average total event size: " << (float)ntothits/nevents << std::endl;

  return 0;
}


int main (int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  CalculateEventSize(DataFileName);

  return 0;
}
