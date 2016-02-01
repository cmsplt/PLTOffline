////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue May 24 09:54:58 CEST 2011
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <numeric>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TH1F.h"
#include "TFile.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"
#include "TTree.h"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE

int ClusterLumi(std::string const DataFileName) {
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  //Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering);
  PLTPlane::FiducialRegion MyFiducialRegion = PLTPlane::kFiducialRegion_All;
  //  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(MyFiducialRegion);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  // Map for all ROC hists and canvas
  std::map<int, TCanvas*> cAllMap;
  std::map<int, TH2F*>    hOccupancyMap;
  std::map<int, TCanvas*> cOccupancyMap;
  std::map<int, std::vector<TH2F*> >    hOccupancyClMap;
  std::map<int, TCanvas*> cOccupancyClMap;
  std::map<int, TH2F*>    hQuantileMap;
  std::map<int, TCanvas*> cQuantileMap;
  std::map<int, TCanvas*> cProjectionMap;
  std::map<int, TCanvas*> cEfficiencyMap;
  std::map<int, TH2F*>    hEfficiencyMap;
  std::map<int, TH1F*>    hEfficiency1DMap;
  std::map<int, TCanvas*> cCoincidenceMap;
  std::map<int, TH1F*>    hCoincidenceMap;
  std::map<int, TH2F*>    hMeanMap;
  std::map<int, TCanvas*> cMeanMap;

  std::vector<float> events;
  std::vector<float> hitlumis;
  std::vector<float> tracklumis;
  std::vector<uint32_t> times;

  int n1hit=0, n2hit=0, n3hit=0;
  int n1track=0, n2track=0, n3track=0;
  int timestamp, event;

  TFile *f = new TFile("clusterLumi.root", "RECREATE");
  TTree *tr = new TTree("hits", "Hit data");
  tr->Branch("event", &event, "event/I");
  tr->Branch("n1hit", &n1hit, "n1hit/I");
  tr->Branch("n2hit", &n2hit, "n2hit/I");
  tr->Branch("n3hit", &n3hit, "n3hit/I");
  tr->Branch("n1track", &n1track, "n1track/I");
  tr->Branch("n2track", &n2track, "n2track/I");
  tr->Branch("n3track", &n3track, "n3track/I");
  tr->Branch("timestamp", &timestamp, "timestamp/I");

  // We want to convert the timestamps from "milliseconds until midnight"
  // to Unix timestamps. In order to do that, first get the date from the
  // file name.
  struct tm startTime;
  const char *startChar = strstr(DataFileName.c_str(), "Slink_");
  if (startChar==NULL) {
    std::cout << "Failed to parse timestamp from file name, sorry!" << std::endl;
    return 1;
  }

  // Next use strptime() to parse the time string.
  char *c = strptime(startChar, "Slink_%Y%m%d.%H%M%S.dat", &startTime);
  if (c==NULL) {
    std::cout << "Failed to parse timestamp from file name, sorry!" << std::endl;
    return 1;
  }
  // We just want the start of the day, so ignore the H/M/S.
  startTime.tm_hour = 0;
  startTime.tm_min = 0;
  startTime.tm_sec = 0;
  time_t timeOffset = mktime(&startTime);
  
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    //if (ientry == 300000) break;

    int n1h=0, n2h=0, n3h=0;
    int n1t=0, n2t=0, n3t=0;
    int hitsByChannel[36][3];
    for (int ic=0; ic<36; ++ic) {
      for (int ir=0; ir<3; ++ir) {
	hitsByChannel[ic][ir] = 0;
      }
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
      if (Plane->Channel() == 11) continue; // skip ch11, it's bad for now
      if (Plane->Channel() < 1 || Plane->Channel() > 36) {
	std::cerr << "Bad channel number found: " << Plane->Channel() << std::endl;
	continue;
      }
      
      if (Plane->NClusters() == 1) n1h++;
      if (Plane->NClusters() == 2) n2h++;
      if (Plane->NClusters() >= 3) n3h++;
      hitsByChannel[Plane->Channel()][Plane->ROC()] += Plane->NClusters();
	
    } // loop over planes

    // look for scopes with >n hits
    for (int ic=0; ic<36; ++ic) {
      if (hitsByChannel[ic][0] >= 3 && hitsByChannel[ic][1] >= 3 && hitsByChannel[ic][2] >= 3) n3t++;
      else if (hitsByChannel[ic][0] >= 2 && hitsByChannel[ic][1] >= 2 && hitsByChannel[ic][2] >= 2) n2t++;
      else if (hitsByChannel[ic][0] >= 1 && hitsByChannel[ic][1] >= 1 && hitsByChannel[ic][2] >=1 ) n1t++;
    }

    n1hit += n1h;
    n2hit += n2h;
    n3hit += n3h;
    n1track += n1t;
    n2track += n2t;
    n3track += n3t;

    if (ientry % 100000 == 99999) {
      float lumi = 0;
      event = ientry+1;
      timestamp = timeOffset+Event.Time()/1000; //convert milliseconds since day start to Unix time
      if (n1hit != 0) {
	lumi = 2*(float)n2hit/n1hit;
	//lumi = -logf((float)n2hit/n1hit);
	hitlumis.push_back(lumi);
	tracklumis.push_back(2*(float)n2track/n1track);
	events.push_back(ientry+1);
	times.push_back(timestamp);
      }
      tr->Fill();
      n1hit=0;
      n2hit=0;
      n3hit=0;
      n1track=0;
      n2track=0;
      n3track=0;
    }
    
  } // loop over events

  std::cout << "Done reading events.  Will make some plots now" << std::endl;

  std::cout << "First hit lumi is " << hitlumis[0] << " at " << events[0] << " time " << times[0] << std::endl;
  std::cout << "Last hit lumi is " << hitlumis[hitlumis.size()-1] << " at " << events[events.size()-1]
	    << " time " << times[times.size() - 1] << std::endl;
  std::cout << "First track lumi is " << tracklumis[0] << " at " << events[0] << " time " << times[0] << std::endl;
  std::cout << "Last track lumi is " << tracklumis[tracklumis.size()-1] << " at " << events[events.size()-1]
	    << " time " << times[times.size() - 1] << std::endl;

  TGraph *gr = new TGraph(hitlumis.size(), events.data(), hitlumis.data());
  gr->Draw("ALP");
  gr->GetXaxis()->SetTitle("Events");
  gr->GetYaxis()->SetTitle("#mu (arb. units)");
  gr->Write();
  TGraph *gr2 = new TGraph(tracklumis.size(), events.data(), tracklumis.data());
  gr2->Draw("ALP");
  gr2->GetXaxis()->SetTitle("Events");
  gr2->GetYaxis()->SetTitle("#mu (arb. units)");
  gr2->Write();

  tr->Write();
  f->Write();
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
  ClusterLumi(DataFileName);

  return 0;
}
