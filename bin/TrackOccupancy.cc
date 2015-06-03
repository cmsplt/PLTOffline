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
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Map for all ROC hists and canvas
  std::map<int, TH2F*> hMap;
  std::map<int, TCanvas*> cMap;


  // Loop over all events
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    if (ientry >= 100000) break;


    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      // If this hists for this telescope don't exist create them
      int const Channel = Telescope->Channel();

      if (hMap.count(Channel * 10) == 0) {
        char BUFF[500]; 
        TString Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 0);
        hMap[Channel * 10 + 0] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
        Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 1);
        hMap[Channel * 10 + 1] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
        Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 2);
        hMap[Channel * 10 + 2] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
        sprintf(BUFF, "plots/TrackOccupancy_%i", Channel);
        cMap[Channel] = new TCanvas(BUFF, BUFF, 900, 300);
        cMap[Channel]->Divide(3,1);
      }

      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        PLTTrack* Track = Telescope->Track(itrack);
        PLTCluster* ClustTest = Track->Cluster(1);
        PLTHit* HitTest = ClustTest->Hit(0);
        float slopex = (Track->fTVX/Track->fTVZ);
        float slopey = (Track->fTVY/Track->fTVZ);
        if (slopex>-0.02&&slopex<0.02&&slopey>-0.01&&slopey<0.03&&HitTest->Column()>11&&HitTest->Column()<40&&HitTest->Row()>18&&HitTest->Row()<60){
          for (size_t icluster = 0; icluster != Track->NClusters(); ++icluster) {
            PLTCluster* Cluster = Track->Cluster(icluster);
            for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {
              PLTHit* Hit = Cluster->Hit(ihit);
              if(ihit>0){continue;}
              hMap[Channel * 10 + Hit->ROC()]->Fill(Hit->Column(), Hit->Row());
            }
          }
        }
      }
    }
  }


  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    //    std::cout << "Here again!" << std::endl;
    // Decode the ID

    TString const Name = it->second->GetName();
    it->second->cd(1);
    hMap[10*it->first + 0]->Draw("zcol");
    it->second->cd(2);
    hMap[10*it->first + 1]->Draw("zcol");
    it->second->cd(3);
    hMap[10*it->first + 2]->Draw("zcol");
    it->second->SaveAs(Name + ".gif");
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
