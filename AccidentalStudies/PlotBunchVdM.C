////////////////////////////////////////////////////////////////////
//
//  PlotBunchVdM -- a script to plot the accidental rates and 
//   luminosity from the output produced by ParseCondDBData, 
//   assuming the analysis has been done on a bunch-by-bunch 
//   basis and is for a VdM scan. Allows user to input which
//   bunch and which scan in the VdM they want to plot. If no 
//   scan given, will do all scans.
//    Daniel Gift, July 14, 2016
//    Adapted from PlotRatesZC by Paul Lujan
//
////////////////////////////////////////////////////////////////////

// This script is currently set up for VdM scan 4954

// Scan possibilities are X1, X2, X3, Y1, Y2, Y3 (for the regular scans 
//                                        in the respective directions and 
//                                        in the correct ordering),
//  1imagingX, 1imagingY, 2imagingX, 2imagingY (for the imaging scans, for
//                                        the respective means and directions),
//  and lengthX, lengthY (for the length calibration scans)

#include <iostream>
#include <string>
#include <vector>
#include <time.h>
const float lumMin = 10.0;
static const float calibration = 11246./310.9144407;
void PlotBunchVdM(std::string BID, std::string scan = ""){
  // Specific to this VdM fill
  std::string fileName = "CombinedRates_4954_" + BID + ".txt";
  const int fill = 4954;
  
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
  gStyle->SetStatY(0.2);

  // Vectors to read in data
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

  std::cout<<"This script ignores rows whose last column is less than "<<lumMin<<std::endl;

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

  // The start and stop of the appropriate scan
  int min;
  int max;

  if (scan == "X1") {
    min = 29085431;
    max = 30039676;
  }
  else if (scan == "Y1") {
    min = 30322339;
    max = 31279577;
  }
  else if (scan == "Y2") {
    min = 31569528;
    max = 32528151;
  }
  else if (scan == "X2") {
    min = 32748126;
    max = 33702425;
  }
  else if (scan == "2imagingX") {
    min = 34047807;
    max = 34991850;
  }
  else if (scan == "2imagingY") {
    min = 35477032;
    max = 36421139;
  }
  else if (scan == "1imagingX") {
    min = 37229694;
    max = 38170894;
  }
  else if (scan == "1imagingY") {
    min = 38452142;
    max = 39404964;
  }
  else if (scan == "lengthX") {
    min = 39844939;
    max = 41035342;
  }
  else if (scan == "lengthY") {
    min = 41320858;
    max = 42527275;
  }
  else if (scan == "X3") {
    min = 43669489;
    max = 44622282;
  }
  else if (scan == "Y3") {
    min = 44942813;
    max = 45902961;
  }
  else if (scan == "") {
    min = 28993655;
    max = 45996271;
  }
  else {
    std::cerr << "Scan Type invalid, silly!" << std::endl;
    return(1);
  }

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d %lf %lf %d %lf %lf %d %lf", &tBegin, &tEnd, &nTrig, &tracksAll, &tracksGood,
    	   &nFilledTrig, &nEmpty, &nFull, &nMeas, &totLumi);

    // Make sure the data does't produce infinities
    if ((nMeas == 0) || (nFilledTrig == (int)0) || (nBunches == 0) || (tracksAll == 0) || (tBegin == tEnd) (nFull == nFilledTrig) || (tracksAll == nTrig) || (nFull == 0.0))
      continue;
    // Ignore small luminosities for which the trend is not valid
    if (totLumi < lumMin)
      continue;
    // Only look at dat in the range for this scan
    if ((tBegin < min) || (tBegin > max)) continue;

    // Process the data.
    fastOrLumi.push_back(totLumi/(nMeas*nBunches));
    trackLumiAll.push_back(-log(1.0-(double)tracksAll/nTrig));
    double f0 = 1.0-(double)nFull/nFilledTrig;
    double f0err = sqrt(f0*(1-f0)/nFilledTrig);
    double goodTrackZC = -log(f0)*calibration;
    double goodTrackZCPlus = -log(f0-f0err)*calibration;
    trackLumiGood.push_back(goodTrackZC);
    int nAcc = tracksAll-tracksGood;
    double accrate = (double)(tracksAll-tracksGood)/(tracksAll);
    accidentalRate.push_back(100.0*accrate);
    accidentalRateErr.push_back(100.0*sqrt(accrate*(1-accrate)/tracksAll));
    accidentalRateHz.push_back((double)nAcc*1000.0/(tEnd-tBegin));
    accidentalRateHzErr.push_back(sqrt(nAcc)*1000.0/(tEnd-tBegin));
    fastOrErr.push_back(0);    // assume this is negligible for now
    trackLumiErr.push_back(fabs(goodTrackZCPlus-goodTrackZC));
    double ratioVal = (totLumi/(nMeas*nBunches))/(goodTrackZC);
    double ratioErrVal = (goodTrackZCPlus/goodTrackZC)*ratioVal - ratioVal;
    ratio.push_back(ratioVal);
    ratioErr.push_back(fabs(ratioErrVal));
    std::cout<<ratioVal<<"     "<<ratioErrVal<<std::endl;
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
  TF1 *f1 = new TF1("f1", "pol1");
  f1->FixParameter(0, 0);
  f1->SetLineColor(kBlue);
  f1->SetLineWidth(1);
  g1->Fit(f1);

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
  TF1 *f2 = new TF1("f2", "pol0"); // pol0 for VdM, pol1 for regular fills
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
  TF1 *f3 = new TF1("f3", "pol0");
  f3->SetLineColor(kBlue);
  f3->SetLineWidth(1);
  g3->Fit(f3);
}
