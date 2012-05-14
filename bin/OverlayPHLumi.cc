////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed May  2 17:32:26 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <vector>
#include <map>

#include "TFile.h"
#include "TString.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TFrame.h"
#include "TPad.h"
#include "TH1F.h"
#include "TGaxis.h"

int OverlayPHLumi (TString const PHFileName, TString const LumiFileName)
{
  TFile PHFile(PHFileName, "read");
  TFile LumiFile(LumiFileName, "read");
  if (!PHFile.IsOpen() || !LumiFile.IsOpen()) {
    std::cerr << "ERROR: cannot open input files" << std::endl;
    exit(1);
  }

  TFile OutFile("MyOutGraphs.root", "recreate");
  if (!OutFile.IsOpen()) {
    std::cerr << "ERROR: Cannot open output file" << std::endl;
    exit(1);
  }

  std::vector<int> PHChannels;
  PHChannels.push_back(13);
  PHChannels.push_back(14);
  PHChannels.push_back(15);
  PHChannels.push_back(16);
  //PHChannels.push_back(24);

  std::map<int, int> LumiChannels;
  LumiChannels[13] = 0;
  LumiChannels[14] = 1;
  LumiChannels[15] = 3;
  LumiChannels[16] = 4;
  //LumiChannels.push_back(5);


  Double_t const xmin = 0;//7500e+3;//pad->GetUxmin();
  Double_t const ymin = 0;
  Double_t const xmax = 15000e+3;//10000e+3;//pad->GetUxmax();
  Double_t const ymax = 70000;

  // Loop over channels
  for (std::vector<int>::iterator ch = PHChannels.begin(); ch != PHChannels.end(); ++ch) {

    TGraph* gLumi = (TGraph*) LumiFile.Get( TString::Format("LumiRages_Ch%i", LumiChannels[*ch]));
    if (!gLumi) {
      std::cerr << "ERROR: cannot find lumi graph" << std::endl;
      continue;
    }

    // loop over rocs
    for (size_t iroc = 0; iroc != 3; ++iroc) {

      TString const Name = TString::Format("TimeAvgGraph_Ch%i_ROC%i", (int) *ch, (int) iroc);
      TCanvas Can(Name, Name);
      Can.cd();


      bool DrawnFirst = false;
      for (int icl = 3; icl >= 1; --icl) {
        TGraphErrors* gPH = (TGraphErrors*) PHFile.Get( TString::Format("TimeAvgGraph_id%i%i_Cl%i", (int) *ch, (int) iroc, (int) icl) );
        if (gPH == 0x0) {
          printf("Skipping %s\n", TString::Format("TimeAvgGraph_id%i%i_Cl%i", (int) *ch, (int) iroc, (int) icl).Data()); 
          continue;
        }
        printf("Drawing ch: %i  roc: %i  iCl: %i\n", *ch, (int) iroc, (int) icl); 
        gPH->GetXaxis()->SetRangeUser(xmin, xmax);
        if (!DrawnFirst) {
          gPH->SetTitle( TString::Format("Channel %i  ROC %i", (int) *ch, (int) iroc) );
          gPH->GetXaxis()->SetTitle("Time (ms)");
          gPH->GetYaxis()->SetTitle("Mean Pulse Height (Electrons)");
          gPH->GetYaxis()->SetLabelSize(0.03);
          gPH->GetYaxis()->SetLabelColor(4);
          gPH->GetYaxis()->SetTitleOffset(1.20);
          gPH->Draw("Ap");
          DrawnFirst = true;
        } else {
          gPH->Draw("p");
        }
      }

      //TPad* pad = (TPad*) Can.GetPad(1)->GetPadPointer();
      Can.cd();
      TPad *overlay = new TPad("overlay","",0,0,1,1);
      overlay->SetFillStyle(4000);
      overlay->SetFillColor(0);
      overlay->SetFrameFillStyle(4000);
      overlay->Draw();
      overlay->cd();
      //gLumi = new TGraphErrors(n2,x2,y2,ex2,ey2);
      TH1F *hframe = overlay->DrawFrame(xmin,ymin,xmax,ymax);
      hframe->GetXaxis()->SetLabelOffset(99);
      hframe->GetYaxis()->SetLabelOffset(99);
      gLumi->Draw("p");

      TGaxis *axis = new TGaxis(xmax,ymin,xmax, ymax,ymin,ymax,510,"+L");
      //axis->SetLineColor(kRed);
      //axis->SetLabelColor(kRed);
      axis->SetLabelSize(0.03);
      //axis->SetLabelOffset(.01);
      axis->SetTitle("Coincidence Counts per 1/3 seconds");
      axis->Draw();



      Can.Modified();
      Can.SaveAs( TString::Format("PulseHeightTime_Ch%i_ROC%i.gif", (int) *ch, (int) iroc) );
      OutFile.cd();
      Can.Write();
    }

  }



  OutFile.Close();





  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [PulseHeightFile] [HistFile]" << std::endl;
    return 1;
  }

  OverlayPHLumi(argv[1], argv[2]);

  return 0;
}
