////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Oct 11 05:59:22 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"


#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"



int TelescopeRotation (std::string const DataFileName)
{
  // Set some basic style
  PLTU::SetStyle();
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);


  // Map for all ROC hists and canvas
  std::map<int, TH1F*> hMap;
  std::map<int, TCanvas*> cMap;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    for (int it = 0; it != Event.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event.Telescope(it);

      int const Channel = Telescope->Channel();

      if (Telescope->HitPlaneBits() != 0x7 || Telescope->NClusters() != 3) {
        continue;
      }

      if (cMap.count(Channel) == 0) {
        char BUFF[500];
        sprintf(BUFF, "PX0MinusPX2_Ch%i", Channel);

        hMap[10*Channel + 0] = new TH1F(BUFF, BUFF, 40, -20, 20);
        sprintf(BUFF, "PY0MinusPY2_Ch%i", Channel);
        hMap[10*Channel + 1] = new TH1F(BUFF, BUFF, 40, -20, 20);
        sprintf(BUFF, "P0MinusP2_Ch%i", Channel);
        cMap[Channel] = new TCanvas(BUFF, BUFF, 600, 300);
        cMap[Channel]->Divide(2,1);
      }

      float PXDiff = Telescope->Plane(0)->Cluster(0)->SeedHit()->Column() - Telescope->Plane(2)->Cluster(0)->SeedHit()->Column();
      float PYDiff = Telescope->Plane(0)->Cluster(0)->SeedHit()->Row()    - Telescope->Plane(2)->Cluster(0)->SeedHit()->Row();

      hMap[10*Channel + 0]->Fill(PXDiff);
      hMap[10*Channel + 1]->Fill(PYDiff);

    }

  }

  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    it->second->cd(1);
    hMap[10*it->first + 0]->Draw();
    it->second->cd(2);
    hMap[10*it->first + 1]->Draw();
    it->second->SaveAs(TString(it->second->GetName()) + ".eps");

  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];

  TelescopeRotation(DataFileName);

  return 0;
}
