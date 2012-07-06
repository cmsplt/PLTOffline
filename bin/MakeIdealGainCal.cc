////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu Mar 29 15:34:52 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTU.h"


int MakeIdealGainCal (std::string const OutFileName)
{
  FILE* f = fopen(OutFileName.c_str(), "w");
  if (!f) {
    std::cerr << "ERROR: cannot open file: " << OutFileName << std::endl;
    return 1;
  }

  int const NChannels = 36;

 
  for (int ich = 1; ich <= NChannels; ++ich) {
    fprintf(f, "%i %i %i %i\n", ich, ich, ich, ich);
  }

  fprintf(f, "\n");

  for (int ich = 1; ich <= NChannels; ++ich) {
    for (int col = PLTU::FIRSTCOL; col <= PLTU::LASTCOL; ++col) {
      for (int row = PLTU::FIRSTROW; row <= PLTU::LASTROW; ++row) {
        for (int roc = 0; roc <= 2; ++roc) {
          fprintf(f, "%2i %1i %2i %2i %15E %15E %15E %15E %15E\n",
              ich,
              roc,
              col,
              row,
               8.727799E-02,
              -2.689344E+01,
               2.104635E+03,
               2.276370E+02,
               5.369915E-01);
        }
      }
    }
  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [OutFile]" << std::endl;
    return 1;
  }

  MakeIdealGainCal(argv[1]);

  return 0;
}
