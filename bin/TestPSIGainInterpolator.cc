////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Aug  6 13:05:19 CEST 2014
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <cstdlib>
#include  <cstdio>

#include "PSIGainInterpolator.h"

int TestPSIGainInterpolator (std::string const InFileName, int const roc, int const col, int const row, int const adc)
{
  PSIGainInterpolator GI;
  GI.ReadFile(InFileName, 1);

  float Result = GI.GetLinearInterpolation(1, roc, col, row, adc);
  printf("adc: %6i   vcal: %8.2f\n", adc, Result);



  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 6) {
    std::cerr << "Usage: " << argv[0] << " [file] [roc] [col] [row] [adc]" << std::endl;
    return 1;
  }

  std::string const InFileName = argv[1];
  int const roc = atoi(argv[2]);
  int const col = atoi(argv[3]);
  int const row = atoi(argv[4]);
  int const adc = atoi(argv[5]);

  TestPSIGainInterpolator(InFileName, roc, col, row, adc);

  return 0;
}
