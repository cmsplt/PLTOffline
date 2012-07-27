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

int SlopeSelection (int Channel, PLTTrack Track)
{
  //The selection window is set by looking at Slope{X,Y}_CH#.gif from MakeTracks.
  return 1;//Remove this line to turn on the selection for CH 13,14,16,24
  float slopeX = Track.fTVX/Track.fTVZ, slopeY = Track.fTVY/Track.fTVZ;
  switch (Channel)
    {
    case 13:
      if(slopeX>-0.013 && slopeX<-0.008 && slopeY>0.010 && slopeY<0.016) return 1;
      break;
    case 24:
      if(slopeX>0.015 && slopeX<0.022 && slopeY>-0.003 && slopeY<0.004) return 1;
      break;
    case 14:
      if(slopeX>-0.012 && slopeX<-0.005 && slopeY>-0.008 && slopeY<-0.005) return 1;
      break;
    case 16:
      if(slopeX>0.002 && slopeX<0.004 && slopeY>-0.022 && slopeY<-0.015) return 1;
      break;
    default:
      return 1;
    }
  return 0;
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
  std::map<int, TH1F*> hEffMapSlopeXN;
  std::map<int, TH1F*> hEffMapSlopeXD;
  std::map<int, TH1F*> hEffMapSlopeYN;
  std::map<int, TH1F*> hEffMapSlopeYD;
  std::map<int, TH1F*> hEffMapPulseHeightN;
  std::map<int, TH1F*> hEffMapPulseHeightD;
  std::map<int, TH1F*> hMapPulseHeights;

  float const PixelDist = 3;

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
          hEffMapN[Channel * 10 + iroc] = new TH2F(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
          Name = TString::Format("EffNumeratorSlopeX_Ch%i_ROC%i", Channel, iroc);
	  hEffMapSlopeXN[Channel * 10 + iroc] = new TH1F(Name, Name, 100, -0.1, 0.1);
          Name = TString::Format("EffNumeratorSlopeY_Ch%i_ROC%i", Channel, iroc);
          hEffMapSlopeYN[Channel * 10 + iroc] = new TH1F(Name, Name, 100, -0.1, 0.1);
          Name = TString::Format("EffNumeratorPulseHeight_Ch%i_ROC%i", Channel, iroc);
          hEffMapPulseHeightN[Channel * 10 + iroc] = new TH1F(Name, Name, 60, -1000, 50000);
          Name = TString::Format("EffDenominator%i_ROC%i", Channel, iroc);
          hEffMapD[Channel * 10 + iroc] = new TH2F(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
          Name = TString::Format("EffDenominatorSlopeX_Ch%i_ROC%i", Channel, iroc);
          hEffMapSlopeXD[Channel * 10 + iroc] = new TH1F(Name, Name, 100, -0.1, 0.1);
          Name = TString::Format("EffDenominatorSlopeY_Ch%i_ROC%i", Channel, iroc);
          hEffMapSlopeYD[Channel * 10 + iroc] = new TH1F(Name, Name, 100, -0.1, 0.1);
          Name = TString::Format("EffDenominatorPulseHeight_Ch%i_ROC%i", Channel, iroc);
          hEffMapPulseHeightD[Channel * 10 + iroc] = new TH1F(Name, Name, 60, -1000, 50000);
          Name = TString::Format("ExtrapolatedTrackPulseHeights_Ch%i_ROC%i", Channel, iroc);
          hMapPulseHeights[Channel * 10 + iroc] = new TH1F(Name, Name, 60, -1000, 50000);
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
          if(SlopeSelection(Channel,Tracks[1])) ++HC[Channel].NFiducial[2];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 2);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[1].TX( CP->LZ ), Tracks[1].TY( CP->LZ ), Channel, 2);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 2]->Fill(PXY.first, PXY.second);
          hEffMapSlopeXD[Channel * 10 + 2]->Fill(Tracks[1].fTVX/Tracks[1].fTVZ);
          hEffMapSlopeYD[Channel * 10 + 2]->Fill(Tracks[1].fTVY/Tracks[1].fTVZ);
	  float cluster_charge = 0;
	  if(Plane[2]->NClusters()) cluster_charge=Plane[2]->Cluster(0)->Charge();
          hEffMapPulseHeightD[Channel * 10 + 2]->Fill(cluster_charge);
          if (Plane[2]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[1].LResiduals( *(Plane[2]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            //printf("ResXY: %15.9f   RPXY: %15.9f\n", ResXY.first, RPXY.first);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
              //hEffMapN[Channel * 10 + 2]->Fill(Plane[2]->Cluster(0)->SeedHit()->Column(), Plane[2]->Cluster(0)->SeedHit()->Row());
              hEffMapN[Channel * 10 + 2]->Fill(PXY.first, PXY.second);
	      hEffMapSlopeXN[Channel * 10 + 2]->Fill(Tracks[1].fTVX/Tracks[1].fTVZ);
	      hEffMapSlopeYN[Channel * 10 + 2]->Fill(Tracks[1].fTVY/Tracks[1].fTVZ);
	      hEffMapPulseHeightN[Channel * 10 + 2]->Fill(cluster_charge);
	      hMapPulseHeights[Channel * 10 + 2]->Fill(cluster_charge);
              if(SlopeSelection(Channel,Tracks[1])) ++HC[Channel].NFiducialAndHit[2];
            } else {
              static int ievent = 0;
              if (ievent < 20) {
                Telescope->DrawTracksAndHits( TString::Format("plots/Jeebus_%04i.gif", ievent++).Data());
              }
	      hMapPulseHeights[Channel * 10 + 2]->Fill(0);
            }
          }
        }
      }

      // Test of plane 1
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[2].IsFiducial(Channel, 1, Alignment, FidRegionTrack) && Tracks[2].NHits() == 2) {
          if(SlopeSelection(Channel,Tracks[2])) ++HC[Channel].NFiducial[1];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 1);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[2].TX( CP->LZ ), Tracks[2].TY( CP->LZ ), Channel, 1);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 1]->Fill(PXY.first, PXY.second);
          hEffMapSlopeXD[Channel * 10 + 1]->Fill(Tracks[2].fTVX/Tracks[2].fTVZ);
          hEffMapSlopeYD[Channel * 10 + 1]->Fill(Tracks[2].fTVY/Tracks[2].fTVZ);
	  float cluster_charge = 0;
	  if(Plane[1]->NClusters()) cluster_charge=Plane[1]->Cluster(0)->Charge();
          hEffMapPulseHeightD[Channel * 10 + 1]->Fill(cluster_charge);
          if (Plane[1]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[2].LResiduals( *(Plane[1]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
              hEffMapN[Channel * 10 + 1]->Fill(PXY.first, PXY.second);
	      hEffMapSlopeXN[Channel * 10 + 1]->Fill(Tracks[2].fTVX/Tracks[2].fTVZ);
	      hEffMapSlopeYN[Channel * 10 + 1]->Fill(Tracks[2].fTVY/Tracks[2].fTVZ);
	      hEffMapPulseHeightN[Channel * 10 + 1]->Fill(cluster_charge);
	      hMapPulseHeights[Channel * 10 + 1]->Fill(cluster_charge);
              if(SlopeSelection(Channel,Tracks[2])) ++HC[Channel].NFiducialAndHit[1];
            } else hMapPulseHeights[Channel * 10 + 1]->Fill(0);
          }
        }
      }

      // Test of plane 0
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[3].IsFiducial(Channel, 0, Alignment, FidRegionTrack) && Tracks[3].NHits() == 2) {
          if(SlopeSelection(Channel,Tracks[3])) ++HC[Channel].NFiducial[0];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 0);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[3].TX( CP->LZ ), Tracks[3].TY( CP->LZ ), Channel, 0);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          hEffMapD[Channel * 10 + 0]->Fill(PXY.first, PXY.second);
          hEffMapSlopeXD[Channel * 10 + 0]->Fill(Tracks[3].fTVX/Tracks[3].fTVZ);
          hEffMapSlopeYD[Channel * 10 + 0]->Fill(Tracks[3].fTVY/Tracks[3].fTVZ);
	  float cluster_charge = 0;
	  if(Plane[0]->NClusters()) cluster_charge=Plane[0]->Cluster(0)->Charge();
          hEffMapPulseHeightD[Channel * 10 + 0]->Fill(cluster_charge);
          if (Plane[0]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[3].LResiduals( *(Plane[0]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
              hEffMapN[Channel * 10 + 0]->Fill(PXY.first, PXY.second);
	      hEffMapSlopeXN[Channel * 10 + 0]->Fill(Tracks[3].fTVX/Tracks[3].fTVZ);
	      hEffMapSlopeYN[Channel * 10 + 0]->Fill(Tracks[3].fTVY/Tracks[3].fTVZ);
	      hEffMapPulseHeightN[Channel * 10 + 0]->Fill(cluster_charge);
	      hMapPulseHeights[Channel * 10 + 0]->Fill(cluster_charge);
              if(SlopeSelection(Channel,Tracks[3])) ++HC[Channel].NFiducialAndHit[0];
            } else hMapPulseHeights[Channel * 10 + 0]->Fill(0);
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

    TString Name = TString::Format("TrackingEfficiencyMap_Ch%i_ROC%i", Channel, ROC);
    TCanvas Can1(Name, Name, 400, 1200);
    Can1.Divide(1, 3);

    Can1.cd(1);
    hEffMapN[id]->SetTitle(Name);
    hEffMapN[id]->Divide(it->second);
    hEffMapN[id]->SetMinimum(0.3);
    hEffMapN[id]->SetMaximum(1.7);
    hEffMapN[id]->SetStats(0);
    hEffMapN[id]->Draw("colz");

    Can1.cd(2)->SetLogy();
    TH1F* Hist1D = FidHistFrom2D(hEffMapN[id], "", 30, FidRegionTrack);
    Hist1D->SetTitle( TString::Format("Tracking Efficiency Ch%i ROC%i", Channel, ROC));
    Hist1D->SetXTitle("Efficiency");
    Hist1D->SetYTitle("# of Pixels");
    Hist1D->SetMinimum(0.5);
    Hist1D->Clone()->Draw();

    Can1.cd(3);
    Hist1D->SetMinimum(0);
    Hist1D->Draw();

    Name = TString::Format("plots/TrackingEfficiencyMap_Ch%i_ROC%i", Channel, ROC);
    Can1.SaveAs(Name + ".gif");

    Name = TString::Format("TrackingEfficiencySlopes_Ch%i_ROC%i", Channel, ROC);
    TCanvas Can2(Name, Name, 600, 1200);
    Can2.Divide(1, 3);

    Can2.cd(1);
    Name = TString::Format("Tracking Efficiency vs. Slope X for Ch%i ROC%i", Channel, ROC);
    hEffMapSlopeXN[id]->SetTitle(Name);
    hEffMapSlopeXN[id]->Divide(hEffMapSlopeXD[id]);
    hEffMapSlopeXN[id]->GetYaxis()->SetTitle("Tracking Efficiency");
    hEffMapSlopeXN[id]->GetXaxis()->SetTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
    hEffMapSlopeXN[id]->SetStats(0);
    hEffMapSlopeXN[id]->Draw();

    Can2.cd(2);
    Name = TString::Format("Tracking Efficiency vs. Slope Y for Ch%i ROC%i", Channel, ROC);
    hEffMapSlopeYN[id]->SetTitle(Name);
    hEffMapSlopeYN[id]->Divide(hEffMapSlopeYD[id]);
    hEffMapSlopeYN[id]->GetYaxis()->SetTitle("Tracking Efficiency");
    hEffMapSlopeYN[id]->GetXaxis()->SetTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
    hEffMapSlopeYN[id]->SetStats(0);
    hEffMapSlopeYN[id]->Draw();

    Can2.cd(3);
    Name = TString::Format("Tracking Efficiency vs. Pulse Height for Ch%i ROC%i", Channel, ROC);
    hEffMapPulseHeightN[id]->SetTitle(Name);
    hEffMapPulseHeightN[id]->Divide(hEffMapPulseHeightD[id]);
    hEffMapPulseHeightN[id]->GetYaxis()->SetTitle("Tracking Efficiency");
    hEffMapPulseHeightN[id]->GetXaxis()->SetTitle("Electrons");
    hEffMapPulseHeightN[id]->SetStats(0);
    hEffMapPulseHeightN[id]->Draw();

    Name = TString::Format("plots/TrackingEfficiencySlopes_Ch%i_ROC%i", Channel, ROC);
    Can2.SaveAs(Name + ".gif");

    Name = TString::Format("ExtrapolatedTrackPulseHeights_Ch%i_ROC%i", Channel, ROC);
    TCanvas Can3(Name, Name, 400, 400);

    Can3.cd(1);
    Name = TString::Format("Extrapolated Track Pulse Heights for Ch%i ROC%i", Channel, ROC);
    hMapPulseHeights[id]->SetTitle(Name);
    //hEffMapSlopeXN[id]->GetYaxis()->SetTitle("Events per");
    hMapPulseHeights[id]->GetXaxis()->SetTitle("Electrons");
    hMapPulseHeights[id]->SetFillColor(40);
    gStyle->SetOptStat(10);
    hMapPulseHeights[id]->Draw();
    
    Name = TString::Format("plots/ExtrapolatedTrackPulseHeights_Ch%i_ROC%i", Channel, ROC);
    Can3.SaveAs(Name + ".gif");

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
