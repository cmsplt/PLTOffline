////////////////////////////////////////////////////////////////////
//
//  PlotAccidentalRates -- a script to plot the accidental rates
//   and relative rates per telescope (FED Channel)
//   given the combined file produced by ParseCondDBDataTele
//    Paul Lujan, November 10 2015
//    Joseph Heideman, March 11 2016
////////////////////////////////////////////////////////////////////

// This script only plots the rates per telescope for a single fill. If you want
// something more advanced, check out PlotAccidentalRatesAllScans or PlotAccidentalRatesAllScansTele.

#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include "TMatrix.h"
void PlotAccidentalRatesAllTele(void) {
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
  std::vector<double> trackLumiErr;



  FILE *rfile = fopen("CombinedRatesTele4555_Align4444.txt", "r");
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file!" << std::endl;
    return(1);
  }
const int nscopes = 14;  //switch to 16 for 2016 Run
const int nsteps;
int nBunches, tBegin, tEnd, nTrig, tracksAllTele[nscopes], tracksGoodTele[nscopes], nMeas;  
double totLumi;
const int fedChannel[nscopes] = {1, 2, 4, 5, 7, 8, 10, 11, 13, 14, 16, 17, 19, 20}; // 22, 23;

 ///  All Channels
const bool plotChannel[nscopes] = {true, true, true, true, true, true, true, true, true, true, false, false, false, false};// true, true;

   /// -Z
//const bool plotChannel[nscopes] = {true, true, true, true, true, true, true, true, false, false, false, false, false}; //, false, false, false};
  
   /// +Z
//const bool plotChannel[nscopes] = {false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true};

 fscanf(rfile, "%d %d", &nsteps, &nBunches);

TMatrixT<double> tracks(nsteps,nscopes);
TMatrixT<double> trackErr(nsteps,nscopes);
TMatrixT<double> reltracks(nsteps,nscopes);
TMatrixT<double> reltrackErr(nsteps,nscopes);

  for (int i=0; i<nsteps; ++i) {
        fscanf(rfile, "%d %d %d %d %d %d %d %d %d %d", &tBegin, &tEnd, &nTrig, &tracksAllTele[0], &tracksGoodTele[0], &tracksAllTele[1], &tracksGoodTele[1], &tracksAllTele[2], &tracksGoodTele[2], &tracksAllTele[3]);

        fscanf(rfile, "%d %d %d %d %d %d %d %d %d %d", &tracksGoodTele[3], &tracksAllTele[4], &tracksGoodTele[4], &tracksAllTele[5], &tracksGoodTele[5], &tracksAllTele[6], &tracksGoodTele[6], &tracksAllTele[7], &tracksGoodTele[7], &tracksAllTele[8]); 

        fscanf(rfile, "%d %d %d %d %d %d %d %d %d %d", &tracksGoodTele[8], &tracksAllTele[9], &tracksGoodTele[9], &tracksAllTele[10], &tracksGoodTele[10], &tracksAllTele[11], &tracksGoodTele[11], &tracksAllTele[12], &tracksGoodTele[12], &tracksAllTele[13]);

        fscanf(rfile, "%d %d %lf\n", &tracksGoodTele[13], &nMeas, &totLumi);

    // Process the data.
//choose which telescope by changing channel # on tracksAllTele and tracksGoodTele
    fastOrLumi.push_back(totLumi/(nMeas*nBunches));

 for (int j=0; j<nscopes; ++j){
//	int nAcc = tracksAllTele[j]-tracksGoodTele[j];
        double accrate = (double)(tracksAllTele[j]-tracksGoodTele[j])/(tracksAllTele[j]);
    tracks(i,j)= 100.0*accrate;
    trackErr(i,j)=100.0*sqrt(accrate*(1-accrate)/tracksAllTele[j]);
    reltracks(i,j) =100.0*accrate/tracks(i,0);
    reltrackErr(i,j) = reltracks(i,j)*sqrt(trackErr(i,j)*trackErr(i,j)/(tracks(i,j)*tracks(i,j))+trackErr(0,j)*trackErr(0,j)/(tracks(0,j)*tracks(0,j)));
        }
    trackLumiErr.push_back(0);
  }

  fclose(rfile);
 
TCanvas *c1 = new TCanvas("c1", "c1", 700, 900);
TGraph* g[nscopes];
TF1* f[nscopes];
TGraph* grel[nscopes];
TF1* frel[nscopes];

std::vector<double> scopevec; 
std::vector<double> scopeErrvec; 
std::vector<double> scoperelvec;
std::vector<double> scoperelErrvec;

