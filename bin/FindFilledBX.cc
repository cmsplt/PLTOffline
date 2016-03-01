////////////////////////////////////////////////////////////////////
//
// FindFilledBX
// Paul Lujan
// February 12, 2016
//
// This script looks at the BX in the Slink header data and sees
// which ones are filled so that we can tell if this data is accurate
// or if it needs to be adjusted in any way. The +z and -z sides are
// treated separately to account for any possible differences between
// the two sides.
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
// BXes with a total number of hits > fillThresh*(largest number of hits)
// will be considered "filled"
const float fillThresh = 0.05;

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

  int hitsNegativeSide[NBX];
  int hitsPositiveSide[NBX];
  for (int i=0; i<NBX; ++i) {
    hitsNegativeSide[i] = 0;
    hitsPositiveSide[i] = 0;
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
      if (Plane->Channel() < 12)
	hitsNegativeSide[Event.BX()] += Plane->NHits();
      else
	hitsPositiveSide[Event.BX()] += Plane->NHits();
    }
    //if (ientry > 500000) break;
  }
  // Process the results.
  int maxVal = 0;
  for (int i=0; i<NBX; ++i) {
    if (hitsNegativeSide[i] > maxVal)
      maxVal = hitsNegativeSide[i];
    if (hitsPositiveSide[i] > maxVal)
      maxVal = hitsPositiveSide[i];
  }
  
  std::cout << "-z: ";
  for (int i=0; i<NBX; ++i) {
    if (hitsNegativeSide[i] > fillThresh * (float)maxVal)
      std::cout << i << " ";
  }
  std::cout << std::endl << "+z: ";
  for (int i=0; i<NBX; ++i) {
    if (hitsPositiveSide[i] > fillThresh * (float)maxVal)
      std::cout << i << " ";
  }
  std::cout << std::endl;
  
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
