////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Sep 13 09:34:07 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTGainCal.h"

int CheckGainCalCoefs (std::string const GainCalFileName, int const Channel)
{
  PLTGainCal GC;

  GC.CheckGainCalFile(GainCalFileName, Channel);

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [GainCalFileName] [Channel]" << std::endl;
    return 1;
  }

  std::string const GainCalFileName = argv[1];
  int const Channel = atoi(argv[2]);

  CheckGainCalCoefs(GainCalFileName, Channel);

  return 0;
}
