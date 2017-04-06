////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Jul  4 19:20:41 CEST 2011
//
////////////////////////////////////////////////////////////////////
//Edited by Thoth Gunter

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include "PLTEvent.h"
#include "PLTU.h"



class HitCounter
{
  public:
    HitCounter () {
      NFiducial[0] = 0;
      NFiducial[1] = 0;
      NFiducial[2] = 0;
      NFiducialAndHit[0] = 0;
      NFiducialAndHit[1] = 0;
      NFiducialAndHit[2] = 0;
    }
    ~HitCounter () {}

    int NFiducial[3];
    int NFiducialAndHit[3];
};



// FUNCTION DEFINITIONS HERE
int TrackingEfficiency (std::string const, std::string const, std::string const);







// CODE BELOW

int TrackingEfficiency (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_Diamond;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_m5_m5;//<-Original
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Map for tracking efficiency
  std::map<uint32_t, std::map<int, HitCounter> > HC;
  
  float const PixelDist = 5;//<-Original
  float slope_x_low = 0.0 - 0.01;
  float slope_y_low = 0.027 - 0.01;
  float slope_x_high = 0.0 + 0.01;
  float slope_y_high = 0.027 + 0.01;

  uint32_t date = 0;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
//    if (ientry > 50000) break;
    if (ientry == 0){
      date = Event.Time();
    }

    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event.Telescope(it);

      int    const Channel = Telescope->Channel();
      size_t const NPlanes = Telescope->NPlanes();

      // make them clean events
      if (Telescope->NHitPlanes() < 2 || Telescope->NHitPlanes() != Telescope->NClusters()) {
        continue;
      }



      PLTPlane* Plane[3] = {0x0, 0x0, 0x0};
      for (size_t ip = 0; ip != NPlanes; ++ip) {
        Plane[ Telescope->Plane(ip)->ROC() ] = Telescope->Plane(ip);
      }

      // To construct 4 tracks.. one testing each plane.. and one using all planes if it be.
      PLTTrack Tracks[4];

      // If it has all 3 planes that'll be number 0
      if (Plane[0]->NClusters() && Plane[1]->NClusters() && Plane[2]->NClusters()) {
        Tracks[0].AddCluster(Plane[0]->Cluster(0));
        Tracks[0].AddCluster(Plane[1]->Cluster(0));
        Tracks[0].AddCluster(Plane[2]->Cluster(0));
        Tracks[0].MakeTrack(Alignment);
      }

      // 2-plane tracks
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

      // Test of plane 2
      if (HC[date][Channel].NFiducial[1] > 1*pow(10, 4)) date = Event.Time();
      if (Plane[0]->NClusters() && Plane[1]->NClusters()) {
        if (Tracks[1].IsFiducial(Channel, 2, Alignment, FidRegionTrack) && Tracks[1].NHits() == 2 && Tracks[1].fTVY/Tracks[1].fTVZ < slope_y_high && Tracks[1].fTVY/Tracks[1].fTVZ > slope_y_low && Tracks[1].fTVX/Tracks[1].fTVZ < slope_x_high && Tracks[1].fTVX/Tracks[1].fTVZ > slope_x_low ) {
          ++HC[date][Channel].NFiducial[2];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 2);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[1].TX( CP->LZ ), Tracks[1].TY( CP->LZ ), Channel, 2);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
	  float cluster_charge = 0;
	  if(Plane[2]->NClusters()) cluster_charge=Plane[2]->Cluster(0)->Charge();
          //hEffMapPulseHeightD[Channel * 10 + 2]->Fill(cluster_charge);
          if (Plane[2]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[1].LResiduals( *(Plane[2]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            //printf("ResXY: %15.9f   RPXY: %15.9f\n", ResXY.first, RPXY.first);
            if (fabs(RPXY.first) <= PixelDist && fabs(RPXY.second) <= PixelDist) {
              ++HC[date][Channel].NFiducialAndHit[2];
            } 
          }
        }
      }

      // Test of plane 1
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[2].IsFiducial(Channel, 1, Alignment, FidRegionTrack) && Tracks[2].NHits() == 2 && Tracks[2].fTVY/Tracks[2].fTVZ < slope_y_high && Tracks[2].fTVY/Tracks[2].fTVZ > slope_y_low && Tracks[2].fTVX/Tracks[2].fTVZ < slope_x_high && Tracks[2].fTVX/Tracks[2].fTVZ > slope_x_low) {
          ++HC[date][Channel].NFiducial[1];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 1);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[2].TX( CP->LZ ), Tracks[2].TY( CP->LZ ), Channel, 1);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
	  float cluster_charge = 0;
	  if(Plane[1]->NClusters()) cluster_charge=Plane[1]->Cluster(0)->Charge();
          //hEffMapPulseHeightD[Channel * 10 + 1]->Fill(cluster_charge);
          if (Plane[1]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[2].LResiduals( *(Plane[1]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (fabs(RPXY.first) <= PixelDist && fabs(RPXY.second) <= PixelDist) {
              ++HC[date][Channel].NFiducialAndHit[1];
            } //else hMapPulseHeights[Channel * 10 + 1]->Fill(0);
          }
        }
      }

      // Test of plane 0
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[3].IsFiducial(Channel, 0, Alignment, FidRegionTrack) && Tracks[3].NHits() == 2 && Tracks[3].fTVY/Tracks[3].fTVZ < slope_y_high && Tracks[3].fTVY/Tracks[3].fTVZ > slope_y_low && Tracks[3].fTVX/Tracks[3].fTVZ < slope_x_high && Tracks[3].fTVX/Tracks[3].fTVZ > slope_x_low) {
          ++HC[date][Channel].NFiducial[0];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 0);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[3].TX( CP->LZ ), Tracks[3].TY( CP->LZ ), Channel, 0);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
	  float cluster_charge = 0;
	  if(Plane[0]->NClusters()) cluster_charge=Plane[0]->Cluster(0)->Charge();
          //hEffMapPulseHeightD[Channel * 10 + 0]->Fill(cluster_charge);
          if (Plane[0]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[3].LResiduals( *(Plane[0]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (fabs(RPXY.first) <= PixelDist && fabs(RPXY.second) <= PixelDist) {
              ++HC[date][Channel].NFiducialAndHit[0];
            } //else hMapPulseHeights[Channel * 10 + 0]->Fill(0);
          }
        }
      }

      //printf("%9i %9i %9i\n", HC[Channel].NFiducialAndHit[0], HC[Channel].NFiducialAndHit[1], HC[Channel].NFiducialAndHit[2]);

    }
  }


  ofstream file;
  file.open("plots/roc_eff.csv");//needs additional formatting
  file << "Time,Channel,ROC,NFiducial,NFiducialAndHit,Efficiency\n";

  for (std::map<uint32_t, std::map<int, HitCounter> >::iterator timestamp_iter = HC.begin(); timestamp_iter != HC.end(); ++timestamp_iter) {
    for (std::map<int, HitCounter>::iterator channel_iter = timestamp_iter->second.begin(); channel_iter != timestamp_iter->second.end(); ++channel_iter) {
      printf("Efficiencies for Channel %2i:\n", timestamp_iter->first);
      for (int roc_number = 0; roc_number != 3; ++roc_number) {
        /*printf("ROC %1i  NFiducial: %10i  NFiducialAndHit: %10i  Efficiency: %12.9f\n",
               i, it->second.NFiducial[i],
               it->second.NFiducialAndHit[i],
               float(it->second.NFiducialAndHit[i]) / float(it->second.NFiducial[i]) );*/
        file << timestamp_iter->first << "," << channel_iter->first << "," << roc_number << "," << channel_iter->second.NFiducial[roc_number] << "," << channel_iter->second.NFiducialAndHit[roc_number] << "," << float(channel_iter->second.NFiducialAndHit[roc_number]) / float(channel_iter->second.NFiducial[roc_number]) << "\n";
      } 
    }
  }

  file.close();
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
