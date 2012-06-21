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
TH1F* FidHistFrom2D (TH2F* hIN, TString const NewName, int const NBins, PLTPlane::FiducialRegion FidRegion)
{
  // This function returns a TH1F* and YOU are then the owner of
  // that memory.  please delete it when you are done!!!

  int const NBinsX = hIN->GetNbinsX();
  int const NBinsY = hIN->GetNbinsY();
  float const ZMin = 0;//hIN->GetMinimum();
  float const ZMax = hIN->GetMaximum() * (1.0 + 1.0 / (float) NBins);
  std::cout << hIN->GetMaximum() << "  " << ZMax << std::endl;
  int const MyNBins = NBins + 1;

  TString const hNAME = NewName == "" ? TString(hIN->GetName()) + "_1DZFid" : NewName;

  TH1F* h;
  h = new TH1F(hNAME, hNAME, MyNBins, ZMin, ZMax);
  h->SetXTitle("Number of Hits");
  h->SetYTitle("Number of Pixels");
  h->GetXaxis()->CenterTitle();
  h->GetYaxis()->CenterTitle();
  h->SetTitleOffset(1.4, "y");
  h->SetFillColor(40);

  for (int ix = 1; ix <= NBinsX; ++ix) {
    for (int iy = 1; iy <= NBinsY; ++iy) {
      int const px = hIN->GetXaxis()->GetBinLowEdge(ix);
      int const py = hIN->GetYaxis()->GetBinLowEdge(iy);
      if (PLTPlane::IsFiducial(FidRegion, px, py)) {
        if (hIN->GetBinContent(ix, iy) > ZMax) {
          h->Fill(ZMax - hIN->GetMaximum() / (float) NBins);
        } else {
          h->Fill( hIN->GetBinContent(ix, iy) );
        }
      }
    }
  }

  return h;
}


int TrackingEfficiency (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_Diamond;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_m2_m2;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Map for tracking efficiency
  std::map<int, HitCounter> HC;
  std::map<int, TH2F*> hEffMapN;
  std::map<int, TH2F*> hEffMapD;

  float const PixelDist = 3;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    if (ientry == 300000) break;

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
          hEffMapN[Channel * 10 + iroc] = new TH2F(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
          Name = TString::Format("EffDenominator%i_ROC%i", Channel, iroc);
          hEffMapD[Channel * 10 + iroc] = new TH2F(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
        }
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
      if (Plane[0]->NClusters() && Plane[1]->NClusters()) {
        if (Tracks[1].IsFiducial(Channel, 2, Alignment, FidRegionTrack) && Tracks[1].NHits() == 2) {
          ++HC[Channel].NFiducial[2];

          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 2);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[1].TX( CP->LZ ), Tracks[1].TY( CP->LZ ), Channel, 2);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 2]->Fill(PXY.first, PXY.second);
          if (Plane[2]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[1].LResiduals( *(Plane[2]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            //printf("ResXY: %15.9f   RPXY: %15.9f\n", ResXY.first, RPXY.first);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
              //hEffMapN[Channel * 10 + 2]->Fill(Plane[2]->Cluster(0)->SeedHit()->Column(), Plane[2]->Cluster(0)->SeedHit()->Row());
              hEffMapN[Channel * 10 + 2]->Fill(PXY.first, PXY.second);
              ++HC[Channel].NFiducialAndHit[2];
            } else {
              static int ievent = 0;
              if (ievent < 20) {
                Telescope->DrawTracksAndHits( TString::Format("Jeebus_%04i.gif", ievent++).Data());
              }
            }
          }
        }
      }

      // Test of plane 1
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[2].IsFiducial(Channel, 1, Alignment, FidRegionTrack) && Tracks[2].NHits() == 2) {
          ++HC[Channel].NFiducial[1];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 1);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[2].TX( CP->LZ ), Tracks[2].TY( CP->LZ ), Channel, 1);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 1]->Fill(PXY.first, PXY.second);
          if (Plane[1]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[2].LResiduals( *(Plane[1]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
              hEffMapN[Channel * 10 + 1]->Fill(PXY.first, PXY.second);
              ++HC[Channel].NFiducialAndHit[1];
            }
          }
        }
      }

      // Test of plane 0
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[3].IsFiducial(Channel, 0, Alignment, FidRegionTrack) && Tracks[3].NHits() == 2) {
          ++HC[Channel].NFiducial[0];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 0);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[3].TX( CP->LZ ), Tracks[3].TY( CP->LZ ), Channel, 0);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 0]->Fill(PXY.first, PXY.second);
          if (Plane[0]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[3].LResiduals( *(Plane[0]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
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
    TCanvas Can(Name, Name, 200, 600);
    Can.Divide(1, 3);


    Can.cd(1);
    hEffMapN[id]->SetTitle(Name);
    hEffMapN[id]->Divide(it->second);
    hEffMapN[id]->SetMinimum(0.3);
    hEffMapN[id]->SetMaximum(1.7);
    hEffMapN[id]->SetStats(0);
    hEffMapN[id]->Draw("colz");

    Can.cd(2)->SetLogy();
    TH1F* Hist1D = FidHistFrom2D(hEffMapN[id], "", 30, FidRegionTrack);
    Hist1D->SetTitle( TString::Format("Tracking Efficiency Ch%i ROC%i", Channel, ROC));
    Hist1D->SetXTitle("Efficiency");
    Hist1D->SetYTitle("# of Pixels");
    Hist1D->SetMinimum(0.5);
    Hist1D->Clone()->Draw();

    Can.cd(3);
    Hist1D->SetMinimum(0);
    Hist1D->Draw();

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
