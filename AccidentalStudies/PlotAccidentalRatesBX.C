////////////////////////////////////////////////////////////////////
//
//  PlotAccidentalRatesBX -- a script to plot the results of
//    MeasureAccidentalRatesBX
//    Paul Lujan, March 8 2016
//
////////////////////////////////////////////////////////////////////

// This script only plots the rates for a single fill (currently fill
// 4449). As such, things like the fill pattern are hardcoded.

const int nBX = 3564;
const int nTrains = 22;
const int leadingBunch[nTrains] = {39, 79, 217, 217+81, 405, 405+81, 1111, 1111+81, 1299, 1299+81,
				   1817, 1817+81, 2005, 2005+81, 2193, 2193+81, 2711, 2711+81,
				   2899, 2899+81, 3087, 3087+81};
const int trainLength[nTrains] = {1, 12, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
				  72, 72, 72, 72, 72, 72, 72, 72, 72, 72};

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

void PlotAccidentalRatesBX(void) {
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
  gStyle->SetOptStat(0);

  TH1F *accRateLeading = new TH1F("accRateLeading", "Accidental rate, leading bunches", 50, 7.0, 12.0);
  TH1F *accRateTrain = new TH1F("accRateTrain", "Accidental rate, train bunches", 50, 7.0, 12.0);

  // Read input file.
  float allTracks[nBX];
  float goodTracks[nBX];
  int totBunches[6];    // number of bunches of each type
  float totAllRate[6];  // and rate for bunches of that type
  float totGoodRate[6]; // rate of good tracks
  const char *typeDescription[6] = {"unfilled", "leading", "train", "filled noncolliding", "1 after train", "2 after train"};
  for (int i=0; i<6; ++i) {
    totBunches[i] = 0;
    totAllRate[i] = 0;
    totGoodRate[i] = 0;
  }

  FILE *infile = fopen("AccidentalRatesBX_4449.txt", "r");
  if (infile == NULL) {
    std::cerr << "Couldn't open rates file!" << std::endl;
    return(1);
  }
  int ibx, numTrigs, tracksAll, tracksGood;

  fscanf(infile, "%d", &ibx);
  if (ibx != nBX) std::cout << "Warning: demons may fly out of your nose!" << std::endl;
  for (int i=0; i<nBX; ++i) {
    fscanf(infile, "%d %d %d %d", &ibx, &numTrigs, &tracksAll, &tracksGood);
    if (numTrigs < 5) continue;
    
    float totRate = (float)tracksAll/numTrigs;
    float goodRate = (float)tracksGood/numTrigs;
    double accrate = 0, accidentalRate = 0, accidentalRateErr = 0;
    if (tracksAll > 0) {
      accrate = (double)(tracksAll-tracksGood)/tracksAll;
      accidentalRate = 100.0*accrate;
      accidentalRateErr = 100.0*sqrt(accrate*(1-accrate)/tracksAll);
    }
  
    int bunchType = 0; // unfilled
    for (int i=0; i<nTrains; ++i) {
      if (ibx == leadingBunch[i]) bunchType = 1; // leading
      if (ibx > leadingBunch[i] && ibx < leadingBunch[i]+trainLength[i]) bunchType = 2; //train
      if (ibx == leadingBunch[i]+trainLength[i]) bunchType = 4; //1 after train
      if (ibx == leadingBunch[i]+trainLength[i]+1) bunchType = 5; //2 after train
    };
    if ((ibx >= 130 && ibx <= 141) || (ibx >= 170 && ibx <= 181))
      bunchType = 3; // filled noncolliding
    
    if (bunchType == 1) {
      // printf("Accidental rate for BX %d (leading) = %.2f +/- %.2f\n", ibx, accidentalRate, accidentalRateErr);
      accRateLeading->Fill(accidentalRate);
    }
    if (bunchType == 2) {
      // printf("Accidental rate for BX %d (train) = %.2f +/ %.2f\n", ibx, accidentalRate, accidentalRateErr);
      accRateTrain->Fill(accidentalRate);
    }
    if (bunchType == 0 && totRate > 0.05)
      std::cout << "Rate for unfilled bunch " << ibx << "=" << totRate << std::endl;

    totBunches[bunchType]++;
    totAllRate[bunchType] += totRate;
    totGoodRate[bunchType] += goodRate; 
  }
  fclose(infile);
  
  // Plot it all.

  TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
  accRateTrain->Draw();
  accRateTrain->Scale(100.0/accRateTrain->GetEntries());
  accRateLeading->Draw("same");
  accRateLeading->Scale(10.0/accRateLeading->GetEntries());
  accRateTrain->SetLineColor(kRed);
  accRateTrain->GetXaxis()->SetTitle("Accidental rate [%]");
  accRateTrain->GetYaxis()->SetTitle("Frequency of occurrence [a.u.]");
  accRateTrain->SetTitle("Accidental rates by BX, fill 4449");
  
  TLegend *l = new TLegend(0.6, 0.6, 0.97, 0.8);
  char buf[512];
  sprintf(buf, "#splitline{Leading/solo bunches}{mean=%.2f%%}", accRateLeading->GetMean());
  l->AddEntry(accRateLeading, buf);
  sprintf(buf, "#splitline{Train bunches}{mean=%.2f%%}", accRateTrain->GetMean());
  l->AddEntry(accRateTrain, buf);
  l->SetFillColor(0);
  l->Draw();
  c1->Print("AccidentalRatesBX_4449.png");

  for (int i=0; i<6; ++i) {
    printf("All/good rate for %s bunches = %.2e / %.2e\n", typeDescription[i],
	   totAllRate[i]/totBunches[i], totGoodRate[i]/totBunches[i]);
  }
}
