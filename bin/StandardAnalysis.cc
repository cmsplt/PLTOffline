////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Jun  5 16:41:13 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TH2F.h"
#include "TCanvas.h"


int StandardAnalysis (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  PLTU::SetStyle();
  gStyle->SetOptStat(111111);

  int const HistColors[4] = { 1, 4, 28, 2 };

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_m2_m2;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_m3_m3;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_Seed_3x3, FidRegionHits);

  //Event.ReadPixelMask("MyPixelMaskFile.dat");



  // Loop over all events.  count them with ientry
  int ientry = 0;
  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    // Loop over all telescopes (FED "Channels")
    for (size_t iTelescope = 0; iTelescope != Event.NTelescopes(); ++iTelescope) {
      PLTTelescope* Telescope = Event.Telescope(iTelescope);

      // Loop over all tracks in this telescope
      for (size_t itrack = 0; itrack < Telescope->NTracks(); ++itrack) {
        PLTTrack* Track = Telescope->Track(itrack);

      }


      // Loop over all Planes in a telescope
      for (size_t iPlane = 0; iPlane != Telescope->NPlanes(); ++iPlane) {
        PLTPlane* Plane = Telescope->Plane(iPlane);

      }




    }



  }
  std::cout << "Events read: " << ientry+1 << std::endl;






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

  StandardAnalysis(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
