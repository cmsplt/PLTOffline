////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu Apr 14 17:34:29 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PLTGainCal.h"

#include "TGraph.h"
#include "TFile.h"
#include "TCanvas.h"

int TestGainCal (TString const GainCalFileName)
{
  PLTGainCal GainCal(GainCalFileName);

  std::cout << GainCal.GetCharge(22, 1, 16, 50, 200) << std::endl;

  int const Begin = 200;
  int const End = 800;
  int const N = End - Begin;
  float X[N];
  float Y[N];

  char BUFF[200];

  TFile f("GainCalPlots.root", "recreate");
  if (!f.IsOpen()) {
    std::cerr << "ERROR: cannot open output file" << std::endl;
    exit(1);
  }

  for (int iroc = 1; iroc != 4; ++iroc) {
    for (int icol = 13; icol <= 38; ++icol) {
      for (int irow = 40; irow <= 79; ++irow) {
        for(int i = Begin; i < End; ++i) {
          X[i-Begin] = i;
          Y[i-Begin] = GainCal.GetCharge(22, iroc, icol, irow, i);
        }

        sprintf(BUFF, "GC_ROC%i_Col%02i_Row%02i", iroc, icol, irow);
        TGraph G(N, X, Y);
        G.SetName(BUFF);
        G.SetTitle(BUFF);
        G.Write();
      }
    }
  }

  f.Close();

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [GainCalFileName]" << std::endl;
    return 1;
  }

  TString const GainCalFileName = argv[1];

  TestGainCal(GainCalFileName);

  return 0;
}
