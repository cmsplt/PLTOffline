////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Jun 20 10:07:44 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include "TRandom.h"

int MakeTrimCalFileFake ()
{
  FILE* f = fopen("FakeTrimCalFast.dat", "w");

  for (int icol = 13; icol <= 39; ++icol) {
    for (int irow = 40; irow <= 79; ++irow) {
      for (int iroc = 0; iroc != 3; ++iroc) {
        for (int itrim = 0; itrim != 16; ++itrim) {
          fprintf(f, "%1i %1i %2i %2i %2i %1i %3i %2i %3i %9.4f\n",
              8, 2, 29, icol, irow, iroc, 40, itrim, 9, gRandom->Gaus(0.5, 2));
        }
      }
    }
  }
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  MakeTrimCalFileFake();

  return 0;
}
