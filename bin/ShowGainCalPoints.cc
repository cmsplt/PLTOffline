////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu Apr 21 16:27:50 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>


#include "TGraph.h"
#include "TCanvas.h"


int ShowGainCalPoints (std::string const InFileName, int const InmFec, int const InmFecChannel, int const InHubAddress, int const Inroc, int const Incol, int const Inrow)
{

  std::ifstream f(InFileName.c_str());
  if (!f) {
    std::cerr << "ERROR; Cannot open file: " << InFileName << std::endl;
    throw;
  }

  int const NMaxPoints = 200;
  double ADC [NMaxPoints];
  double VCAL[NMaxPoints];

  std::stringstream s;

  int mFec, mFecChannel, hubAddress, roc, col, row;
  float adc, vcal;

  int i = 0;
  for (std::string line; std::getline(f, line); ) {
    s.clear();
    s.str(line);
    s >> mFec
      >> mFecChannel
      >> hubAddress
      >> roc
      >> col
      >> row
      >> adc
      >> vcal;

    if (mFec == InmFec && mFecChannel == InmFecChannel && hubAddress == InHubAddress && roc == Inroc && col == Incol && row == Inrow) {
      ADC[i]  = adc;
      VCAL[i] = vcal;
      ++i;
      if (i == NMaxPoints) {
        break;
      }
    }
  }

  TGraph g(i, ADC, VCAL);
  TCanvas c;
  c.cd();
  g.Draw("a*");
  c.SaveAs("plots/GainCalPlot.eps");


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 8) {
    std::cerr << "Usage: " << argv[0] << " [InFileName] [mFec] [mFecChan] [hubAdd] [roc] [col] [row]" << std::endl;
    return 1;
  }

  std::string InFileName = argv[1];
  int const mFec = atoi(argv[2]);
  int const mFecChannel = atoi(argv[3]);
  int const hubAddress = atoi(argv[4]);
  int const roc = atoi(argv[5]);
  int const col = atoi(argv[6]);
  int const row = atoi(argv[7]);

  ShowGainCalPoints(InFileName, mFec, mFecChannel, hubAddress, roc, col, row);

  return 0;

}
