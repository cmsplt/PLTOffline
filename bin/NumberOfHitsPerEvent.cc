////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Jul 13 17:18:10 CEST 2011
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

// FUNCTION DEFINITIONS HERE


// CODE BELOW

int NumberOfHitsPerEvent(std::string const DataFileName)
{
  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName);

  // Map for all ROC hists and canvas
  std::map<int, TH1F*> hMap;
  std::map<int, TH1F*> hClMap;
  std::map<int, TH1F*> hHPCMap;

  std::map<int, TCanvas*> cMap;

  // Loop over all events in file
  int ientry = 0;
  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    // Loop over all planes with hits in event
    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {

      // THIS plane is
      PLTPlane* Plane = Event.Plane(ip);
      if (Plane->ROC() > 2) {
        std::cerr << "WARNING: ROC > 2 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->ROC() < 0) {
        std::cerr << "WARNING: ROC < 0 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->Channel() > 99) {
        std::cerr << "WARNING: Channel > 99 found: " << Plane->Channel() << std::endl;
        continue;
      }

      int ID = 10 * Plane->Channel() + Plane->ROC();
      if (!hMap.count(ID)) {
        TString Name = TString::Format("NClustersPerEvent_Ch%02i_ROC%1i",  Plane->Channel(), Plane->ROC());
        hClMap[ID] = new TH1F(Name, Name, 50, 0, 50);
        Name = TString::Format("NHitsPerEvent_Ch%02i_ROC%1i",  Plane->Channel(), Plane->ROC());
        hMap[ID] = new TH1F(Name, Name, 50, 0, 50);
        Name = TString::Format("NHitsPerCluster_Ch%02i_ROC%1i",  Plane->Channel(), Plane->ROC());
        hHPCMap[ID] = new TH1F(Name, Name, 25, 0, 25);

        Name = TString::Format("NHitsPerEvent_Ch%02i",  Plane->Channel());
        if (!cMap.count(Plane->Channel())) {
          cMap[Plane->Channel()] = new TCanvas(Name, Name, 900, 900);
          cMap[Plane->Channel()]->Divide(3,3);
        }
      }

      hMap[ID]->Fill(Plane->NHits());
      hClMap[ID]->Fill(Plane->NClusters());
      for (size_t ic = 0; ic != Plane->NClusters(); ++ic) {
        hHPCMap[ID]->Fill( Plane->Cluster(ic)->NHits() );
      }


    }

  }
  std::cout << "Number of events: " << ientry << std::endl;

  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, TH1F*>::iterator it = hMap.begin(); it != hMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    printf("Drawing hist for Channel %2i ROC %i\n", Channel, ROC);

    cMap[Channel]->cd(ROC+1)->SetLogy(1);
    hMap[it->first]->Draw();
    cMap[Channel]->cd(ROC+1+3)->SetLogy(1);
    hClMap[it->first]->Draw();
    cMap[Channel]->cd(ROC+1+6)->SetLogy(1);
    hHPCMap[it->first]->Draw();
  }

  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    TString Name = TString::Format("plots/NHitsPerEvent_Ch%02i.gif", it->first);
    it->second->SaveAs( Name );
    delete it->second;
  }



  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  NumberOfHitsPerEvent(DataFileName);

  return 0;
}
