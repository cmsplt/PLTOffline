////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Jun  7 15:41:22 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "TFile.h"
#include "TString.h"
#include "TH1.h"
#include "TSpectrum.h"
#include "TCanvas.h"
#include "TLine.h"


int FitAddressLevels (TString const FileName, int const mFec, int const mFecChannel, int const hubAddress)
{
  TFile InFile(FileName, "read");

  TString HistName = TString::Format("mFec%i_mFecChannel%i_hub%i_header", mFec, mFecChannel, hubAddress);
  TH1* headerLevels = (TH1*) InFile.Get(HistName);
  headerLevels->SetTitle("TBM");

  TH1* addressLevels[3];
  HistName = TString::Format("mFec%d_mFecChannel%d_hub%d_roc%d", mFec, mFecChannel, hubAddress, 0);
  addressLevels[0]  = (TH1*)InFile.Get(HistName);
  addressLevels[0]->SetTitle("ROC 0");
  HistName = TString::Format("mFec%d_mFecChannel%d_hub%d_roc%d", mFec, mFecChannel, hubAddress, 1);
  addressLevels[1]  = (TH1*)InFile.Get(HistName);
  addressLevels[1]->SetTitle("ROC 1");
  HistName = TString::Format("mFec%d_mFecChannel%d_hub%d_roc%d", mFec, mFecChannel, hubAddress, 2);
  addressLevels[2]  = (TH1*)InFile.Get(HistName);
  addressLevels[2]->SetTitle("ROC 2");

  TLine Line[4];
  Line[0].SetLineColor(2);
  Line[1].SetLineColor(2);
  Line[2].SetLineColor(2);
  Line[3].SetLineColor(2);

  Line[0].SetX1(400); Line[0].SetX2(400);
  Line[0].SetY1(0);   Line[0].SetY2(16000);

  Line[1].SetX1(460); Line[1].SetX2(460);
  Line[1].SetY1(0);   Line[1].SetY2(16000);

  Line[2].SetX1(550); Line[2].SetX2(550);
  Line[2].SetY1(0);    Line[2].SetY2(16000);

  Line[3].SetX1(650); Line[3].SetX2(650);
  Line[3].SetY1(0);    Line[3].SetY2(16000);


  TCanvas Can("AddressLevels", "AddressLevels", 800, 800);
  Can.Divide(1,4);
  Can.cd(1);
  headerLevels->Draw();
  Can.cd(2);
  addressLevels[0]->Draw();
  Line[0].Draw("same");
  Line[1].Draw("same");
  Line[2].Draw("same");
  Line[3].Draw("same");
  Can.cd(3);
  addressLevels[1]->Draw();
  Line[0].Draw("same");
  Line[1].Draw("same");
  Line[2].Draw("same");
  Line[3].Draw("same");
  Can.cd(4);
  addressLevels[2]->Draw();
  Line[0].Draw("same");
  Line[1].Draw("same");
  Line[2].Draw("same");
  Line[3].Draw("same");

  Can.SaveAs("AddressLevels.gif");


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  FitAddressLevels("~/level_histos.root", 8, 2, 29);

  return 0;
}
