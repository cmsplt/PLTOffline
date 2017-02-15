////////////////////////////////////////////////////////////////////
//
// PlotBXDistribution
// Paul Lujan
// November 25, 2015
//
// A very simple script to look for the BX read out in each event
// and plot the resulting distribution.
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <numeric>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TH1F.h"
#include "TFile.h"
#include "TCanvas.h"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE

// CODE BELOW

int PlotBXDistribution (std::string const DataFileName) {
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  // The plot.
  TH1F *bx = new TH1F("bx", "bx", 3564, 0.5, 3564.5);
  int n[3564] = {0};

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 50000 == 0) {
      int nsec = Event.Time()/1000;
      int hr = nsec/3600;
      int min = (nsec-(hr*3600))/60;
      int sec = nsec % 60;
      std::cout << "Processing event: " << ientry << " at " << std::setfill('0') << std::setw(2)
		<< hr << ":" << std::setw(2) << min << ":" << std::setw(2) << sec << "."
		<< std::setw(3) << Event.Time()%1000 << std::endl;
    }
    //if (ientry < 100000) continue;
    //if (ientry > 1000000) break;
    n[Event.BX()]++;
    bx->Fill(Event.BX());
  }
  std::cout << "Saving plot" << std::endl;
  for (int i=0; i<15; ++i) {
    std::cout << i << ": " << n[i] << std::endl;
  }
  TCanvas c("bx", "bx", 900, 900);
  c.cd();
  bx->Draw();
  c.SaveAs("bx.png");

  TFile *f = new TFile("bx.root", "RECREATE");
  bx->Write();
  f->Close();
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  PlotBXDistribution(DataFileName);

  return 0;
}
