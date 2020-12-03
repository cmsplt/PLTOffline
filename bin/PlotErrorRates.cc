////////////////////////////////////////////////////////////////////
//
// PlotErrors
// Paul Lujan
// December 1, 2020
//
// This plots the number of errors per minute of each type over
// the course of a single fill.
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <algorithm>

#include "PLTEvent.h"
#include "PLTError.h"

#include "TROOT.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"

int main (int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];

  // Set up the PLTEvent reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  // Set up vectors
  std::vector<int> nTimeoutError = {0};
  std::vector<int> nEventNumberError = {0};
  std::vector<int> nNearFullError = {0};
  std::vector<int> nFEDTrailerError = {0};
  std::vector<int> nTBMError = {0};
  std::vector<int> nUnknownError = {0};
  std::vector<int> startTimes = {0};
  int currentStep = 0;
  uint32_t currentStepStart = 0;
  unsigned int stepLength = 60*1000;

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

    // Step bookkeeping
    if (currentStep == 0 && currentStepStart == 0) {
      currentStepStart = Event.Time();
      startTimes[0] = currentStepStart;
    }
    if ((Event.Time() - currentStepStart) > stepLength) {
      //std::cout << "starting new step @" << Event.Time() << " from " << currentStepStart << std::endl;
      nTimeoutError.push_back(0);
      nEventNumberError.push_back(0);
      nNearFullError.push_back(0);
      nFEDTrailerError.push_back(0);
      nTBMError.push_back(0);
      nUnknownError.push_back(0);
      currentStepStart = Event.Time();
      startTimes.push_back(currentStepStart);
      currentStep++;
    }

    const std::vector<PLTError>& errors = Event.GetErrors();
    for (std::vector<PLTError>::const_iterator it = errors.begin(); it != errors.end(); ++it) {
      ErrorType errType = it->GetErrorType();
      if (errType == kTimeOut) nTimeoutError[currentStep]++;
      else if (errType == kEventNumberError) nEventNumberError[currentStep]++;
      else if (errType == kNearFull) nNearFullError[currentStep]++;
      else if (errType == kFEDTrailerError) nFEDTrailerError[currentStep]++;
      else if (errType == kTBMError) nTBMError[currentStep]++;
      else nUnknownError[currentStep]++;
    }

    //if (currentStep == 3) break;
  }

  TGraph *g_to = new TGraph(startTimes.size(), startTimes.data(), nTimeoutError.data());
  TGraph *g_en = new TGraph(startTimes.size(), startTimes.data(), nEventNumberError.data());
  TGraph *g_nf = new TGraph(startTimes.size(), startTimes.data(), nNearFullError.data());
  TGraph *g_ft = new TGraph(startTimes.size(), startTimes.data(), nFEDTrailerError.data());
  TGraph *g_tb = new TGraph(startTimes.size(), startTimes.data(), nTBMError.data());
  TGraph *g_un = new TGraph(startTimes.size(), startTimes.data(), nUnknownError.data());

  int nmax = std::max({*std::max_element(nTimeoutError.begin(), nTimeoutError.end()), *std::max_element(nEventNumberError.begin(), nEventNumberError.end()), 
	*std::max_element(nNearFullError.begin(), nNearFullError.end()), *std::max_element(nFEDTrailerError.begin(), nFEDTrailerError.end()), 
	*std::max_element(nTBMError.begin(), nTBMError.end()), *std::max_element(nUnknownError.begin(), nUnknownError.end())});

  TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
  g_to->Draw("AP");
  g_to->SetTitle("Error rates vs. time");
  //g_to->GetXaxis()->SetTitle("CERN time");
  g_to->GetXaxis()->SetTitle("PLT time");
  g_to->GetYaxis()->SetTitle("Errors/minute");
  g_to->GetYaxis()->SetTitleOffset(1.2);
  g_to->SetMarkerStyle(kFullCircle);
  g_to->SetMarkerColor(kBlack);
  g_to->SetMarkerSize(1);
  //g_to->GetXaxis()->SetTimeDisplay(1);
  //g_to->GetXaxis()->SetTimeFormat("%H:%M");
  //g_to->GetXaxis()->SetTimeOffset(0,"gmt");
  g_to->SetMaximum((float)nmax*1.05);

  g_en->Draw("P same");
  g_en->SetMarkerStyle(kFullCircle);
  g_en->SetMarkerColor(2);
  g_en->SetMarkerSize(1);

  g_nf->Draw("P same");
  g_nf->SetMarkerStyle(kFullCircle);
  g_nf->SetMarkerColor(3);
  g_nf->SetMarkerSize(1);

  g_ft->Draw("P same");
  g_ft->SetMarkerStyle(kFullCircle);
  g_ft->SetMarkerColor(4);
  g_ft->SetMarkerSize(1);

  g_tb->Draw("P same");
  g_tb->SetMarkerStyle(kFullCircle);
  g_tb->SetMarkerColor(5);
  g_tb->SetMarkerSize(1);

  g_un->Draw("P same");
  g_un->SetMarkerStyle(kFullCircle);
  g_un->SetMarkerColor(6);
  g_un->SetMarkerSize(1);

  TLegend *l = new TLegend(0.5, 0.73, 0.8, 0.88);
  l->AddEntry(g_to, "Timeout error", "LP");
  l->AddEntry(g_en, "Event number error", "LP");
  l->AddEntry(g_nf, "Near full error", "LP");
  l->AddEntry(g_ft, "FED trailer error", "LP");
  l->AddEntry(g_tb, "TBM error", "LP");
  l->AddEntry(g_un, "Unknown error", "LP");
  l->SetBorderSize(0);
  l->SetFillColor(0);
  l->Draw();
  
  c1->Print("ErrorRates.png");

  return 0;
}
