////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Jul  4 19:20:41 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"



// FUNCTION DEFINITIONS HERE
int TrackingEfficiency (std::string const, std::string const, std::string const);







// CODE BELOW


int TrackingEfficiency (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Number of tracks that are fiducial to [ROC] and number of times hit =)
  int NFiducial[3] = {0, 0, 0};
  int NFiducialAndHit[3] = {0, 0, 0};

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {

    if (ientry == 100000) break;

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event.Telescope(it);

      int    const Channel = Telescope->Channel();
      size_t const NPlanes = Telescope->NPlanes();

      // Only consider telescopes with 2 or more planes
      if (NPlanes < 2) {
        continue;
      }

      // make them clean events
      if (Telescope->NClusters() != NPlanes) {
        continue;
      }

      PLTPlane* Plane[3] = {0x0, 0x0, 0x0};
      for (size_t ip = 0; ip != NPlanes; ++ip) {
        Plane[ Telescope->Plane(ip)->ROC() ] = Telescope->Plane(ip);
      }

      PLTTrack Tracks[4];
      if (Plane[0]->NClusters() && Plane[1]->NClusters() && Plane[2]->NClusters()) {
        Tracks[0].AddCluster(Plane[0]->Cluster(0));
        Tracks[0].AddCluster(Plane[1]->Cluster(0));
        Tracks[0].AddCluster(Plane[2]->Cluster(0));
        Tracks[0].MakeTrack(Alignment);
      }
      if (Plane[0]->NClusters() && Plane[1]->NClusters()) {
        Tracks[1].AddCluster(Plane[0]->Cluster(0));
        Tracks[1].AddCluster(Plane[1]->Cluster(0));
        Tracks[1].MakeTrack(Alignment);
      }
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        Tracks[2].AddCluster(Plane[0]->Cluster(0));
        Tracks[2].AddCluster(Plane[2]->Cluster(0));
        Tracks[2].MakeTrack(Alignment);
      }
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        Tracks[3].AddCluster(Plane[1]->Cluster(0));
        Tracks[3].AddCluster(Plane[2]->Cluster(0));
        Tracks[3].MakeTrack(Alignment);
      }

      if (Plane[0]->NClusters() && Plane[1]->NClusters()) {
        if (Tracks[1].IsFiducial(Channel, 2, Alignment)) {
          ++NFiducial[2];
          if (Plane[2]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[1].LResiduals( *(Plane[2]->Cluster(0)), Alignment );
            std::cout << "MYRES: " << ResXY.first << std::endl;
            printf("TrackRESXX %f %f\n", Tracks[1].LResidualX(2),  Tracks[1].LResidualY(2));
            if (abs(ResXY.first) <= 5 && abs(ResXY.second) <= 5) {
              ++NFiducialAndHit[2];
            }
          }
        } else {
        }
      }

    }
  }


  printf("Efficiencies:\n");
  for (int i = 0; i != 3; ++i) {
    printf("ROC %1i  NFiducial: %10i  NFiducialAndHit: %10i  Efficiency: %12.9f\n", i, NFiducial[i], NFiducialAndHit[i], float(NFiducialAndHit[i]) / float(NFiducial[i]) );
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

  TrackingEfficiency(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
