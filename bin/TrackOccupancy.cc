////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Nov  1 10:26:12 CET 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"
#include "PLTU.h"


int TrackOccupancy (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_m2_m2);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Map for all ROC hists and canvas
  std::map<int, TH2F*> hMap;


  // Loop over all events
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    if (ientry <  300000) continue;
    if (ientry >= 600000) break;


    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      // If this hists for this telescope don't exist create them
      int const Channel = Telescope->Channel();

      if (hMap.count(Channel * 10) == 0) {
        TString Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 0);
        hMap[Channel * 10 + 0] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
        Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 1);
        hMap[Channel * 10 + 1] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
        Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 2);
        hMap[Channel * 10 + 2] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
      }

      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        PLTTrack* Track = Telescope->Track(itrack);

        for (size_t icluster = 0; icluster != Track->NClusters(); ++icluster) {
          PLTCluster* Cluster = Track->Cluster(icluster);

          for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {
            PLTHit* Hit = Cluster->Hit(ihit);
            hMap[Channel * 10 + Hit->ROC()]->Fill(Hit->Column(), Hit->Row());
          }
        }

      }
    }
  }


  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, TH2F*>::iterator it = hMap.begin(); it != hMap.end(); ++it) {
    //    std::cout << "Here again!" << std::endl;
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    TString const Name = it->second->GetName();

    TCanvas Can(Name, Name, 400, 400);
    Can.cd();
    it->second->Draw("zcol");
    Can.SaveAs(Name + ".gif");
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

  TrackOccupancy(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
