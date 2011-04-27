////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Apr 18 15:44:44 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PLTGainCal.h"

int GetParamsFromFile (TString const GainCalFileName, int const ch, int const roc, int const col, int const row)
{
  PLTGainCal GainCal(GainCalFileName);
  printf("a/b/c: %f   %f   %f\n",
      GainCal.GetCoef(0, ch, roc, col, row),
      GainCal.GetCoef(1, ch, roc, col, row),
      GainCal.GetCoef(2, ch, roc, col, row)
      );
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 6) {
    std::cerr << "Usage: " << argv[0] << " [File] [ch] [roc] [col] [row]" << std::endl;
    return 1;
  }

  TString const GainCalFileName = argv[1];
  int const ch  = atoi(argv[2]);
  int const roc = atoi(argv[3]);
  int const col = atoi(argv[4]);
  int const row = atoi(argv[5]);

  GetParamsFromFile(GainCalFileName, ch, roc, col, row);

  return 0;
}
