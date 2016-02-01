////////////////////////////////////////////////////////////////////
//
//  PlotTransparentAllScans -- a script to plot the missing triplet
//   rate from the transparent buffer data for a variety of fills.
//   Based on (and shares much code with) PlotAccidentalRatesAllScans.
//    Paul Lujan, December 1 2015
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

const int nFiles = 5;
const char *fileNames[nFiles] = {
  "CombinedRates_MuScan_Central.txt",
  "CombinedRates_4467_clean.txt",
  //"CombinedRates_4337.txt",
  "CombinedRates_4381_clean.txt",
  "CombinedRates_4444_clean.txt",
  //"CombinedRates_4545.txt",
  "CombinedRates_4565_clean.txt",
};
const char *fillLabels[nFiles] = {
  "Fill 4435 (mu scan, Sep 28, 881b)",
  "Fill 4467 (Oct 6-7, B=3.8, 1596b)",
  //"Fill 4337 (Sep 9, B=0, 447b)",
  "Fill 4381 (Sep 17, B=3.8, 1021b)",
  "Fill 4444 (Sep 30, B=3.8, 1453b)",
  //"Fill 4545 (Oct 29, B=0, 2232b)",
  "Fill 4565 (Nov 2, B=3.8, 2232b)"
};

std::vector<double> fastOrLumiAll;
std::vector<double> fastOrLumiErrAll;
std::vector<double> missRateAll;
std::vector<double> missRateErrAll;

TGraph *ReadCombinedFile(const char *fileName) {
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
  

void PlotTransparentAllScans(void) {
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
    g[i] = ReadCombinedFile(fileNames[i]);
  }

  TGraph *gall = new TGraphErrors(fastOrLumiAll.size(), &(fastOrLumiAll[0]), &(missRateAll[0]),
			       &(fastOrLumiErrAll[0]), &(missRateErrAll[0]));
  gall->Draw("AP");
  gall->SetTitle("Missing triplet rate vs. online luminosity");
  gall->GetXaxis()->SetTitle("Online per-bunch inst lumi (Hz/#mub)");
  gall->GetYaxis()->SetTitle("Measured missing triplet rate (%)");
  //gall->GetYaxis()->SetTitleOffset(1.4);
  gall->GetYaxis()->SetRangeUser(2.5, 16.5);
  TF1 *f2 = new TF1("f2", "pol1");
  f2->SetLineColor(kBlack);
  f2->SetLineWidth(1);
  gall->Fit(f2);
  
  for (int i=0; i<nFiles; ++i) {
    g[i]->Draw("P same");
    g[i]->SetMarkerStyle(kFullCircle);
    int icolor = i+2;
    if (icolor >= 5) icolor++;
    if (icolor >= 10) icolor++;
    g[i]->SetMarkerColor(icolor);
    g[i]->SetLineColor(icolor);
    g[i]->SetMarkerSize(1);

    char buf[32];
    sprintf(buf, "f%d", i);
    f[i] = new TF1(buf, "pol1");
    f[i]->SetLineColor(icolor);
    f[i]->SetLineWidth(1);
    g[i]->Fit(f[i], "", "", 0, 2.3);
  }
  g[0]->Draw("P same");

  TLegend *l = new TLegend(0.61, 0.36, 0.99, 0.69);
  for (int i=0; i<nFiles; ++i) {
    l->AddEntry(g[i], fillLabels[i], "LP");
  }
  l->AddEntry(f2, "Fit to all points", "L");
  l->SetFillColor(0);
  l->SetBorderSize(0);
  l->Draw();

  std::cout << "Value of total fit at x=2 is " << f2->Eval(2.0) << std::endl;
  for (int i=0; i<nFiles; ++i) {
    std::cout << "Value of fit " << i << " at x=2 is " << f[i]->Eval(2.0) << std::endl;
  }

  //c1->Print("MissRate_AllScans.png");
  c1->Print("MissRate_MagnetOn_Final.png");
}
