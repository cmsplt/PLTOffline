////////////////////////////////////////////////////////////////////
//
// FindLeadingBX
// Paul Lujan
// June 10, 2020
//
// This is an adaptation of FindFilledBX to do specifically the
// leading bunch crossings. Also much simpler because it doesn't
// need to separate data by channel
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <numeric>
#include <algorithm>

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
// BXes with a total number of hits > fillThresh*(largest number of hits)
// will be considered "filled"
const float fillThresh = 0.05;

const int nFEDChannels = 36;
const int nChannels = 16;

// Translation of pixel FED channel number to readout channel number.
const int channelNumber[36] = {-1, 0, 1, -1, 2, 3, -1, 4, 5, -1, 6, 7,  // 0-11
			       -1, 8, 9, -1, 10, 11, -1, 12, 13, -1, 14, 15, // 12-23
			       -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

// CODE BELOW

int FindFilledBX(const std::string DataFileName) {
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  int hits[NBX];
  for (int i=0; i<NBX; ++i) {
    hits[i] = 0;
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

    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {
      PLTPlane* Plane = Event.Plane(ip);
      hits[Event.BX()] += Plane->NHits();
    }
    if (ientry > 500000) break;
  }
  
  // Process the results.
  int maxVal = 0;
  
  // Find maximum and count number of filled bunches.
  for (int i=0; i<NBX; ++i) {
    if (hits[i] > maxVal)
      maxVal = hits[i];
  }
  int totFilled = 0;
  int totLeading = 0;
  for (int i=0; i<NBX; ++i) {
    if (hits[i] > fillThresh*(float)maxVal) {
      totFilled++;
      if (i>0 && hits[i-1] < fillThresh*(float)maxVal)
	totLeading++;
    }
  }
      
  std::cout << "Total number of filled bunches: " << totFilled << std::endl;
  std::cout << "*** All BX numbers use INTERNAL convention (starting at 0) ***" << std::endl;
  for (int i=0; i<NBX; ++i) {
    if (hits[i] > fillThresh*(float)maxVal)
      std::cout << i << " ";
  }
  std::cout << std::endl;

  std::cout << "Total number of LEADING bunches: " << totLeading << std::endl;
  for (int i=0; i<NBX; ++i) {
    if (i > 0 && hits[i] > fillThresh*(float)maxVal && hits[i-1] < fillThresh*(float)maxVal)
      std::cout << i << ",";
  }
  std::cout << std::endl;

  std::vector<int> t1 = {5, 13, 15};
  std::cout << "Test 1: " << (std::find(t1.begin(), t1.end(), 12) != t1.end()) << std::endl;
  std::cout << "Test 2: " << (std::find(t1.begin(), t1.end(), 13) != t1.end()) << std::endl;
  std::cout << "Test 3: " << (std::find(t1.begin(), t1.end(), 15) != t1.end()) << std::endl;
  std::cout << "Test 4: " << (std::find(t1.begin(), t1.end(), 17) != t1.end()) << std::endl;
  return 0;
}

int main (int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  FindFilledBX(DataFileName);

  return 0;
}
