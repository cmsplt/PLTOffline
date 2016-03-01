////////////////////////////////////////////////////////////////////
//
//  PlotSlopesByStep -- a script to make a nice version of the slope
//   distributions for individual steps in the VdM scan, as produced
//   by MeasureAccidentals and stored in histo_slopes.root
//
//    Paul Lujan, February 9 2016
//
////////////////////////////////////////////////////////////////////

const char *infile = "histo_slopes.root";

#include <iostream>
#include <string>

void PlotSlopesByStep(void) {
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

  TCanvas *c1 = new TCanvas("c1", "c1", 1200, 900);
  c1->Divide(2,2);

  c1->cd(1);
  TFile *f = new TFile(infile);
  h1 = (TH1F*)f->Get("SlopeX_Step13");
  h1->GetXaxis()->SetTitle("Track SlopeX, all scopes");
  h1->GetYaxis()->SetTitle("Number of tracks");
  h1->SetTitle("SlopeX, central step (zero separation), first scan");
  h1->GetYaxis()->SetTitleOffset(1.5);
  h1->Draw();

  c1->cd(2);
  TFile *f = new TFile(infile);
  h2 = (TH1F*)f->Get("SlopeY_Step13");
  h2->GetXaxis()->SetTitle("Track SlopeY, all scopes");
  h2->GetYaxis()->SetTitle("Number of tracks");
  h2->SetTitle("SlopeY, central step (zero separation), first scan");
  h2->GetYaxis()->SetTitleOffset(1.5);
  h2->Draw();

  c1->cd(3);
  h3 = (TH1F*)f->Get("SlopeX_Step1");
  h3->GetXaxis()->SetTitle("Track SlopeX, all scopes");
  h3->GetYaxis()->SetTitle("Number of tracks");
  h3->SetTitle("SlopeX, first step (max separation), first scan");
  h3->Draw();

  c1->cd(4);
  h4 = (TH1F*)f->Get("SlopeY_Step1");
  h4->GetXaxis()->SetTitle("Track SlopeY, all scopes");
  h4->GetYaxis()->SetTitle("Number of tracks");
  h4->SetTitle("SlopeY, first step (max separation), first scan");
  h4->Draw();

  c1->Print("SlopesByStep.png");
}
