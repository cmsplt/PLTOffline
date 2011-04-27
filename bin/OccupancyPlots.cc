////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Apr  4 11:17:28 CDT 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <map>

#include "TH2F.h"
#include "TCanvas.h"
#include "TString.h"
#include "TStyle.h"




TH2F* GetNewHist2(int const ch, int const roc)
{
  char BUFF[200];
  sprintf(BUFF, "Occupancy_Ch%02i_ROC%1i", ch, roc);

  return new TH2F(BUFF, TString(BUFF)+";Column;Pixel", 35, 10, 45, 50, 35, 85);
}





int OccupancyPlots (TString const InFileName)
{
  gStyle->SetOptStat(11);
  std::ifstream f(InFileName.Data());
  if (!f) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    exit(1);
  }

  std::map<int, TH2F*> m2;

  int ch, roc, col, pix, adc, ev;
  std::map< std::pair<int, int>, int> m1;

  for (int i = 0; f >> ch >> roc >> col >> pix >> adc >> ev; ++i) {
    if (!m2[ch*10 + roc]) {
      m2[ch*10 + roc] = GetNewHist2(ch, roc);
    }
    m2[ch*10 + roc]->Fill(col, pix);

    ++m1[ std::make_pair(10*ch+roc, ev) ];


  }

  for (std::map<int, TH2F*>::iterator it = m2.begin(); it != m2.end(); ++it) {
    char BUFF[200];
    sprintf(BUFF, "Occupancy_Ch%02i_ROC%1i.eps", it->first/10, it->first % 10);
    TCanvas c;
    it->second->Draw("colz");
    c.SaveAs(BUFF);
  }

  //for (std::map< std::pair<int, int>, int >::iterator it = m1.begin(); it != m1.end(); ++it) {
  //  char BUFF[200];
  //  sprintf(BUFF, "Occupancy_Ch%02i_ROC%1i.eps", it->first.first/10, it->first.first % 10);
  //  TCanvas c;
  //  it->second->Draw();
  //  c.SaveAs(BUFF);
  //}

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  TString const InFileName = argv[1];

  OccupancyPlots(InFileName);

  return 0;
}
