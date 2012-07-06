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
  //PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName, true);
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits    = PLTPlane::kFiducialRegion_All;
  PLTPlane::FiducialRegion FidRegionCluster = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, FidRegionCluster);
  //Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  //Event.ReadPixelMask("PixelMask_miniplt.conv.dat");

  int ientry = 0;
  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    printf("Processing event: %15i   NTelescopes: %3i\n", ientry, (int) Event.NTelescopes());

    if (ientry >= 20) {
      break;
    }

    for (size_t itelescope = 0; itelescope != Event.NTelescopes(); ++itelescope) {
      PLTTelescope* Telescope = Event.Telescope(itelescope);
      printf("  Telescope: %3i  NTracks: %4i  NPlanes: %2i\n", Telescope->Channel(), (int) Telescope->NTracks(), (int) Telescope->NPlanes());

      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        PLTTrack* Track = Telescope->Track(itrack);
        printf("    Track: %4i  SlopeY: %12.9f  SlopeY: %12.9f  D: %15.9f\n", (int) itrack, Track->fTVY/Track->fTVZ, Track->fTVX/Track->fTVZ, Track->D2());
      }


      for (size_t iplane = 0; iplane != Telescope->NPlanes(); ++iplane) {
        PLTPlane* Plane = Telescope->Plane(iplane);
        printf("    Plane id: %3i  NClusters: %4i   NHits: %4i\n", Plane->Channel() * 10 + Plane->ROC(), (int) Plane->NClusters(), (int) Plane->NHits());

        for (size_t icluster = 0; icluster != Plane->NClusters(); ++icluster) {
          PLTCluster* Cluster = Plane->Cluster(icluster);
          std::pair<float, float> Center = Cluster->LCenter();
          printf("      Cluster: %4i  NHits: %3i  Charge: %9.0f  LX: %10.6f  LY: %10.6f\n", (int) icluster, (int) Cluster->NHits(), Cluster->Charge(), Center.first, Center.second);

          for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {
            PLTHit* Hit = Cluster->Hit(ihit);
            printf("        Hit: %4i  Charge: %9.0f  LX: %10.6f  LY: %10.6f   Row: %3i  Column: %3i\n", (int) ihit, Hit->Charge(), Hit->LX(), Hit->LY(), Hit->Row(), Hit->Column());
          }
        }


        for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {
          PLTHit* Hit = Plane->Hit(ihit);
          printf("      Hit: %4i  Charge: %9.0f  LX: %10.6f  LY: %10.6f   Row: %3i  Column: %3i\n", (int) ihit, Hit->Charge(), Hit->LX(), Hit->LY(), Hit->Row(), Hit->Column());
        }
        for (size_t ihit = 0; ihit != Plane->NUnclusteredHits(); ++ihit) {
          PLTHit* Hit = Plane->UnclusteredHit(ihit);
          printf("      Hit Unclustered: %4i  Charge: %9.0f  LX: %10.6f  LY: %10.6f   Row: %3i  Column: %3i\n", (int) ihit, Hit->Charge(), Hit->LX(), Hit->LY(), Hit->Row(), Hit->Column());
        }
      }

      Telescope->DrawTracksAndHits( TString::Format("Event_%09i_Ch%02i.gif", ientry, Telescope->Channel()).Data() );
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