// Plot it all.
for(int ig = 0; ig < nscopes; ++ig){
for(int irow = 0; irow < nsteps; ++irow){
  scopevec.push_back(tracks(irow,ig));
  scopeErrvec.push_back(trackErr(irow,ig));
  scoperelvec.push_back(reltracks(irow,ig));
  scoperelErrvec.push_back(reltrackErr(irow,ig));   
      }
     g[ig] = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(scopevec[0]), &(trackLumiErr[0]), &(scopeErrvec[0]));
     grel[ig] = new TGraphErrors(nsteps, &(fastOrLumi[0]), &(scoperelvec[0]), &(trackLumiErr[0]), &(scoperelErrvec[0]));
   scopevec.resize(0);
   scopeErrvec.resize(0);
   scoperelvec.resize(0);
   scoperelErrvec.resize(0);
 }
 
  g[0]->Draw("APX");
  g[0]->SetTitle("Accidental rate vs. fast-or lumi");
  g[0]->GetXaxis()->SetTitle("Average fast-or lumi per bunch");
  g[0]->GetYaxis()->SetTitle("Accidental rate in pixel data (% of tracks)");
  g[0]->GetYaxis()->SetTitleOffset(1.4);
  g[0]->GetYaxis()->SetRangeUser(7,13);
  g[0]->SetMarkerStyle(kFullCircle);
  g[0]->SetMarkerColor(kBlack);
  g[0]->SetLineColor(kBlack);
  g[0]->SetMarkerSize(1);
  f[0] = new TF1("f0","pol1");
  f[0]->SetLineColor(kBlack);
  f[0]->SetLineWidth(1);
  g[0]->Fit(f[0], "", "", 0, 2.3);

   TLegend *lg = new TLegend(0.80,0.65,0.99,0.95);
    lg->SetHeader("Telescope FED Channel");
    lg->AddEntry(g[0],"FED Channel 1","LP");
    lg->Draw();
   
   char buf[32], lgbuf[32];
      
   for (int ic=1; ic < nscopes; ++ic) {
 if (plotChannel[ic]) {   
 
        g[ic]->Draw("P same");
        g[ic]->SetMarkerStyle(kFullCircle);
     int iColor = ic+2;
    if(iColor>=5) iColor++;
    if(iColor>=10) iColor++;
        g[ic]->SetMarkerColor(iColor);
        g[ic]->SetLineColor(iColor);
        g[ic]->SetMarkerSize(1);
    sprintf(buf,"f%i",fedChannel[ic]);
     f[ic] = new TF1(buf,"pol1");
     f[ic]->SetLineColor(iColor);
     f[ic]->SetLineWidth(1);
     g[ic]->Fit(f[ic]);
   sprintf(lgbuf,"FED Channel %i", fedChannel[ic]);
     lg->AddEntry(g[ic],lgbuf,"LP");   
       }
     }
     
TCanvas *c2 = new TCanvas("c2","c2", 700, 900);
  grel[0]->Draw("APX");
  grel[0]->SetTitle("Relative Accidental rate vs. fast-or lumi");
  grel[0]->GetXaxis()->SetTitle("Average fast-or lumi per bunch");
  grel[0]->GetYaxis()->SetTitle("Rel. Accd. rate in pixel data (FED Channel/FED 1)");
  grel[0]->GetYaxis()->SetTitleOffset(1.4);
  grel[0]->GetYaxis()->SetRangeUser(.70,1.30);
  grel[0]->SetMarkerStyle(kFullCircle);
  grel[0]->SetMarkerColor(kBlack);
  grel[0]->SetLineColor(kBlack);
  grel[0]->SetMarkerSize(1);
  frel[0] = new TF1("frel0","pol1");
  frel[0]->SetLineColor(kBlack);
  frel[0]->SetLineWidth(1);
  grel[0]->Fit(frel[0], "", "", 0, 2.3);

   TLegend *lgrel = new TLegend(0.80,0.65,0.99,0.95);
    lgrel->SetHeader("Telescope FED Channel");
    lgrel->AddEntry(g[0],"FED Channel 1","LP");
    lgrel->Draw();
      
   for (int ic=1; ic < nscopes; ++ic) {
 if (plotChannel[ic]) {   
        grel[ic]->Draw("P same");
        grel[ic]->SetMarkerStyle(kFullCircle);
     int iColor = ic+2;
    if(iColor>=5) iColor++;
    if(iColor>=10) iColor++;
        grel[ic]->SetMarkerColor(iColor);
        grel[ic]->SetLineColor(iColor);
        grel[ic]->SetMarkerSize(1);
    sprintf(buf,"frel%i",fedChannel[ic]);
     frel[ic] = new TF1(buf,"pol1");
     frel[ic]->SetLineColor(iColor);
     frel[ic]->SetLineWidth(1);
     grel[ic]->Fit(frel[ic]);
   sprintf(lgbuf,"FED Channel %i", fedChannel[ic]);
     lgrel->AddEntry(grel[ic],lgbuf,"LP");   
       }
     }

          // r8->SetMarkerSize(1); 
          //c2->Print("AccidentalRate_MuScan1_Central_Final.png");
     }
