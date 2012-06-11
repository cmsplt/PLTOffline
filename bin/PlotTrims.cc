////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Aug 23 13:34:22 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <vector>

#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"

int PlotTrims (std::vector<TString> const& FileNames, TString const Name)
{
  TH1F hTrims("Trims", "All trims", 17, 0, 16);
  TH2F h2Trims("Trims2D", "All Trims", 35, 10, 45, 60, 30, 90);

  for (size_t ifile = 0; ifile != FileNames.size(); ++ifile) {
    std::ifstream f(FileNames[ifile].Data());
    if (!f.is_open()) {
      std::cerr << "ERROR: cannot open file " << FileNames[ifile] << std::endl;
      throw;
    }

    int junk;
    for (int i = 0; i != 36; ++i) {
      f >> junk;
    }

    int col, row, trim;
    while (!f.eof()) {
      f >> col >> row >> junk >> junk >> trim;
      printf("%2i %2i %2i\n", col, row, trim);
      if (trim >= 0 ) hTrims.Fill((float) trim);
      if (trim >= 0 ) h2Trims.SetBinContent(h2Trims.FindBin(col, row), trim);
    }

    TCanvas C;
    C.cd();
    hTrims.SetTitle(Name);
    hTrims.Draw();
    C.SetLogy(1);
    C.SaveAs(Name+".gif");

    C.cd();
    h2Trims.SetTitle(Name);
    h2Trims.Draw("colz");
    C.SetLogy(0);
    C.SaveAs(Name+".gif");


  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " [FileName] [Title]" << std::endl;
    return 1;
  }

  std::vector<TString> FileNames;
  for (int i = 1; i < 2; ++i) {
    FileNames.push_back(argv[i]);
  }
  PlotTrims(FileNames, argv[2]);

  return 0;
}
