////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu Mar  8 18:26:18 UTC 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"
#include "PLTU.h"





// FUNCTION DEFINITIONS HERE
int MakeTracks (std::string const, std::string const, std::string const);









int MakeTracks (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_Diamond;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_m1_m1;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);


  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      static int ievent = 0;
      if (Telescope->NTracks() > 0 && ievent < 20) {
        Telescope->DrawTracksAndHits( TString::Format("plots/Tracks_Ch%i_Ev%i.gif", Telescope->Channel(), ievent++).Data() );
      }


    }


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

  MakeTracks(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
