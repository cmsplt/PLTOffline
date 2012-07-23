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
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_Diamond;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_Diamond;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_Seed_3x3, FidRegionHits);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  std::map<int, int> NTrkEvMap;

  TH2F* HistBeamSpot[3];
  HistBeamSpot[0] = new TH2F("BeamSpotX", "BeamSpot X=0;Y;Z;NTracks", 25, -50, 50, 25, -540, 340);
  HistBeamSpot[1] = new TH2F("BeamSpotY", "BeamSpot Y=0;X;Z;NTracks", 25, -50, 50, 25, -540, 340);
  HistBeamSpot[2] = new TH2F("BeamSpotZ", "BeamSpot Z=0;X;Y;NTracks", 25, -50, 50, 25, -50, 50);

  std::map<int, TH1F*> MapSlopeY;
  std::map<int, TH1F*> MapSlopeX;

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
        MapSlopeX[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaY/#DeltaZ");
      }


      if (Telescope->NClusters() > 3) continue;


      if (Telescope->NTracks() > 0 && NTrkEvMap[Telescope->Channel()]++ < 20) {
        Telescope->DrawTracksAndHits( TString::Format("plots/Tracks_Ch%i_Ev%i.gif", Telescope->Channel(), NTrkEvMap[Telescope->Channel()]).Data() );
      }

      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        PLTTrack* Track = Telescope->Track(itrack);

        HistBeamSpot[0]->Fill( Track->fPlaner[0][1], Track->fPlaner[0][2]);
        HistBeamSpot[1]->Fill( Track->fPlaner[1][0], Track->fPlaner[1][2]);
        HistBeamSpot[2]->Fill( Track->fPlaner[2][0], Track->fPlaner[2][1]);

        MapSlopeY[Telescope->Channel()]->Fill(Track->fTVY/Track->fTVZ);
        MapSlopeX[Telescope->Channel()]->Fill(Track->fTVX/Track->fTVZ);

      }
    }


  }

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
  Can.SaveAs("BeamSpot.gif");

  for (std::map<int, TH1F*>::iterator it = MapSlopeY.begin(); it != MapSlopeY.end(); ++it) {
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs( TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH1F*>::iterator it = MapSlopeX.begin(); it != MapSlopeX.end(); ++it) {
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs( TString(it->second->GetName()) + ".gif");
    delete it->second;
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
