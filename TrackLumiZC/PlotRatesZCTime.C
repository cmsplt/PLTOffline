////////////////////////////////////////////////////////////////////
//
// PlotRatesZCTime -- plots rates from track zero-counting vs. time
//
// This script is like PlotRatesZC, but plots vs. time on the
// horizontal axis rather than vs. lumi. It was originally created
// to further investigate the strange behavior of the track ZC
// lumi in fill 5005, which turned out to be due to ch7 dropping
// out of the readout midway through the fill.
//
// Paul Lujan, June 15 2016
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

void PlotRatesZCTime(void) {
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
  std::vector<double> stepNum;
  std::vector<double> fastOrLumi;
  std::vector<double> trackLumiAll;
  std::vector<double> trackLumiGood;
  std::vector<double> accidentalRate;
  std::vector<double> trackLumiErr;
  std::vector<double> accidentalRateErr;
  std::vector<double> accidentalRateHz;
  std::vector<double> accidentalRateHzErr;
  std::vector<double> ratio;

  FILE *rfile = fopen("CombinedRatesZC_5005.txt", "r");
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file!" << std::endl;
    return(1);
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig, tracksAll, tracksGood, nMeas;
  int nFilledTrig, nEmpty, nFull;
  double totLumi;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d %d %d %d %d %d %d %lf", &tBegin, &tEnd, &nTrig, &tracksAll, &tracksGood,
	   &nFilledTrig, &nEmpty, &nFull, &nMeas, &totLumi);
    // Process the data.
    stepNum.push_back(i);
    fastOrLumi.push_back(totLumi/(nMeas*nBunches));
    trackLumiAll.push_back(-log(1.0-(double)tracksAll/nTrig));
    double goodTrackZC = -log(1.0-(double)nFull/nFilledTrig)*2.8;
    //double goodTrackZC = (double)tracksGood*16.5/nTrig;
    std::cout << goodTrackZC << std::endl;
    trackLumiGood.push_back(goodTrackZC);
    int nAcc = tracksAll-tracksGood;
    double accrate = (double)(tracksAll-tracksGood)/tracksAll;
    accidentalRate.push_back(100.0*accrate);
    accidentalRateErr.push_back(100.0*sqrt(accrate*(1-accrate)/tracksAll));
    accidentalRateHz.push_back((double)nAcc*1000.0/(tEnd-tBegin));
    accidentalRateHzErr.push_back(sqrt(nAcc)*1000.0/(tEnd-tBegin));
    trackLumiErr.push_back(0); // not implemented yet
    ratio.push_back((totLumi/(nMeas*nBunches))/goodTrackZC);
  }
  fclose(rfile);

  // Plot it all.

  TGraph *g1 = new TGraph(nsteps, &(stepNum[0]), &(fastOrLumi[0]));
  TGraph *g2 = new TGraph(nsteps, &(stepNum[0]), &(trackLumiGood[0]));
  TGraph *g3 = new TGraph(nsteps, &(stepNum[0]), &(ratio[0]));

  TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
  g1->Draw("AP");
  g1->SetTitle("Track zero-counting rate vs. time, Fill 5005");
  g1->GetXaxis()->SetTitle("Step number (arbitrary)");
  g1->GetYaxis()->SetTitle("Zero-counting luminosity (arb. units)");
  g1->GetYaxis()->SetTitleOffset(1.7);
  g1->SetMarkerStyle(kFullCircle);
  g1->SetMarkerColor(kBlue);
  g1->SetMarkerSize(1);
  g2->Draw("P same");
  g2->SetMarkerStyle(kFullCircle);
  g2->SetMarkerColor(kRed);
  g2->SetMarkerSize(1);
  TLegend *l = new TLegend(0.5, 0.73, 0.8, 0.88);
  l->AddEntry(g1, "Fast-or lumi", "LP");
  l->AddEntry(g2, "Track lumi, zero counting", "LP");
  l->SetBorderSize(0);
  l->SetFillColor(0);
  l->Draw();

  TCanvas *c3 = new TCanvas("c3", "c3", 600, 600);
  g3->Draw("AP");
  g3->SetTitle("Fast-or/track ratio vs. time");
  g3->GetXaxis()->SetTitle("Step number (arbitrary)");
  g3->GetYaxis()->SetTitle("Ratio of fast-or rate to track zero-counting lumi");
  g3->GetYaxis()->SetTitleOffset(1.4);
  g3->SetMarkerStyle(kFullCircle);
  g3->SetMarkerColor(kBlue);
  g3->SetLineColor(kBlue);
  g3->SetMarkerSize(1);
//   TF1 *f3 = new TF1("f3", "pol1");
//   f3->SetLineColor(kBlue);
//   f3->SetLineWidth(1);
//   g3->Fit(f3);

  c1->Print("TrackZCTime5005.png");
  c3->Print("RatioVsTime5005.png");
}
