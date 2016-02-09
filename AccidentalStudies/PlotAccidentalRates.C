////////////////////////////////////////////////////////////////////
//
//  PlotAccidentalRates -- a script to plot the accidental rates
//   given the combined file produced by ParseCondDBData
//    Paul Lujan, November 10 2015
//
////////////////////////////////////////////////////////////////////

// This script only plots the rates for a single fill (currently the
// VdM scans from fill 4266). If you want something more advanced,
// check out PlotAccidentalRatesAllScans.

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

void PlotAccidentalRates(void) {
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

  // Read input file.
  std::vector<double> fastOrLumi;
  std::vector<double> trackLumiAll;
  std::vector<double> trackLumiGood;
  std::vector<double> accidentalRate;
  std::vector<double> trackLumiErr;
  std::vector<double> accidentalRateErr;
  std::vector<double> accidentalRateHz;
  std::vector<double> accidentalRateHzErr;

  FILE *rfile = fopen("CombinedRates_4266_AllScans_Central.txt", "r");
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file!" << std::endl;
    return(1);
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig, tracksAll, tracksGood, nMeas;
  double totLumi;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d %d %d %d %lf", &tBegin, &tEnd, &nTrig, &tracksAll, &tracksGood, &nMeas, &totLumi);
    // Process the data.
    fastOrLumi.push_back(totLumi/(nMeas*nBunches));
    trackLumiAll.push_back(1000.0*(double)tracksAll/(tEnd-tBegin));
    trackLumiGood.push_back(1000.0*(double)tracksGood/(tEnd-tBegin));
    int nAcc = tracksAll-tracksGood;
    double accrate = (double)(tracksAll-tracksGood)/tracksAll;
    accidentalRate.push_back(100.0*accrate);
    accidentalRateErr.push_back(100.0*sqrt(accrate*(1-accrate)/tracksAll));
    accidentalRateHz.push_back((double)nAcc*1000.0/(tEnd-tBegin));
    accidentalRateHzErr.push_back(sqrt(nAcc)*1000.0/(tEnd-tBegin));
    trackLumiErr.push_back(0); // not implemented yet
  }
  fclose(rfile);

  // Plot it all.

  TGraph *g1 = new TGraph(nsteps, &(trackLumiAll[0]), &(fastOrLumi[0]));
  TGraph *g2 = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(accidentalRate[0]),
				&(trackLumiErr[0]), &(accidentalRateErr[0]));
  TGraph *g3 = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(accidentalRateHz[0]),
				&(trackLumiErr[0]), &(accidentalRateHzErr[0]));

  // TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
  // g1->Draw("AP");
  // g1->SetTitle("Fast-or vs. pixel track lumi");
  // g1->GetXaxis()->SetTitle("Pixel track lumi (rate of all tracks, Hz)");
  // g1->GetYaxis()->SetTitle("Average fast-or lumi per bunch");
  // g1->GetYaxis()->SetTitleOffset(1.7);
  // g1->SetMarkerStyle(kFullCircle);
  // g1->SetMarkerColor(kBlue);
  // g1->SetMarkerSize(1);
  // TF1 *f1 = new TF1("f1", "pol1");
  // f1->FixParameter(0, 0);
  // f1->SetLineColor(kBlue);
  // f1->SetLineWidth(1);
  // g1->Fit(f1);

  TCanvas *c2 = new TCanvas("c2", "c2", 600, 600);
  g2->Draw("AP");
  g2->SetTitle("Accidental rate vs. online luminosity, VdM scans");
  g2->GetXaxis()->SetTitle("Online per-bunch inst. lumi (Hz/#mub)");
  g2->GetYaxis()->SetTitle("Measured accidental rate (%)");
  g2->GetYaxis()->SetTitleOffset(1.4);
  g2->SetMarkerStyle(kFullCircle);
  g2->SetMarkerColor(kBlue);
  g2->SetLineColor(kBlue);
  g2->SetMarkerSize(1);
  TF1 *f2 = new TF1("f2", "pol0"); // pol0 for VdM, pol1 for regular fills
  f2->SetLineColor(kBlue);
  f2->SetLineWidth(1);
  g2->Fit(f2);

  // TCanvas *c3 = new TCanvas("c3", "c3", 600, 600);
  // g3->Draw("AP");
  // g3->SetTitle("Accidental rate vs. fast-or lumi");
  // g3->GetXaxis()->SetTitle("Average fast-or lumi per bunch");
  // g3->GetYaxis()->SetTitle("Accidental rate in pixel data (Hz)");
  // g3->GetYaxis()->SetTitleOffset(1.4);
  // g3->SetMarkerStyle(kFullCircle);
  // g3->SetMarkerColor(kBlue);
  // g3->SetLineColor(kBlue);
  // g3->SetMarkerSize(1);
  // TF1 *f3 = new TF1("f3", "pol2");
  // f3->SetLineColor(kBlue);
  // f3->SetLineWidth(1);
  // g3->Fit(f3);
  
  c2->Print("AccidentalRate_VdMScans_Central.png");
}
