////////////////////////////////////////////////////////////////////
//
//  PlotAccidentalRates -- a script to plot the accidental rates for a single Telescope (FED Channel)
//   given the combined file produced by ParseCondDBDataTele
//    Paul Lujan, November 10 2015
//    Joseph Heideman, March 11 2016
////////////////////////////////////////////////////////////////////

// This script only plots the rates for a single fill. If you want
// something more advanced, check out PlotAccidentalRatesAllScans.

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

void PlotAccidentalRatesTele(void) {
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

  FILE *rfile = fopen("CombinedRatesTele4555_Align4444.txt", "r");
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file!" << std::endl;
    return(1);
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig, tracksAllTele1, tracksGoodTele1, tracksAllTele2, tracksGoodTele2, tracksAllTele4, tracksGoodTele4, tracksAllTele5, tracksGoodTele5, tracksAllTele7, tracksGoodTele7, tracksAllTele8, tracksGoodTele8, tracksAllTele10, tracksGoodTele10, tracksAllTele11, tracksGoodTele11, tracksAllTele13, tracksGoodTele13, tracksAllTele14, tracksGoodTele14, tracksAllTele16, tracksGoodTele16, tracksAllTele17, tracksGoodTele17, tracksAllTele19, tracksGoodTele19, tracksAllTele20, tracksGoodTele20, nMeas;
  double totLumi;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
//    fscanf(rfile, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %lf", &tBegin, &tEnd, &nTrig, &tracksAllTele1, &tracksGoodTele1, &tracksAllTele2, &tracksGoodTele2, &tracksAllTele4, &tracksGoodTele4, &tracksAllTele5, &tracksGoodTele5, &tracksAllTele7, &tracksGoodTele7, &tracksAllTele8, &tracksGoodTele8, &tracksAllTele10, &tracksGoodTele10, &tracksAllTele11, &tracksGoodTele11, &tracksAllTele13, &tracksGoodTele13, &tracksAllTele14, &tracksGoodTele14, &tracksAllTele16, &tracksGoodTele16, &tracksAllTele17, &tracksGoodTele17, &tracksAllTele19, &tracksGoodTele19, &tracksAllTele20, &tracksGoodTele20, &nMeas, &totLumi);
    fscanf(rfile, "%d %d %d %d %d %d %d %d %d %d", &tBegin, &tEnd, &nTrig, &tracksAllTele1, &tracksGoodTele1, &tracksAllTele2, &tracksGoodTele2, &tracksAllTele4, &tracksGoodTele4, &tracksAllTele5);

	fscanf(rfile, "%d %d %d %d %d %d %d %d %d %d", &tracksGoodTele5, &tracksAllTele7, &tracksGoodTele7, &tracksAllTele8, &tracksGoodTele8, &tracksAllTele10, &tracksGoodTele10, &tracksAllTele11, &tracksGoodTele11, &tracksAllTele13); 

        fscanf(rfile, "%d %d %d %d %d %d %d %d %d %d", &tracksGoodTele13, &tracksAllTele14, &tracksGoodTele14, &tracksAllTele16, &tracksGoodTele16, &tracksAllTele17, &tracksGoodTele17, &tracksAllTele19, &tracksGoodTele19, &tracksAllTele20);

	fscanf(rfile, "%d %d %lf\n", &tracksGoodTele20, &nMeas, &totLumi);

    // Process the data.
//
//
//CHOOSE WHICH TELESCOPE BY CHANGING CHANNEL # AT THE END OF   tracksAllTele   AND   tracksGoodTele
//
// 
   fastOrLumi.push_back(totLumi/(nMeas*nBunches));
    trackLumiAll.push_back(1000.0*(double)tracksAllTele11/(tEnd-tBegin));
    trackLumiGood.push_back(1000.0*(double)tracksGoodTele11/(tEnd-tBegin));
    int nAcc = tracksAllTele11-tracksGoodTele11;
    double accrate = (double)(tracksAllTele11-tracksGoodTele11)/tracksAllTele11;
    accidentalRate.push_back(100.0*accrate);
    accidentalRateErr.push_back(100.0*sqrt(accrate*(1-accrate)/tracksAllTele11));
    accidentalRateHz.push_back((double)nAcc*1000.0/(tEnd-tBegin));
    accidentalRateHzErr.push_back(sqrt(nAcc)*1000.0/(tEnd-tBegin));
    trackLumiErr.push_back(0); // not implemented yet
  }
  fclose(rfile);

  // Plot 
  TGraph *g2 = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(accidentalRate[0]),
				&(trackLumiErr[0]), &(accidentalRateErr[0]));



  TCanvas *c2 = new TCanvas("c2", "c2", 600, 600);
  g2->Draw("AP");
  g2->SetTitle("Accidental rate vs. fast-or lumi");
  g2->GetXaxis()->SetTitle("Average fast-or lumi per bunch");
  g2->GetYaxis()->SetTitle("Accidental rate in pixel data (% of tracks)");
  g2->GetYaxis()->SetTitleOffset(1.4);
  g2->SetMarkerStyle(kFullCircle);
  g2->SetMarkerColor(kBlue);
  g2->SetLineColor(kBlue);
  g2->SetMarkerSize(1);
  TF1 *f2 = new TF1("f2", "pol1");
  f2->SetLineColor(kBlue);
  f2->SetLineWidth(1);
  g2->Fit(f2);
  
  //c2->Print("AccidentalRate_MuScan1_Central_Final.png");
}
