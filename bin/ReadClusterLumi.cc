////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue May 24 09:54:58 CEST 2011
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <math.h>

#include "TH1F.h"
#include "TFile.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TROOT.h"
#include "TTree.h"
#include "TChain.h"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE

const int nPlanes = 42;
const int nScopes = 13; // since ch11 is excluded
const int eventsPerEntry = 100000;

int ReadClusterLumi(std::string const DataFileName) {
  TFile *f = new TFile("clusterLumiPlots.root", "RECREATE");

  TChain tr("hits");
  tr.Add(DataFileName.c_str());
  int nentries = tr.GetEntries();
  std::cout << "Found " << nentries << " entries in tree" << std::endl;
  if (nentries == 0) return 0;

  int event=0, n1hit=0, n2hit=0, n3hit=0;
  int n1track=0, n2track=0, n3track=0, timestamp=0;

  tr.SetBranchAddress("event", &event);
  tr.SetBranchAddress("n1hit", &n1hit);
  tr.SetBranchAddress("n2hit", &n2hit);
  tr.SetBranchAddress("n3hit", &n3hit);
  tr.SetBranchAddress("n1track", &n1track);
  tr.SetBranchAddress("n2track", &n2track);
  tr.SetBranchAddress("n3track", &n3track);
  tr.SetBranchAddress("timestamp", &timestamp);

  std::vector<float> events;
  std::vector<float> hitLumiZero;
  std::vector<float> hitLumi10;
  std::vector<float> hitLumi21;
  std::vector<float> trackLumiZero;
  std::vector<float> trackLumi10;
  std::vector<float> trackLumi21;
  //std::vector<uint32_t> times;
  std::vector<float> times;

  // Loop over all events in file
  for (int ientry = 0; ientry < nentries; ++ientry) {
    tr.GetEntry(ientry);
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

//     std::cout << "got e " << event << " h1 " << n1hit << " h2 " << n2hit << " h3 " << n3hit
// 	      << " t1 " << n1track << " t2 " << n2track << " t3 " << n3track << " ts " << timestamp << std::endl;
    
    events.push_back(event);
    int n0hit = nPlanes*eventsPerEntry - n1hit - n2hit - n3hit;
    hitLumiZero.push_back(-logf((float)n0hit/(nPlanes*eventsPerEntry)));
    hitLumi10.push_back((float)n1hit/n0hit);
    hitLumi21.push_back(2.0*(float)n2hit/n1hit);

    int n0track = nScopes*eventsPerEntry - n1track - n2track - n3track;
    trackLumiZero.push_back(-logf((float)n0track/(nScopes*eventsPerEntry)));
    trackLumi10.push_back((float)n1track/n0track);
    trackLumi21.push_back(2.0*(float)n2track/n1track);
    
    times.push_back(timestamp+7200); // small fix for time zone issues
  }

  std::cout << "Done reading events.  Will make some plots now" << std::endl;

  std::cout << "First hit lumi is " << hitLumiZero[0] << " at " << events[0] << " time " << times[0] << std::endl;
  std::cout << "Last hit lumi is " << hitLumiZero[hitLumiZero.size()-1] << " at " << events[events.size()-1]
	    << " time " << times[times.size() - 1] << std::endl;
  std::cout << "First track lumi is " << trackLumiZero[0] << " at " << events[0] << " time " << times[0] << std::endl;
  std::cout << "Last track lumi is " << trackLumiZero[trackLumiZero.size()-1] << " at " << events[events.size()-1]
	    << " time " << times[times.size() - 1] << std::endl;

  TGraph *grH0 = new TGraph(hitLumiZero.size(), times.data(), hitLumiZero.data());
  grH0->Draw("ALP");
  grH0->SetName("grH0");
  grH0->SetTitle("Lumi from PLT pixel, hit, -ln f_{0}");
  grH0->GetXaxis()->SetTitle("Time");
  grH0->GetXaxis()->SetTimeDisplay(1);
  grH0->GetXaxis()->SetTimeFormat("%H:%M");
  grH0->GetXaxis()->SetTimeOffset(0, "gmt");
  grH0->GetYaxis()->SetTitle("#mu (arb. units)");
  grH0->Write();
  TGraph *grH10 = new TGraph(hitLumi10.size(), times.data(), hitLumi10.data());
  grH10->Draw("ALP");
  grH10->SetName("grH10");
  grH10->SetTitle("Lumi from PLT pixel, hit, f_{1}/f_{0}");
  grH10->GetXaxis()->SetTitle("Time");
  grH10->GetXaxis()->SetTimeDisplay(1);
  grH10->GetXaxis()->SetTimeFormat("%H:%M");
  grH10->GetXaxis()->SetTimeOffset(0, "gmt");
  grH10->GetYaxis()->SetTitle("#mu (arb. units)");
  grH10->Write();
  TGraph *grH21 = new TGraph(hitLumi21.size(), times.data(), hitLumi21.data());
  grH21->Draw("ALP");
  grH21->SetName("grH21");
  grH21->SetTitle("Lumi from PLT pixel, hit, 2*f_{2}/f_{1}");
  grH21->GetXaxis()->SetTitle("Time");
  grH21->GetXaxis()->SetTimeDisplay(1);
  grH21->GetXaxis()->SetTimeFormat("%H:%M");
  grH21->GetXaxis()->SetTimeOffset(0, "gmt");
  grH21->GetYaxis()->SetTitle("#mu (arb. units)");
  grH21->Write();

  TGraph *grT0 = new TGraph(trackLumiZero.size(), times.data(), trackLumiZero.data());
  grT0->Draw("ALP");
  grT0->SetName("grT0");
  grT0->SetTitle("Lumi from PLT pixel, track, -ln f_{0}");
  grT0->GetXaxis()->SetTitle("Time");
  grT0->GetXaxis()->SetTimeDisplay(1);
  grT0->GetXaxis()->SetTimeFormat("%H:%M");
  grT0->GetXaxis()->SetTimeOffset(0, "gmt");
  grT0->GetYaxis()->SetTitle("#mu (arb. units)");
  grT0->Write();
  TGraph *grT10 = new TGraph(trackLumi10.size(), times.data(), trackLumi10.data());
  grT10->Draw("ALP");
  grT10->SetName("grT10");
  grT10->SetTitle("Lumi from PLT pixel, track, f_{1}/f_{0}");
  grT10->GetXaxis()->SetTitle("Time");
  grT10->GetXaxis()->SetTimeDisplay(1);
  grT10->GetXaxis()->SetTimeFormat("%H:%M");
  grT10->GetXaxis()->SetTimeOffset(0, "gmt");
  grT10->GetYaxis()->SetTitle("#mu (arb. units)");
  grT10->Write();
  TGraph *grT21 = new TGraph(trackLumi21.size(), times.data(), trackLumi21.data());
  grT21->Draw("ALP");
  grT21->SetName("grT21");
  grT21->SetTitle("Lumi from PLT pixel, track, 2*f_{2}/f_{1}");
  grT21->GetXaxis()->SetTitle("Time");
  grT21->GetXaxis()->SetTimeDisplay(1);
  grT21->GetXaxis()->SetTimeFormat("%H:%M");
  grT21->GetXaxis()->SetTimeOffset(0, "gmt");
  grT21->GetYaxis()->SetTitle("#mu (arb. units)");
  grT21->Write();

  f->Write();
  f->Close();
    
  return 0;
}

int main (int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [RootFile.root]" << std::endl;
    return 1;
  }
  
  std::string const DataFileName = argv[1];
  ReadClusterLumi(DataFileName);
  
  return 0;
}
