////////////////////////////////////////////////////////////////////
//
// Grant Riley
//
// Created on: Fri May  8 18:26:18 UTC 2015
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TFile.h"




// FUNCTION DEFINITIONS HERE
int TrackDiagnostics (std::string const, std::string const, std::string const);









int TrackDiagnostics (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();
  gStyle->SetOptStat(1111);

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, FidRegionHits);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  std::map<int, int> NTrkEvMap;

  TH2F* HistBeamSpot[3];
  HistBeamSpot[0] = new TH2F("BeamSpotX", "BeamSpot X=0;Y;Z;NTracks", 25, -50, 50, 25, -540, 340);
  HistBeamSpot[1] = new TH2F("BeamSpotY", "BeamSpot Y=0;X;Z;NTracks", 25, -50, 50, 25, -540, 340);
  HistBeamSpot[2] = new TH2F("BeamSpotZ", "BeamSpot Z=0;X;Y;NTracks", 25, -50, 50, 25, -50, 50);

  std::map<int, TH1F*> MapSlopeY;
  std::map<int, TH1F*> MapSlopeX;
  std::map<int, TH1F*> MapSlopeY_3Cluster;
  std::map<int, TH1F*> MapSlopeX_3Cluster;
  std::map<int, TH1F*> MapSlopeY_2Cluster;
  std::map<int, TH1F*> MapSlopeX_2Cluster;
  std::map<int, TH2F*> MapSlope2D;
  std::map<int, TH1F*> NClusters_inTrack;
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }



    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      
      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);



      if (!MapSlopeY[Telescope->Channel()]) {
        TString Name = TString::Format("SlopeY_Ch%i", Telescope->Channel());
        MapSlopeY[Telescope->Channel()] = new TH1F(Name, Name, 50, -0.1, 0.1);
        MapSlopeY[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
        Name = TString::Format("SlopeX_Ch%i", Telescope->Channel());
        MapSlopeX[Telescope->Channel()] = new TH1F(Name, Name, 50, -0.1, 0.1);
        MapSlopeX[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
        Name = TString::Format("Slope2D_Ch%i", Telescope->Channel());
        MapSlope2D[Telescope->Channel()] = new TH2F(Name, Name, 100, -0.1, 0.1, 100, -0.1, 0.1);
        MapSlope2D[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
        MapSlope2D[Telescope->Channel()]->SetYTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
        Name = TString::Format("3Cluster_SlopeY_Ch%i", Telescope->Channel());
        MapSlopeY_3Cluster[Telescope->Channel()] = new TH1F(Name, Name, 50, -0.1, 0.1);
        MapSlopeY_3Cluster[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
        Name = TString::Format("3Cluster_SlopeX_Ch%i", Telescope->Channel());
        MapSlopeX_3Cluster[Telescope->Channel()] = new TH1F(Name, Name, 50, -0.1, 0.1);
        MapSlopeX_3Cluster[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
        Name = TString::Format("2Cluster_SlopeY_Ch%i", Telescope->Channel());
        MapSlopeY_2Cluster[Telescope->Channel()] = new TH1F(Name, Name, 50, -0.1, 0.1);
        MapSlopeY_2Cluster[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
        Name = TString::Format("2Cluster_SlopeX_Ch%i", Telescope->Channel());
        MapSlopeX_2Cluster[Telescope->Channel()] = new TH1F(Name, Name, 50, -0.1, 0.1);
        MapSlopeX_2Cluster[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
        Name = TString::Format("NClusters_inTrack_Ch%i", Telescope->Channel());
        NClusters_inTrack[Telescope->Channel()] = new TH1F(Name, Name, 15, -1, 14);
        NClusters_inTrack[Telescope->Channel()]->SetXTitle("Number of Clusters Per Track");
        
        
      }
//      if (Telescope->NClusters() > 3) continue;



      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        PLTTrack* Track = Telescope->Track(itrack);

        HistBeamSpot[0]->Fill( Track->fPlaner[0][1], Track->fPlaner[0][2]);
        HistBeamSpot[1]->Fill( Track->fPlaner[1][0], Track->fPlaner[1][2]);
        HistBeamSpot[2]->Fill( Track->fPlaner[2][0], Track->fPlaner[2][1]);

        MapSlopeY[Telescope->Channel()]->Fill(Track->fTVY/Track->fTVZ);
        MapSlopeX[Telescope->Channel()]->Fill(Track->fTVX/Track->fTVZ);
        MapSlope2D[Telescope->Channel()]->Fill(Track->fTVX/Track->fTVZ, Track->fTVY/Track->fTVZ);
        NClusters_inTrack[Telescope->Channel()]->Fill(Track->NClusters());
        if (Track->NClusters()==2){
        MapSlopeY_2Cluster[Telescope->Channel()]->Fill(Track->fTVY/Track->fTVZ);
        MapSlopeX_2Cluster[Telescope->Channel()]->Fill(Track->fTVX/Track->fTVZ);
}
        if (Track->NClusters()==3){
        MapSlopeY_3Cluster[Telescope->Channel()]->Fill(Track->fTVY/Track->fTVZ);
        MapSlopeX_3Cluster[Telescope->Channel()]->Fill(Track->fTVX/Track->fTVZ);
}
      }
    }


  }


  TFile *f = new TFile("Track_Diagnostics.root","RECREATE");

  TCanvas Can("BeamSpot", "BeamSpot", 900, 900);
  Can.Divide(3, 3);
  Can.cd(1);
  HistBeamSpot[0]->Draw("colz");
  Can.cd(2);
  HistBeamSpot[1]->Draw("colz");
  Can.cd(3);
  HistBeamSpot[2]->Draw("colz");

  Can.cd(1+3);
  HistBeamSpot[0]->ProjectionX()->Draw("ep");
  Can.cd(2+3);
  HistBeamSpot[1]->ProjectionX()->Draw("ep");
  Can.cd(3+3);
  HistBeamSpot[2]->ProjectionX()->Draw("ep");

  Can.cd(1+6);
  HistBeamSpot[0]->ProjectionY()->Draw("ep");
  Can.cd(2+6);
  HistBeamSpot[1]->ProjectionY()->Draw("ep");
  Can.cd(3+6);
  HistBeamSpot[2]->ProjectionY()->Draw("ep");
//  Can.SaveAs("plots/BeamSpot.gif");
  
  HistBeamSpot[0]->Write();
  HistBeamSpot[1]->Write();
  HistBeamSpot[2]->Write();

  for (std::map<int, TH1F*>::iterator it = MapSlopeY.begin(); it != MapSlopeY.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
//    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH1F*>::iterator it = MapSlopeX.begin(); it != MapSlopeX.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
//    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH2F*>::iterator it = MapSlope2D.begin(); it != MapSlope2D.end(); ++it) {
    it->second->Write();
    //TCanvas Can;
    //Can.cd();
    //it->second->Draw("hist");
    //Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    //delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = MapSlopeY_2Cluster.begin(); it != MapSlopeY_2Cluster.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
//    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH1F*>::iterator it = MapSlopeX_2Cluster.begin(); it != MapSlopeX_2Cluster.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
//    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = MapSlopeY_3Cluster.begin(); it != MapSlopeY_3Cluster.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
//    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH1F*>::iterator it = MapSlopeX_3Cluster.begin(); it != MapSlopeX_3Cluster.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
//    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
for (std::map<int, TH1F*>::iterator it = NClusters_inTrack.begin(); it != NClusters_inTrack.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
//    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  f->Close();

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

  TrackDiagnostics(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
