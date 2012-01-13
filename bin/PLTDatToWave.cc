////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Jan 24 17:48:23 EST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "TString.h"
#include "TGraph.h"
#include "TCanvas.h"


int PLTDatToWave (int const N, TString const InputFileName)
{
  //int const kNUMBER = 205;
  int const kBEGIN  =  30;
  int const kNUMBER = 100;

  float Time[kNUMBER];
  float Value[kNUMBER];

  std::ifstream InFile(InputFileName.Data());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open file for reading: " << InputFileName << std::endl;
    throw;
  }

  std::string line;
  for (int i = 1; i < N && !InFile.eof(); ++i) {
    std::getline(InFile, line);
  }

  for (int i = 0; !InFile.eof(); ++i) {
    if (i < kBEGIN) {
      continue;
    } else if (i >= kBEGIN + kNUMBER) {
      break;
    }
    Time[i - kBEGIN] = (float) i - kBEGIN;
    InFile >> Value[i - kBEGIN];
  }

  TGraph gr(kNUMBER, Time, Value);
  TCanvas Can;
  Can.cd();
  gr.Draw("ac*");
  Can.SaveAs("plots/Wave.gif");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [number] [Input.dat] " << std::endl;
    return 1;
  }

  int const N = atoi(argv[1]);
  TString const InputFileName = argv[2];

  PLTDatToWave(N, InputFileName);

  return 0;
}
