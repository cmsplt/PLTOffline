////////////////////////////////////////////////////////////////////
//
//  OccupancyVsTime -- a simple script to see if the occupancy
//    in a given scope/ROC is changing over time
//    Paul Lujan, June 16 2016
//     time-handling code derived from MeasureAccidentals.cc, so
//     like MeasureAccidentals you can either feed it a timestamp
//     file to go step-by-step like for a scan, or leave that argument
//     blank to do 5-minute chunks
//   produces plots/occupancy_vs_time.root
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <iomanip>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TFile.h"
#include "PLTTimestampReader.h"

int OccupancyVsTime(const std::string, const std::string, const std::string, const std::string);

int OccupancyVsTime(const std::string DataFileName, const std::string TimestampFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  if (TimestampFileName != "")
    std::cout << "TimestampFileName: " << TimestampFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();
  gStyle->SetOptStat(1111);

  // Read timestamp file, if it exists.
  bool useTimestamps = false;
  int nSteps = 1;
  PLTTimestampReader *tsReader = nullptr;
  std::vector<std::pair<uint32_t, uint32_t> > timestamps;
  if (TimestampFileName != "") {
    tsReader = new PLTTimestampReader(TimestampFileName);
    timestamps = tsReader->getTimestamps();
    nSteps = tsReader->getSize();
    useTimestamps = true;
  }

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  std::vector<int> nEvents(nSteps);
  std::map<int, std::vector<int> > HitsByChannel;
  std::map<int, std::vector<int> > HitsByROC;

  const int nChannels = 13;
  const int fedChannel[nChannels] = {2, 4, 5, 7, 8, 10, 11, 13, 14, 16, 17, 19, 20};
  for (int i=0; i<nChannels; ++i) {
    int ch = fedChannel[i];
    HitsByChannel[ch].resize(nSteps);
    HitsByROC[10*ch].resize(nSteps);
    HitsByROC[10*ch+1].resize(nSteps);
    HitsByROC[10*ch+2].resize(nSteps);
  }  

  int stepNumber = 0;
  uint32_t currentStepStart = 0;
  // const uint32_t stepLength = 60000; // 1 minute
  const uint32_t stepLength = 300000; // 5 minutes
  
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 100000 == 0) {
      int nsec = Event.Time()/1000;
      int hr = nsec/3600;
      int min = (nsec-(hr*3600))/60;
      int sec = nsec % 60;
      std::cout << "Processing event: " << ientry << " at " << Event.Time() << " ("
		<< std::setfill('0') << std::setw(2) << (hr%24) << ":" << std::setw(2) << min
		<< ":" << std::setw(2) << sec << "." << std::setw(3) << Event.Time()%1000
		<< ")" << std::endl;
      FILE *f = fopen("abort.txt", "r");
      if (f != NULL) {
	fclose(f);
	break;
      }
    }
    //if (ientry>=5000000) break;

    if (useTimestamps) {
      if (Event.Time() > tsReader->lastTime()) break;
      stepNumber = tsReader->findTimestamp(Event.Time());
      if (stepNumber == -1) continue;
    } else {
      if (stepNumber == 0 && currentStepStart == 0) currentStepStart = Event.Time();
      if ((Event.Time() - currentStepStart) > stepLength) {
	//std::cout << "starting new step @" << Event.Time() << " from " << currentStepStart << std::endl;
	timestamps.push_back(std::make_pair(currentStepStart, Event.Time()-1));
	for (int i=0; i<nChannels; ++i) {
	  int ch = fedChannel[i];
	  HitsByChannel[ch].push_back(0);
	  HitsByROC[10*ch].push_back(0);
	  HitsByROC[10*ch+1].push_back(0);
	  HitsByROC[10*ch+2].push_back(0);
	}  

	currentStepStart = Event.Time();
	stepNumber++;
	nSteps++;
      }
    }
    nEvents[stepNumber]++;

    // Loop over all planes
    
    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {
      PLTPlane* Plane = Event.Plane(ip);
      int ch = Plane->Channel();
      if (ch < 2 || ch > 20) {
	std::cout << "Bad channel number found: " << ch << std::endl;
      } else {
	int roc = Plane->ROC();
	HitsByChannel[ch][stepNumber]++;
	HitsByROC[10*ch+roc][stepNumber]++;
      }
    } // plane loop


  } // event loop
  // properly catch the last step
  timestamps.push_back(std::make_pair(currentStepStart, Event.Time()));

  TFile *f = new TFile("plots/occupancy_vs_time.root","RECREATE");

  float xvals[1024];
  float yvalsc[1024];
  float yvalsr0[1024];
  float yvalsr1[1024];
  float yvalsr2[1024];
  char buf[64];

  for (int ich=0; ich<nChannels; ++ich) {
    int ch = fedChannel[ich];
    for (int istep=0; istep<nSteps; ++istep) {
      xvals[istep] = istep;
      yvalsc[istep] = HitsByChannel[ch][istep];
      yvalsr0[istep] = HitsByROC[10*ch][istep];
      yvalsr1[istep] = HitsByROC[10*ch+1][istep];
      yvalsr2[istep] = HitsByROC[10*ch+2][istep];
    }
    TGraph *gch = new TGraph(nSteps, xvals, yvalsc);
    sprintf(buf, "OccVsTimeCh%d", ch);
    gch->SetName(buf);
    TGraph *gr0 = new TGraph(nSteps, xvals, yvalsr0);
    sprintf(buf, "OccVsTimeCh%dROC0", ch);
    gr0->SetName(buf);
    TGraph *gr1 = new TGraph(nSteps, xvals, yvalsr1);
    sprintf(buf, "OccVsTimeCh%dROC1", ch);
    gr1->SetName(buf);
    TGraph *gr2 = new TGraph(nSteps, xvals, yvalsr2);
    sprintf(buf, "OccVsTimeCh%dROC2", ch);
    gr2->SetName(buf);
    
    gch->Write();
    gr0->Write();
    gr1->Write();
    gr2->Write();
  }
 
  f->Close();

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: " << argv[0] << " DataFile.dat [TimestampFile.dat]" << std::endl;
    return 1;
  }

  const std::string DataFileName = argv[1];
  const std::string TimestampFileName = (argc == 3) ? argv[2] : "";

  OccupancyVsTime(DataFileName, TimestampFileName);

  return 0;
}
