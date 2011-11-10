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

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_Diamond);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Map for tracking efficiency
  std::map<int, HitCounter> HC;
  std::map<int, TH2F*> hEffMapN;
  std::map<int, TH2F*> hEffMapD;

  int const PixelDist = 20;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
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


      // Make some hists for this telescope
      if (!hEffMapN.count(Channel * 10 + 0)) {

        // Make a numerator and demonitor hist for every roc for this channel
        for (int iroc = 0; iroc != 3; ++iroc) {
          TString Name = TString::Format("EffNumerator_Ch%i_ROC%i", Channel, iroc);
          hEffMapN[Channel * 10 + iroc] = new TH2F(Name, Name, PLTU::NCOL - 1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW - 1, PLTU::FIRSTROW, PLTU::LASTROW);
          Name = TString::Format("EffDenominator%i_ROC%i", Channel, iroc);
          hEffMapD[Channel * 10 + iroc] = new TH2F(Name, Name, PLTU::NCOL - 1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW - 1, PLTU::FIRSTROW, PLTU::LASTROW);
        }
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
          ++HC[Channel].NFiducial[2];

          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 2);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[1].TX( CP->LZ ), Tracks[1].TY( CP->LZ ), Channel, 2);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 2]->Fill(PXY.first, PXY.second);
          if (Plane[2]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[1].LResiduals( *(Plane[2]->Cluster(0)), Alignment );
            if (abs(ResXY.first) <= PixelDist && abs(ResXY.second) <= PixelDist) {
              //hEffMapN[Channel * 10 + 2]->Fill(Plane[2]->Cluster(0)->SeedHit()->Column(), Plane[2]->Cluster(0)->SeedHit()->Row());
              hEffMapN[Channel * 10 + 2]->Fill(PXY.first, PXY.second);
              ++HC[Channel].NFiducialAndHit[2];
            }
          }
        }
      }
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[2].IsFiducial(Channel, 1, Alignment)) {
          ++HC[Channel].NFiducial[1];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 1);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[2].TX( CP->LZ ), Tracks[2].TY( CP->LZ ), Channel, 1);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 1]->Fill(PXY.first, PXY.second);
          if (Plane[1]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[2].LResiduals( *(Plane[1]->Cluster(0)), Alignment );
            if (abs(ResXY.first) <= PixelDist && abs(ResXY.second) <= PixelDist) {
              hEffMapN[Channel * 10 + 1]->Fill(PXY.first, PXY.second);
              ++HC[Channel].NFiducialAndHit[1];
            }
          }
        }
      }
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[3].IsFiducial(Channel, 0, Alignment)) {
          ++HC[Channel].NFiducial[0];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 0);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[3].TX( CP->LZ ), Tracks[3].TY( CP->LZ ), Channel, 0);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 0]->Fill(PXY.first, PXY.second);
          if (Plane[0]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[3].LResiduals( *(Plane[0]->Cluster(0)), Alignment );
            if (abs(ResXY.first) <= PixelDist && abs(ResXY.second) <= PixelDist) {
              hEffMapN[Channel * 10 + 0]->Fill(PXY.first, PXY.second);
              ++HC[Channel].NFiducialAndHit[0];
            }
          }
        }
      }

      //printf("%9i %9i %9i\n", HC[Channel].NFiducialAndHit[0], HC[Channel].NFiducialAndHit[1], HC[Channel].NFiducialAndHit[2]);

    }
  }


  // Save some efficiency maps
  for (std::map<int, TH2F*>::iterator it = hEffMapD.begin(); it != hEffMapD.end(); ++it) {
    int const id = it->first;
    int const Channel = id / 10;
    int const ROC     = id % 10;

    TString const Name = TString::Format("TrackingEfficiencyMap_Ch%i_ROC%i", Channel, ROC);
    TCanvas Can(Name, Name);
    Can.cd();

    // Rebin Denom and Numer
    //hEffMapN[id]->Rebin2D();
    //it->second->Rebin2D();

    hEffMapN[id]->SetTitle(Name);
    hEffMapN[id]->SetMinimum(0);
    hEffMapN[id]->SetMaximum(2);

    hEffMapN[id]->Divide(it->second);
    hEffMapN[id]->Draw("colz");
    Can.SaveAs(Name + ".gif");

  }


  for (std::map<int, HitCounter>::iterator it = HC.begin(); it != HC.end(); ++it) {
    printf("Efficiencies for Channel %2i:\n", it->first);
    for (int i = 0; i != 3; ++i) {
      printf("ROC %1i  NFiducial: %10i  NFiducialAndHit: %10i  Efficiency: %12.9f\n",
             i, it->second.NFiducial[i],
             it->second.NFiducialAndHit[i],
             float(it->second.NFiducialAndHit[i]) / float(it->second.NFiducial[i]) );
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

  TrackingEfficiency(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
