////////////////////////////////////////////////////////////////////
//
//  PlotAccidentalRates2016Single -- a script to plot the accidental
//  rates for a single fill given the combined file produced by
//  ParseCondDBData
//    Paul Lujan, June 15 2016
//
////////////////////////////////////////////////////////////////////

// This script is similar to the 2015 script PlotAccidentalRates.C but
// has a few updates. Specifically, some of the less useful plots have
// been removed and some more useful plots have been added; the plots
// made are now:
//  good track rate vs. online lumi
//  accidental rate vs. online lumi
//  ratio of track lumi/online lumi vs. online lumi
// Currently this plots for fill 5013 and uses the raw track rate
// but you can also try to use the zero-counting rate by looking for
// the ZC lines below.

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

void PlotAccidentalRates2016Single(void) {
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
  std::vector<double> ratio;

  FILE *rfile = fopen("CombinedRates_5013.txt", "r");
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
    double allTrackRate = (double)tracksAll/nTrig;
    // double allTrackRate = -log(1.0-(double)tracksAll/nTrig);
    //trackLumiAll.push_back(allTrackRate);
    //double goodTrackRate = -log(1.0-(double)tracksGood/nTrig); // ZC
    double goodTrackRate = (double)tracksGood/nTrig;
    trackLumiGood.push_back(goodTrackRate);
    int nAcc = tracksAll-tracksGood;
    double accrate = (double)(tracksAll-tracksGood)/tracksAll;
    accidentalRate.push_back(100.0*accrate);
    accidentalRateErr.push_back(100.0*sqrt(accrate*(1-accrate)/tracksAll));
    accidentalRateHz.push_back((double)nAcc*1000.0/(tEnd-tBegin));
    accidentalRateHzErr.push_back(sqrt(nAcc)*1000.0/(tEnd-tBegin));
    trackLumiErr.push_back(0); // not implemented yet
    ratio.push_back((totLumi/(nMeas*nBunches))/goodTrackRate);
  }
  fclose(rfile);

  // Plot it all.

  TGraph *g1 = new TGraph(nsteps, &(fastOrLumi[0]), &(trackLumiGood[0]));
  TGraph *g2 = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(accidentalRate[0]),
				&(trackLumiErr[0]), &(accidentalRateErr[0]));
  TGraph *g3 = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(ratio[0]));

  TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
  g1->Draw("AP");
  g1->SetTitle("Good track rate vs. fast-or rate, Fill 5013");
  g1->GetXaxis()->SetTitle("Avg. corrected fast-or lumi per bunch (Hz/#mub)");
  g1->GetYaxis()->SetTitle("Rate of good tracks in Slink data (tracks/trigger)");
  g1->GetYaxis()->SetTitleOffset(1.7);
  g1->SetMarkerStyle(kFullCircle);
  g1->SetMarkerColor(kBlue);
  g1->SetMarkerSize(1);
  TF1 *f1 = new TF1("f1", "pol1");
  f1->FixParameter(0, 0);
  f1->SetLineColor(kBlue);
  f1->SetLineWidth(1);
  g1->Fit(f1);

  TCanvas *c2 = new TCanvas("c2", "c2", 600, 600);
  g2->Draw("AP");
  g2->SetTitle("Accidental rate vs. online luminosity");
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

  TCanvas *c3 = new TCanvas("c3", "c3", 600, 600);
  g3->Draw("AP");
  g3->SetTitle("Fast-or/track ratio vs. lumi");
  g3->GetXaxis()->SetTitle("Avg. corrected fast-or lumi per bunch (Hz/#mub)");
  g3->GetYaxis()->SetTitle("Ratio of fast-or rate to good track rate");
  g3->GetYaxis()->SetTitleOffset(1.4);
  g3->SetMarkerStyle(kFullCircle);
  g3->SetMarkerColor(kBlue);
  g3->SetLineColor(kBlue);
  g3->SetMarkerSize(1);
  TF1 *f3 = new TF1("f3", "pol1");
  f3->SetLineColor(kBlue);
  f3->SetLineWidth(1);
  g3->Fit(f3);

  c1->Print("TracksVsFastOr5013.png");
  //c2->Print("AccidentalRate5013.png");
  c3->Print("TrackRatio5013.png");
}
