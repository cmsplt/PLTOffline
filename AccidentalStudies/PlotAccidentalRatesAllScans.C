////////////////////////////////////////////////////////////////////
//
//  PlotAccidentalRatesAllScans -- an enhanced version of
//  PlotAccidentalRates which plots a number of fills on the same
//  plot
//    Paul Lujan, December 6 2015
//
////////////////////////////////////////////////////////////////////

// To use: set nFiles to the total number of fills, then just specify
// the file names, labels, and whether you want the fit to be drawn for
// each fill in the arrays below.

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

// Magnet-on fills
const int nFiles = 8;
const char *fileNames[nFiles] = {
  "CombinedRates_4201_clean.txt",
  // "CombinedRates_4243_clean.txt",
  // "CombinedRates_4246_clean.txt",
  "CombinedRates_4266_AllScans_Central.txt",
  "CombinedRates_4381_clean.txt",
  "CombinedRates_MuScan_Central.txt",
  "CombinedRates_4444.txt",
  "CombinedRates_4467_clean.txt",
  "CombinedRates_4565_clean.txt",
  "CombinedRates_4569_clean.txt"
};
const char *fillLabels[nFiles] = {
  "Fill 4201 (Aug 13, 26b)",
  // "Fill 4243 (Aug 21, 303b)",
  // "Fill 4246 (Aug 21-22, #it{50ns}, 254b)",
  "Fill 4266 (VdM scan, Aug 25, 30b)",
  "Fill 4381 (Sep 17, 1021b)",
  "Fill 4435 (mu scan, Sep 28, 881b)",
  "Fill 4444 (Sep 30, 1453b)",
  "Fill 4467 (Oct 6-7, 1596b)",
  "Fill 4565 (Nov 2, 2232b)",
  "Fill 4569 (Nov 2, 2232b, firmware fix)"
};
const bool doAllFit = false;   // show fit to all points

// Use this to exclude a single fit from being drawn.
const bool doFit[nFiles] = { false, false, true, true, true, true, true, true };

// Magnet-off fills
// const int nFiles = 3;
// const char *fileNames[nFiles] = {
//  "CombinedRates_4208_clean.txt",
//  "CombinedRates_4341_clean.txt",
//  "CombinedRates_4545_clean.txt",
// };
// const char *fillLabels[nFiles] = {
//  "Fill 4208 (Aug 14, 74b)",
//  "Fill 4341 (Sep 09, 733b)",
//  "Fill 4545 (Oct 29, 2232b)",
// };
// const bool doAllFit = true;   // show fit to all points
// // Use this to exclude a single fit from being drawn.
// const bool doFit[nFiles] = { true, true, true };

std::vector<double> fastOrLumiAll;
std::vector<double> fastOrLumiErrAll;
std::vector<double> accidentalRateAll;
std::vector<double> accidentalRateErrAll;

TGraph *readCombinedFile(const char *fileName) {
  // Read input file.
  std::vector<double> fastOrLumi;
  std::vector<double> fastOrLumiErr;
  std::vector<double> accidentalRate;
  std::vector<double> accidentalRateErr;

  FILE *rfile = fopen(fileName, "r");
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file " << fileName << "!" << std::endl;
    return(NULL);
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig, tracksAll, tracksGood, nMeas;
  double totLumi;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d %d %d %d %lf", &tBegin, &tEnd, &nTrig, &tracksAll, &tracksGood, &nMeas, &totLumi);
    // Process the data.
    fastOrLumi.push_back(totLumi/(nMeas*nBunches));
    fastOrLumiErr.push_back(0); // not implemented yet
    double accrate = (double)(tracksAll-tracksGood)/tracksAll;
    accidentalRate.push_back(100.0*accrate);
    accidentalRateErr.push_back(100.0*sqrt(accrate*(1-accrate)/tracksAll));

    fastOrLumiAll.push_back(fastOrLumi.back());
    fastOrLumiErrAll.push_back(fastOrLumiErr.back());
    accidentalRateAll.push_back(accidentalRate.back());
    accidentalRateErrAll.push_back(accidentalRateErr.back());
  }
  fclose(rfile);

  TGraph *g = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(accidentalRate[0]),
			       &(fastOrLumiErr[0]), &(accidentalRateErr[0]));

  return g;
}

void PlotAccidentalRatesAllScans(void) {
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

  // Plot it all.

  TCanvas *c1 = new TCanvas("c1", "c1", 900, 600);
  gPad->SetRightMargin(0.4);
  TGraph* g[nFiles];
  TF1* f[nFiles];

  for (int i=0; i<nFiles; ++i) {
    g[i] = readCombinedFile(fileNames[i]);
  }

  TGraph *gall = new TGraphErrors(fastOrLumiAll.size(), &(fastOrLumiAll[0]), &(accidentalRateAll[0]),
			       &(fastOrLumiErrAll[0]), &(accidentalRateErrAll[0]));
  gall->Draw("APX");
  gall->SetTitle("Accidental rate vs. online luminosity");
  gall->GetXaxis()->SetTitle("Online per-bunch inst lumi (Hz/#mub)");
  gall->GetYaxis()->SetTitle("Measured accidental rate (%)");
  //gall->GetYaxis()->SetTitleOffset(1.4);
  gall->GetYaxis()->SetRangeUser(4.5, 12.0);
  if (nFiles > 8) gall->GetYaxis()->SetRangeUser(4.5, 22.0);

  TF1 *fall;
  if (doAllFit) {
    fall = new TF1("fall", "pol1");
    fall->SetLineColor(kBlack);
    fall->SetLineWidth(1);
    gall->Fit(fall);
  }
  
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
    if (doFit[i]) {
      sprintf(buf, "f%d", i);
      f[i] = new TF1(buf, "pol1");
      f[i]->SetLineColor(icolor);
      f[i]->SetLineWidth(1);
      g[i]->Fit(f[i], "", "", 0, 2.3);
    }
  }
  g[0]->Draw("P same");

  TLegend *l = new TLegend(0.61, 0.36, 0.99, 0.69);
  for (int i=0; i<nFiles; ++i) {
    l->AddEntry(g[i], fillLabels[i], "LP");
  }
  if (doAllFit)
    l->AddEntry(fall, "Fit to all points", "L");
  l->SetFillColor(0);
  l->SetBorderSize(0);
  l->Draw();

  if (doAllFit)
    std::cout << "Value of total fit at x=2 is " << fall->Eval(2.0) << std::endl;
  for (int i=0; i<nFiles; ++i) {
    if (doFit[i])
      std::cout << "Value of fit " << i << " at x=2 is " << f[i]->Eval(2.0) << std::endl;
  }

  // c1->Print("AccidentalRate_AllScans_Clean.png");
  // c1->Print("AccidentalRate_MagnetOff.png");
}
