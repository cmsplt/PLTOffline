////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri Apr  6 15:56:13 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"



int PrintBXTracks (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_Diamond;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

  std::map<int, int> BXMap;
  std::map<int, int> BXMapCh;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    if (ientry > 10000) break;

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);
      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        ++BXMap[Event.BX()];
        ++BXMapCh[Event.BX()*100+Telescope->Channel()];
      }
    }
  }

  for (std::map<int, int>::iterator it = BXMap.begin(); it != BXMap.end(); ++it) {
    printf("BX: %5i   N: %15i\n", it->first, it->second);
  }

  for (std::map<int, int>::iterator it = BXMapCh.begin(); it != BXMapCh.end(); ++it) {
    printf("BX: %5i  Ch: %2i  N: %15i\n", it->first / 100, it->first % 100, it->second);
  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];

  PrintBXTracks(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
