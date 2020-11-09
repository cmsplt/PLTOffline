////////////////////////////////////////////////////////////////////
//
// PlotTrackRatesVdM -- plots the raw track rates (all tracks, good tracks, and bad tracks) for a single VdM
// scan. Based (more or less) on PlotTrackLumiVdM, but way simpler.
//
// Paul Lujan, October 25, 2020
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <time.h>
#include <math.h>
#include "TROOT.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLegend.h"

const int nPixelChannels = 13;

// Call with three arguments: the name of the file you want to plot, the name of the output file to generate,
// and the label to include in the plot title. Also defaults are provided at the end.

void PlotTrackRatesVdM(std::string scanFileName, std::string outFileName, std::string plotLabel) {
  gROOT->SetStyle("Plain");
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadLeftMargin(0.12);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleX(0.1);
  gStyle->SetTitleY(1.0);
  gStyle->SetTitleH(0.09);
  gStyle->SetTitleW(0.7);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetLegendBorderSize(0);
  //gStyle->SetOptFit(1111);

  // Read in the luminosity information.

  std::vector<double> stepNum;
  std::vector<double> allTrackRate;
  std::vector<double> goodTrackRate;
  std::vector<double> badTrackRate;
  std::vector<double> badTrackPercent;

  std::ifstream scanFile(scanFileName);
  if (!scanFile.is_open()) {
    std::cerr << "Couldn't open track luminosity file!" << std::endl;
    return;
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig, nFilledTrig;
  float tracksAll, tracksGood, nEmpty, nFull;

  std::string line;
  // Get header line.
  std::getline(scanFile, line);
  std::stringstream ss(line);
  ss >> nsteps >> nBunches;
  // Go through the rest of the lines.
  int nStep = 0;
  while (1) {
    std::getline(scanFile, line);
    if (scanFile.eof()) break;
    if (line.empty()) continue; // skip blank lines
    if (line.at(0) == '#') continue; // skip comment lines

    // Break into fields. (Ignore the per-channel data at the end, if it's there.)
    std::stringstream ss(line);
    ss >> tBegin >> tEnd >> nTrig >> tracksAll >> tracksGood >> nFilledTrig >> nEmpty >> nFull;
    
    nStep++;
    
    stepNum.push_back(nStep);
    allTrackRate.push_back((float)tracksAll/nTrig);
    goodTrackRate.push_back((float)tracksGood/nTrig);
    badTrackRate.push_back((float)(tracksAll-tracksGood)/nTrig);
    badTrackPercent.push_back(100*(1-(float)tracksGood/tracksAll));
  }
  scanFile.close();

  // Plot the results.
  TGraph *g_all = new TGraph(stepNum.size(), stepNum.data(), allTrackRate.data());
  TGraph *g_good = new TGraph(stepNum.size(), stepNum.data(), goodTrackRate.data());
  TGraph *g_bad = new TGraph(stepNum.size(), stepNum.data(), badTrackRate.data());

  TCanvas *c1 = new TCanvas("c1", "c1", 1200, 600);
  c1->Divide(2,1);
  c1->cd(1);
  g_all->Draw("AP");
  std::string plotTitle = "Track rates, "+plotLabel;
  g_all->SetTitle(plotTitle.c_str());
  g_all->GetXaxis()->SetTitle("Step number");
  g_all->GetYaxis()->SetTitle("Track rate (tracks per trigger)");
  g_all->GetYaxis()->SetTitleOffset(1.45);
  g_all->SetMarkerStyle(kFullCircle);
  g_all->SetMarkerColor(kBlack);
  g_all->SetMarkerSize(1);

  g_good->Draw("P same");
  g_good->SetMarkerStyle(kFullCircle);
  g_good->SetMarkerColor(kBlue);
  g_good->SetMarkerSize(1);

  g_bad->Draw("P same");
  g_bad->SetMarkerStyle(kFullCircle);
  g_bad->SetMarkerColor(kRed);
  g_bad->SetMarkerSize(1);

  gPad->SetLogy();
  g_all->SetMinimum(1e-6);
 
  TLegend *l1 = new TLegend(0.7, 0.68, 0.9, 0.78);
  l1->AddEntry(g_all, "All tracks", "P");
  l1->AddEntry(g_good, "Good tracks", "P");
  l1->AddEntry(g_bad, "Bad tracks", "P");
  l1->Draw();

  c1->cd(2);
  TGraph *g_pct = new TGraph(stepNum.size(), stepNum.data(), badTrackPercent.data());
  g_pct->Draw("AP");
  plotTitle = "Bad track fraction, "+plotLabel;
  g_pct->SetTitle(plotTitle.c_str());
  g_pct->GetXaxis()->SetTitle("Step number");
  g_pct->GetYaxis()->SetTitle("Percentage of bad tracks");
  g_pct->GetYaxis()->SetTitleOffset(1.45);
  g_pct->SetMarkerStyle(kFullCircle);
  g_pct->SetMarkerColor(kRed);
  g_pct->SetMarkerSize(1);

  c1->Print(outFileName.c_str());
}

// If called with no arguments, then do the X and Y scans for fill 4954.
void PlotTrackRatesVdM() {
  PlotTrackRatesVdM("TrackLumiZC_4954_X1.txt", "TrackRates_VdM_4954_X1.png", "fill 4954, scan X1");
  PlotTrackRatesVdM("TrackLumiZC_4954_Y1.txt", "TrackRates_VdM_4954_Y1.png", "fill 4954, scan Y1");
}
