////////////////////////////////////////////////////////////////////
//
// PlotActiveBunches
// Daniel Gift
// June 21, 2016
//
// Adapted from PlotBXDistribution by Paul Lujan, November 25, 2015
//
// A script that determines which bunches are active in the current 
// fill and plots the number of hits as a function of BX. Also gives
// total number of active bunches and a list of which bunches are 
// active.
//
// For number of bunches, see Int_t numActive.
// For vector of bunches, see std::vector<Double_t> activeBunches.
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
#include "TProfile.h"
#include "TText.h"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE
const int NBX = 3564;
const int numEvents = 10000000; // Number of events to look at to determin activity

// CODE BELOW

int PlotActiveBunches (std::string const DataFileName) {
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
 
  // The plot.
  TProfile *avg = new TProfile("avg", "Average Number of Hits;Bunch ID;Average Hits per Event", NBX, 0.5, NBX + .5);
 
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 50000 == 0) {
      std::cout << "Processing event: " << ientry << " at " << Event.ReadableTime() << std::endl;
    }
    if (ientry > numEvents) break;

    // +1 because BX starts counting at 0, but LHC convention is to start at 1
    avg->Fill(Event.BX()+1, Event.NHits());
  }
   
  std::cout << "Saving plot" << std::endl;
  std::vector<Double_t> num;
  
  // Make a vector for all the bin values and sort it
  for (Int_t index = 1; index <= avg->GetXaxis()->GetNbins(); ++index) {
    num.push_back((float)(avg->GetBinContent(index)));
  }
  std::sort(num.begin(), num.end());

  // Find a large jump in the occupancy of each bin
  Double_t runningAvg = 0;
  Int_t breakPoint = 0;
  for (Int_t index = 1; index < NBX; ++index) {
    Double_t diff = num[index] - num[index - 1];
    
    // 6000 is somewhat arbitrary, but it worked. Adjust as need be.
    // Also wait a few events (here 4) to build up a running average first
    if ((diff > 1000*runningAvg) && (num[index-1] != 0) && (index > 4)) {
      breakPoint = index;
      break;
    }

    // Keep track of the average difference so far. 
    // We want this average to be greatly exceeded at the jump
    runningAvg = ((Double_t)(runningAvg*(index - 1) + diff))/((Double_t)index);
  }
   
  Int_t numActive = NBX-breakPoint;
  
  // Make a vector of the active bunches
  std::vector<Int_t> activeBunches;
  for (int index = 1; index < avg->GetXaxis()->GetNbins(); ++index) {
    if (avg->GetBinContent(index) >= num[breakPoint]) {
      activeBunches.push_back(index);
    }
  }
  
  // Plot

  TCanvas c("avg", "Average Number of Hits per BX", 900, 650);
  c.cd();
  gStyle->SetTitleX(.15);
  avg->Draw();
  
  // Draw a line separating the data
  double split = (num[breakPoint] + num[breakPoint-1])/2.0;
  
  TLine *line = new TLine(0, split , NBX+2, split);  
  line->SetLineWidth(4);
  line->SetLineColor(kRed);
  line->Draw();
  
  // Label text
  TText *t = new TText(NBX-1000,split*1.05,"Active Bunches");
  t->SetTextFont(133);
  t->SetTextSize(20);
  t->Draw();
  TText *u = new TText(NBX-1000,split*0.8,"Inactive Bunches");
  u->SetTextFont(133);
  u->SetTextSize(20);
  u->Draw();
  c.SaveAs("avg.png");
  
  
  // Save as a root file
  TFile *f = new TFile("avg.root", "RECREATE");
  avg->Write();
  line->Write();
  t->Write();
  u->Write();
  f->Close();
  for (size_t i = 0; i < activeBunches.size(); ++i) {
    std::cout<<activeBunches[i]<<std::endl;
  }
 
  std::cout << "Number of active bunches: " << numActive << std::endl;

  // Error handling. If this happens try changing the 6000 above
  if (breakPoint == 0)
    std::cout<<"Did not find large gap in data; break between active and inactive may be unclear"<<std::endl;
  
  
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }
  
  std::string const DataFileName = argv[1];
  PlotActiveBunches(DataFileName);
  
  return 0;
}
