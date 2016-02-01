////////////////////////////////////////////////////////////////////
//
//  PlotPixelMissRateComparison -- compare the miss rate calculated
//   from pixel data to the transparent buffer rate
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

const char *transparentFile = "CombinedRates_4444.txt";
const char *pixelFile = "CombinedRatesPixel_4444.txt";

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
  }
  fclose(rfile);
  
  // Plot it all.
  
  TGraph *g = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(missRate[0]),
			       &(fastOrLumiErr[0]), &(missRateErr[0]));
  return g;
}
  

void PlotPixelMissRateComparison(void) {
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
  TGraph* gTr = ReadCombinedFile(transparentFile);

  // read the pixel file. no handy function for this one alas
  // because we've got this mess of algos
  std::vector<double> fastOrLumi;
  std::vector<double> fastOrLumiErr;
  std::vector<double> missRate[4];
  std::vector<double> missRateErr[4];

  FILE *rfile = fopen(pixelFile, "r");
  int nBunches;
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file "
	      << fileName << "!" << std::endl;
    return(NULL);
  }
  int nsteps, tBegin, tEnd, nTrig, nTriples[4], nTriplesMissed[4], nMeas;
  double totLumi;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d", &tBegin, &tEnd, &nTrig);
    for (int iAlg=0; iAlg<4; iAlg++)
      fscanf(rfile, "%d %d", &(nTriples[iAlg]), &(nTriplesMissed[iAlg]));
    fscanf(rfile, "%d %lf", &nMeas, &totLumi);
    // Process the data.
    fastOrLumi.push_back(totLumi/(nMeas*nBunches));
    for (int iAlg=0; iAlg<4; ++iAlg) {
      double missrate = (double)(nTriplesMissed[iAlg])/nTriples[iAlg];
      missRate[iAlg].push_back(100.0*missrate);
      missRateErr[iAlg].push_back(100.0*sqrt(missrate*(1-missrate)/nTriples[iAlg]));
    }
    fastOrLumiErr.push_back(0); // not implemented yet
  }
  fclose(rfile);
  
  // Plot it all.
  TGraph *gPix[4];
  for (int iAlg=0; iAlg<4; ++iAlg) {
    gPix[iAlg] = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(missRate[iAlg][0]),
				  &(fastOrLumiErr[0]), &(missRateErr[iAlg][0]));
  }

  gTr->Draw("AP");
  gTr->SetTitle("Missing triplet rate vs. fast-or lumi");
  gTr->GetXaxis()->SetTitle("Average fast-or lumi per bunch");
  gTr->GetYaxis()->SetTitle("Missing triplet rate (%)");
  gTr->GetYaxis()->SetTitleOffset(1.4);
  gTr->GetYaxis()->SetRangeUser(2.5, 22);
  
  for (int i=0; i<4; ++i) {
    gPix[i]->Draw("P same");
    gPix[i]->SetMarkerStyle(kFullCircle);
    gPix[i]->SetMarkerColor(i+2);
    gPix[i]->SetLineColor(i+2);
    gPix[i]->SetMarkerSize(1);
  }

  TLegend *l = new TLegend(0.61, 0.36, 0.99, 0.69);
  l->AddEntry(gTr, "Transparent buffer", "LP");
  l->AddEntry(gPix[0], "Pixel data, algo 1", "LP");
  l->AddEntry(gPix[3], "Pixel data, algo 2", "LP");
  l->AddEntry(gPix[1], "Pixel data, algo 3", "LP");
  l->AddEntry(gPix[2], "Pixel data, algo 4", "LP");
  l->SetFillColor(0);
  l->SetBorderSize(0);
  l->Draw();

  c1->Print("MissRate_PixelComparison.png");
}
