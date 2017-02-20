////////////////////////////////////////////////////////////////////
//
//  PlotRatesZC -- a script to plot the accidental rates and
//   luminosity from zero-counting given the combined file produced 
//   by ParseCondDBData. For new files with 10 rows and that were
//   calculated from channel averaging
//     Paul Lujan, November 10 2015
//     Edited by Daniel Gift, July 14 2016
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

// The fast-or sigmavis isn't going to be very sensible for the Slink
// data. Instead just use 3 which seems to be pretty good for the moment.
//static const float calibration = 11246./310.9144407;
static const float calibration = 3.0;

void PlotRatesZC(const std::string fileName, const int fill) {
  // Set this true of you want the program to ignore very small luminosities
  // In general, you probably either want this, or it won't matter
  bool eliminateSmall = true;
  // Minimum luminosity (value in last row of input) to be considered valid 
  // Only relvant if eliminateSmall is true
  float minLum = 1.0;

  // style from PLTU
  gROOT->SetStyle("Plain");                  
  gStyle->SetPalette(1);
  gStyle->SetPadLeftMargin(0.17);
  gStyle->SetPadRightMargin(0.17);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleX(0.18);
  gStyle->SetTitleY(1.0);
  gStyle->SetTitleH(0.09);
  gStyle->SetTitleW(0.7);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetOptFit(111);
  gStyle->SetStatY(0.35);

  //Vectors into which we read the data
  std::vector<double> fastOrLumi;
  std::vector<double> trackLumiAll;
  std::vector<double> trackLumiGood;
  std::vector<double> accidentalRate;
  std::vector<double> fastOrErr;
  std::vector<double> trackLumiErr;
  std::vector<double> accidentalRateErr;
  std::vector<double> accidentalRateHz;
  std::vector<double> accidentalRateHzErr;
  std::vector<double> ratio;
  std::vector<double> ratioErr;

  if (eliminateSmall)
    std::cout<<"This program ignores rows whose last element is smaller than "<<minLum<<std::endl;
  // Read input file.
  FILE *rfile = fopen(fileName.c_str(), "r");
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file!" << std::endl;
    return(1);
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig, nMeas;
  double tracksAll, tracksGood;

  int nFilledTrig;
  double nEmpty, nFull;
  double totLumi;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d %lf %lf %d %lf %lf %d %lf", &tBegin, &tEnd, &nTrig, &tracksAll, &tracksGood,
    	   &nFilledTrig, &nEmpty, &nFull, &nMeas, &totLumi);

    // Make sure the data in this row won't cause infinities
    if ((nMeas == 0) || (nFilledTrig == (int)0) || (nBunches == 0) || (tracksAll == 0) || (tBegin == tEnd) (nFull == nFilledTrig) || (tracksAll == nTrig) || (nFull == 0.0))
      continue;

    // If turned on, ignore small luminosities that might throw off the data
    if (eliminateSmall) {
      if (totLumi < minLum)
	continue;
    }
  
    // Process the data.
    fastOrLumi.push_back(totLumi/(nMeas*nBunches));
    trackLumiAll.push_back(-log(1.0-(double)tracksAll/nTrig));
    double f0 = 1.0-(double)nFull/nFilledTrig;
    // It's unclear whether the error should include 1/sqrt(n_channels) or not. If I do include it
    // the errors look too small, so I'm leaving it out for the moment.
    double f0err = sqrt(f0*(1-f0)/nFilledTrig);///sqrt(13.);
    double goodTrackZC = -log(f0)*calibration;
    double goodTrackZCPlus = -log(f0-f0err)*calibration;
    trackLumiGood.push_back(goodTrackZC);
    int nAcc = tracksAll-tracksGood;
    double accrate = (double)(tracksAll-tracksGood)/(tracksAll);
    accidentalRate.push_back(100.0*accrate);
    accidentalRateErr.push_back(100.0*sqrt(accrate*(1-accrate)/tracksAll));///sqrt(13.);
    accidentalRateHz.push_back((double)nAcc*1000.0/(tEnd-tBegin));
    accidentalRateHzErr.push_back(sqrt(nAcc)*1000.0/(tEnd-tBegin));
    fastOrErr.push_back(0);    // assume this is negligible for now
    trackLumiErr.push_back(fabs(goodTrackZCPlus-goodTrackZC));
    double ratioVal = (totLumi/(nMeas*nBunches))/(goodTrackZC);
    double ratioErrVal = ((goodTrackZCPlus/goodTrackZC)*ratioVal - ratioVal);
    ratio.push_back(ratioVal);
    ratioErr.push_back(fabs(ratioErrVal));
  }
  fclose(rfile);

  // Plot it all.
  nsteps = ratio.size();
  TGraph *g1 = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(trackLumiGood[0]),
				&(fastOrErr[0]), &(trackLumiErr[0]));
  TGraph *g2 = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(accidentalRate[0]),
				&(trackLumiErr[0]), &(accidentalRateErr[0]));
  TGraph *g3 = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(ratio[0]),
				&(fastOrErr[0]), &(ratioErr[0]));

  std::stringstream plotTitle;

  TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
  g1->Draw("AP");
  plotTitle << "Track zero-counting rate vs. fast-or rate, Fill " << fill;
  g1->SetTitle((plotTitle.str()).c_str());
  g1->GetXaxis()->SetTitle("Avg. corrected fast-or lumi per bunch (Hz/#mub)");
  g1->GetYaxis()->SetTitle("Zero-counting lumi from Slink data");
  g1->GetYaxis()->SetTitleOffset(2.1);
  g1->SetMarkerStyle(kFullCircle);
  g1->SetMarkerColor(kBlue);
  g1->SetMarkerSize(1);
  TF1 *f1 = new TF1("f1", "pol1"); // or pol2 if you're paranoid
  f1->FixParameter(0, 0);
  f1->SetLineColor(kBlue);
  f1->SetLineWidth(1);
  g1->Fit(f1);
  c1->Update();

  plotTitle.str("");
  TCanvas *c2 = new TCanvas("c2", "c2", 600, 600);
  g2->Draw("AP");
  plotTitle << "Accidental rate vs. online luminosity, Fill " << fill;
  g2->SetTitle((plotTitle.str()).c_str());
  g2->GetXaxis()->SetTitle("Online per-bunch inst. lumi (Hz/#mub)");
  g2->GetYaxis()->SetTitle("Measured accidental rate (%)");
  g2->GetYaxis()->SetTitleOffset(1.6);
  g2->SetMarkerStyle(kFullCircle);
  g2->SetMarkerColor(kBlue);
  g2->SetLineColor(kBlue);
  g2->SetMarkerSize(1);
  TF1 *f2 = new TF1("f2", "pol1"); // pol0 for VdM, pol1 for regular fills
  f2->SetLineColor(kBlue);
  f2->SetLineWidth(1);
  g2->Fit(f2);
  gStyle->SetStatY(0.8);

  plotTitle.str("");
  TCanvas *c3 = new TCanvas("c3", "c3", 600, 600);
  g3->Draw("AP");
  plotTitle << "Fast-or/track ratio vs. lumi, Fill " << fill;
  g3->SetTitle((plotTitle.str()).c_str());
  g3->GetXaxis()->SetTitle("Avg. corrected fast-or lumi per bunch (Hz/#mub)");
  g3->GetYaxis()->SetTitle("Ratio of fast-or rate to track zero-counting lumi");
  g3->GetYaxis()->SetTitleOffset(1.6);
  g3->SetMarkerStyle(kFullCircle);
  g3->SetMarkerColor(kBlue);
  g3->SetLineColor(kBlue);
  g3->SetMarkerSize(1);
  // You can use either pol0 or pol1 here depending on what you're looking for.
  TF1 *f3 = new TF1("f3", "pol1");
  f3->SetLineColor(kBlue);
  f3->SetLineWidth(1);
  g3->Fit(f3);

  char buf[64];
  sprintf(buf, "TrackZCVsFastOr%d.png", fill);
  c1->Print(buf);
  sprintf(buf, "TrackZCRatio%d.png", fill);
  c3->Print(buf);
}
