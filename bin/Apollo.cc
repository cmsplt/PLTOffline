////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Jun 26 16:02:25 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"







int Verbose (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  PLTU::SetStyle();
  gStyle->SetOptStat(0);

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits    = PLTPlane::kFiducialRegion_All;
  PLTPlane::FiducialRegion FidRegionCluster = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, FidRegionCluster);


  int ientry = 0;
  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 1000 == 0) {
      printf("Processing event: %15i\n", ientry);
    }


    for (size_t itelescope = 0; itelescope != Event.NTelescopes(); ++itelescope) {
      PLTTelescope* Telescope = Event.Telescope(itelescope);

      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        PLTTrack* Track = Telescope->Track(itrack);
      }




    }
  }





  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [GainCalFileName] [AlignmentFileName]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << GainCalFileName << std::endl;

  Verbose(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
