////////////////////////////////////////////////////////////////////
//
//  PlotAccidentalRatesMasks -- a tool for plotting the accidental
//   rates for different mask sizes. Based on
//   PlotAccidentalRates2016 and cleaned up for the PLT paper.
//    Paul Lujan, April 10, 2022
//
////////////////////////////////////////////////////////////////////

// To use: set nFiles to the total number of fills, then just specify
// the file names, labels, and whether you want the fit to be drawn for
// each fill in the arrays below.

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

const int nFiles = 6;
const char *fileNames[nFiles] = {
  "CombinedRates_4892.txt",
  "CombinedRates_4892_28x41_32x46.txt",
  "CombinedRates_4892_28x41_30x43.txt",
  "CombinedRates_4892_24x36_28x42.txt",
  "CombinedRates_4892_24x36_26x38.txt",
  "CombinedRates_4892_20x30_24x36.txt",
};
const char *fillLabels[nFiles] = {
  "#splitline{28x41 / 34x50}{(2015 area)}", "28x41 / 32x46", "28x41 / 30x43", "24x36 / 28x42",
  "#splitline{24x36 / 26x38}{(selected for 2016)}", "20x30 / 24x36"
};
const int plotColors[nFiles] = {kRed, kCyan, kMagenta, kBlue, kBlack, kGreen};

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

void PlotAccidentalRatesMasks(void) {
  for (int i=0; i<nFiles; ++i) {
    doFit[i] = true;
    xmax[i] = 4.0;
  }

  // style from PLTU
  gROOT->SetStyle("Plain");                  
  gStyle->SetPalette(1);
  gStyle->SetPadLeftMargin(0.07);
  gStyle->SetPadRightMargin(0.17);
  gStyle->SetPadTopMargin(0.04);
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
  gall->SetTitle("");
  gall->GetXaxis()->SetTitle("Uncorrected fast-or SBIL (Hz/#mub)");
  gall->GetYaxis()->SetTitle("Measured accidental rate (%)");
  gall->GetYaxis()->SetTitleOffset(0.8);
  gall->GetYaxis()->SetRangeUser(4.5, 12);
  gall->GetXaxis()->SetRangeUser(2, 3.2);

  for (int i=0; i<nFiles; ++i) {
    g[i]->Draw("P same");
    g[i]->SetMarkerStyle(kFullCircle);
    int icolor = plotColors[i];
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

  TLegend *l = new TLegend(0.61, 0.21, 0.99, 0.84);
  l->AddEntry("", "Central / outer plane area", "");
  for (int i=0; i<nFiles; ++i) {
    l->AddEntry(g[i], fillLabels[i], "LP");
  }

  l->SetFillColor(0);
  l->SetBorderSize(0);
  l->Draw();

  TText *t1 = new TText(0, 0, "CMS");
  t1->SetNDC();
  t1->SetX(0.10);
  t1->SetY(0.90);
  t1->SetTextFont(61);
  t1->SetTextSize(0.05);
  t1->Draw();
  TText *t2 = new TText(0, 0, "Preliminary");
  t2->SetNDC();
  t2->SetX(0.175);
  t2->SetY(0.90);
  t2->SetTextFont(52);
  t2->SetTextSize(0.05);
  t2->Draw();
  TText *t3 = new TText(0, 0, "2016");
  t3->SetNDC();
  t3->SetX(0.10);
  t3->SetY(0.84);
  t3->SetTextSize(0.05);
  t3->Draw();

  for (int i=0; i<nFiles; ++i) {
    if (doFit[i])
      std::cout << "Value of fit " << i << " at x=3 is " << f[i]->Eval(3.0) << std::endl;
  }

  c1->Print("AccidentalRate_MaskCalibration.png");
  c1->Print("AccidentalRate_MaskCalibration.pdf");
}
