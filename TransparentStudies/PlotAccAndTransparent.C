////////////////////////////////////////////////////////////////////
//
//  PlotAccAndTransparent -- a script to plot the accidental rate
//   and missing triplet rate for a single fill together on the same
//   plot. This is currently configured for Fill 4243, for which the
//   missing triplet rate may be slightly biased due to the use of the
//   fast-or coincidence trigger.
//    Paul Lujan, December 5 2015
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

#include "TGraph.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TAxis.h"
#include "TGraphErrors.h"

const int nFiles = 2;
const char *fileNames[nFiles] = {
  "CombinedRates_4243.txt",
  "../AccidentalStudies/CombinedRates_4243_clean.txt"
};
const char *fillLabels[nFiles] = {
  "Missing triplet rate",
  "Accidental rate"
};

std::vector<double> fastOrLumiAll;
std::vector<double> fastOrLumiErrAll;
std::vector<double> missRateAll;
std::vector<double> missRateErrAll;

TGraph *ReadCombinedFile(const char *fileName, int iFile) {
  // Read input file.
  std::vector<double> fastOrLumi;
  std::vector<double> fastOrLumiErr;
  std::vector<double> missRate;
  std::vector<double> missRateErr;

  FILE *rfile;
  rfile = fopen(fileName, "r");
  int nBunches;
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file "
	      << fileName << "!" << std::endl;
    return(NULL);
  }
  int nsteps, tBegin, tEnd, nTrig, nTriples, nTriplesMissed, nMeas;
  double totLumi;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d %d %d %d %lf", &tBegin, &tEnd, &nTrig, &nTriples, &nTriplesMissed, &nMeas, &totLumi);
    // Process the data.
    fastOrLumi.push_back(totLumi/(nMeas*nBunches));
    double missrate = (double)(nTriplesMissed)/nTriples;
    // sorry the names are so very wrong in the accidental case
    if (iFile==1) missrate = 1 - missrate;
    missRate.push_back(100.0*missrate);
    missRateErr.push_back(100.0*sqrt(missrate*(1-missrate)/nTriples));
    fastOrLumiErr.push_back(0); // not implemented yet

    fastOrLumiAll.push_back(fastOrLumi.back());
    fastOrLumiErrAll.push_back(fastOrLumiErr.back());
    missRateAll.push_back(missRate.back());
    missRateErrAll.push_back(missRateErr.back());
  }
  fclose(rfile);
  
  // Plot it all.
  
  TGraph *g = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(missRate[0]),
			       &(fastOrLumiErr[0]), &(missRateErr[0]));

  return g;
}
  

void PlotAccAndTransparent(void) {
  // style from PLTU
  gROOT->SetStyle("Plain");                  
  gStyle->SetPalette(1);
  gStyle->SetPadLeftMargin(0.17);
  gStyle->SetPadRightMargin(0.17);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleX(0.1);
  gStyle->SetTitleY(1.0);
  gStyle->SetTitleH(0.09);
  gStyle->SetTitleW(0.7);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetLegendBorderSize(0);
  //gStyle->SetOptFit(1111);

  TCanvas *c1 = new TCanvas("c1", "c1", 900, 600);
  gPad->SetRightMargin(0.4);
  TGraph* g[nFiles];
  TF1* f[nFiles];

  for (int i=0; i<nFiles; ++i) {
    g[i] = ReadCombinedFile(fileNames[i], i);
  }

  TGraph *gall = new TGraphErrors(fastOrLumiAll.size(), &(fastOrLumiAll[0]), &(missRateAll[0]),
			       &(fastOrLumiErrAll[0]), &(missRateErrAll[0]));
  gall->Draw("AP");
  gall->SetTitle("Corrections, Fill 4243");
  gall->GetXaxis()->SetTitle("Average fast-or lumi per bunch");
  gall->GetYaxis()->SetTitle("Correction [%]");
  gall->GetYaxis()->SetTitleOffset(1.4);
  gall->GetYaxis()->SetRangeUser(13, 23);
  
  for (int i=0; i<nFiles; ++i) {
    g[i]->Draw("P same");
    g[i]->SetMarkerStyle(kFullCircle);
    g[i]->SetMarkerColor(i+2);
    g[i]->SetLineColor(i+2);
    g[i]->SetMarkerSize(1);

    char buf[32];
    sprintf(buf, "f%d", i);
    f[i] = new TF1(buf, "pol1");
    f[i]->SetLineColor(i+2);
    f[i]->SetLineWidth(1);
    g[i]->Fit(f[i], "", "", 0, 2.3);
  }
  g[0]->Draw("P same");

  TLegend *l = new TLegend(0.61, 0.46, 0.99, 0.69);
  for (int i=0; i<nFiles; ++i) {
    l->AddEntry(g[i], fillLabels[i], "LP");
    if (i==0) l->AddEntry(g[i], "#splitline{Warning: missing triplet rate is biased}{slightly low due to fast-or trigger}", "");
  }
  l->SetFillColor(0);
  l->SetBorderSize(0);
  l->Draw();

  c1->Print("Rates_Fill4243.png");
}
