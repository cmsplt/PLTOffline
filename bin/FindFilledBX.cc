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

  int hits[nChannels][NBX];
  for (int i=0; i<nChannels; ++i) {
    for (int j=0; j<NBX; ++j) {
      hits[i][j] = 0;
    }
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
      if (channelNumber[Plane->Channel()] == -1) {
	std::cout << "Bad channel number found: " << Plane->Channel() << std::endl;
      } else {
	hits[channelNumber[Plane->Channel()]][Event.BX()] += Plane->NHits();
      }
    }
    if (ientry > 500000) break;
  }
  
  // Process the results.
  int maxValTele = 0;
  int maxValSide = 0; // these will have different ranges so we need to keep track separately
  int hitsBySide[2][NBX];
  const char *sideName[2] = {"-z", "+z"};
  
  // Find maximum and fill hits by side arrays.
  for (int iBX=0; iBX<NBX; ++iBX) {
    for (int iSide=0; iSide<2; ++iSide) {
      hitsBySide[iSide][iBX] = 0;
    }

    for (int iCh=0; iCh<nChannels; ++iCh) {
      hitsBySide[iCh/8][iBX] += hits[iCh][iBX];
      if (hits[iCh][iBX] > maxValTele) maxValTele = hits[iCh][iBX];
    }

    for (int iSide=0; iSide<2; ++iSide) {
      if (hitsBySide[iSide][iBX] > maxValSide) maxValSide = hitsBySide[iSide][iBX];
    }
  }
  
  std::cout << "*** All BX numbers use CMS/LHC convention (starting at 1) ***" << std::endl;
  std::cout << "=== BY SIDE ===" << std::endl;
  for (int i=0; i<2; ++i) {
    std::cout << sideName[i] << ": ";
    for (int j=0; j<NBX; ++j) {
      if (hitsBySide[i][j] > fillThresh * (float)maxValSide)
	std::cout << j+1 << " ";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl << "=== BY TELESCOPE (readout channel number) ===" << std::endl;
  for (int i=0; i<nChannels; ++i) {
    std::cout << "[" << i << "]: ";
    for (int j=0; j<NBX; ++j) {
      if (hits[i][j] > fillThresh * (float)maxValTele)
	std::cout << j+1 << " ";
    }
    std::cout << std::endl;
  }

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
