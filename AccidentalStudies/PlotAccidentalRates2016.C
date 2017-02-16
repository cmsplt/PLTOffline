////////////////////////////////////////////////////////////////////
//
//  PlotAccidentalRates2016 -- an updated version of PlotAccidentalRatesAllScans
//    for 2016 fills
//    Paul Lujan, May 9 2016
//
////////////////////////////////////////////////////////////////////

// To use: set nFiles to the total number of fills, then just specify
// the file names, labels, and whether you want the fit to be drawn for
// each fill in the arrays below.

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

const int nFiles = 7;
const char *fileNames[nFiles] = {
  //"CombinedRates_4569_clean.txt",
  //"CombinedRates_4879_1hr_clean.txt",
  //"CombinedRates_4892.txt",
  //"CombinedRates_4895_clean.txt",
  "CombinedRates_4925_clean.txt",
  "CombinedRates_4930.txt",
  "CombinedRates_4945_VdMCombined_Central.txt",
  "CombinedRates_4958.txt",
  "CombinedRates_4985.txt",
  "CombinedRates_5005_MuScan.txt",
  "CombinedRates_5013.txt"
};
const char *fillLabels[nFiles] = {
  //"Fill 4569 (Nov 2 2015, 2232b)",
  //"Fill 4879 (Apr 28, 49b)",
  //"Fill 4892 (May 7, 74b)",
  //"Fill 4895 (May 8, 301b)",
  "Fill 4925 (May 14, 589b, new mask)",
  "Fill 4930 (May 16, 770b)",
  "Fill 4945 (May 18, 32b, first VdM scan)",
  "Fill 4958 (May 28, 1453b)",
  "Fill 4985 (June 3, 2028b)",
  "Fill 5005 (June 11, 590b, mu scan)",
  "Fill 5013 (June 12, 2028b)",
};
const bool doAllFit = false;   // show fit to all points

// Use this to exclude a single fit from being drawn.
bool doFit[nFiles];
float xmax[nFiles];

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

void PlotAccidentalRates2016(void) {
  for (int i=0; i<nFiles; ++i) {
    doFit[i] = true;
    xmax[i] = 4.0;
  }
  doFit[2] = false;
  xmax[2] = 0.5;

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

  fastOrLumiAll.push_back(0.0);
  accidentalRateAll.push_back(0.0);
  fastOrLumiErrAll.push_back(0.0);
  accidentalRateErrAll.push_back(0.0);
  
  TGraph *gall = new TGraphErrors(fastOrLumiAll.size(), &(fastOrLumiAll[0]), &(accidentalRateAll[0]),
			       &(fastOrLumiErrAll[0]), &(accidentalRateErrAll[0]));
  gall->Draw("APX");
  gall->SetTitle("Accidental rate vs. online luminosity");
  gall->GetXaxis()->SetTitle("Online per-bunch inst lumi (Hz/#mub)");
  gall->GetYaxis()->SetTitle("Measured accidental rate (%)");
  //gall->GetYaxis()->SetTitleOffset(1.4);
  gall->GetYaxis()->SetRangeUser(1.0, 8.0);

  TF1 *fall;
  if (doAllFit) {
    fall = new TF1("fall", "pol1");
    fall->SetLineColor(kBlack);
    fall->SetLineWidth(1);
    gall->Fit(fall);
  }

//   f2015 = new TF1("f2015", "4.76 + 2.74*x", 0, 4.0);
//   f2015->SetLineColor(kBlack);
//   f2015->Draw("same L");
  
  for (int i=0; i<nFiles; ++i) {
    g[i]->Draw("P same");
    g[i]->SetMarkerStyle(kFullCircle);
    int icolor = i+2;
    if (icolor >= 5) icolor++;
    if (icolor >= 6) icolor++;
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
      g[i]->Fit(f[i], "", "", 0, xmax[i]);
    }
  }
  g[0]->Draw("P same");

  TLegend *l = new TLegend(0.61, 0.36, 0.99, 0.69);
  for (int i=0; i<nFiles; ++i) {
    l->AddEntry(g[i], fillLabels[i], "LP");
  }
  if (doAllFit)
    l->AddEntry(fall, "Fit to all points", "L");
  // l->AddEntry(f2015, "2015 accidental rate", "L");
  l->SetFillColor(0);
  l->SetBorderSize(0);
  l->Draw();

  if (doAllFit)
    std::cout << "Value of total fit at x=3 is " << fall->Eval(3.0) << std::endl;
  for (int i=0; i<nFiles; ++i) {
    if (doFit[i])
      std::cout << "Value of fit " << i << " at x=3 is " << f[i]->Eval(3.0) << std::endl;
  }

  c1->Print("AccidentalRate_2016_New.png");
}
