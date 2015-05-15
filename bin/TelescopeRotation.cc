////////////////////////////////////////////////////////////////////
//
// Fixed by Grant Riley
//
// Created on: May 15 2015
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"

#include "PLTTrack.h"
#include "PLTAlignment.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"



int TelescopeRotation (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  // Set some basic style
  PLTU::SetStyle();
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);


  // Map for all ROC hists and canvas
  std::map<int, TH1F*> hMap;
  std::map<int, TCanvas*> cMap;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    for (unsigned it = 0; it != Event.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event.Telescope(it);

      int const Channel = Telescope->Channel();

      if (Telescope->NTracks() != 1 || Telescope->NClusters() != 3) {
        continue;
      }

      if (cMap.count(Channel) == 0) {
        char BUFF[500];
        sprintf(BUFF, "PX0MinusPX2_Ch%i", Channel);

        hMap[10*Channel + 0] = new TH1F(BUFF, BUFF, 40, -20, 20);
        sprintf(BUFF, "plots/PY0MinusPY2_Ch%i", Channel);
        hMap[10*Channel + 1] = new TH1F(BUFF, BUFF, 40, -20, 20);
        sprintf(BUFF, "plots/P0MinusP2_Ch%i", Channel);
        cMap[Channel] = new TCanvas(BUFF, BUFF, 600, 300);
        cMap[Channel]->Divide(2,1);
      }
      PLTTrack* Track = Telescope->Track(0);
      float PXDiff = Track->Cluster(0)->SeedHit()->Column() - Track->Cluster(2)->SeedHit()->Column();
      float PYDiff = Track->Cluster(0)->SeedHit()->Row()    - Track->Cluster(2)->SeedHit()->Row();

      hMap[10*Channel + 0]->Fill(PXDiff);
      hMap[10*Channel + 1]->Fill(PYDiff);

    }

  }

  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    it->second->cd(1);
    hMap[10*it->first + 0]->Draw();
    it->second->cd(2);
    hMap[10*it->first + 1]->Draw();
    it->second->SaveAs(TString(it->second->GetName()) + ".gif");

  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];

  TelescopeRotation(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
