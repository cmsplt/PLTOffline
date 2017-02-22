////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Oct 18 10:03:56 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <stdio.h>

int MakeStraightAlignment (std::string const FileName)
{
  FILE* f = fopen(FileName.c_str(), "w");
  if (!f) {
    std::cerr << "ERROR: cannot open file for writing: " << FileName << std::endl;
    throw;
  }

  fprintf(f, "# numbers are:\n");
  fprintf(f, "# Channel   Plane   CWLRotation   LXTrans  LYTrans  LZTrans\n\n");
  for (int ich = 1; ich <= 36; ++ich) {
    fprintf(f, "%2i  %2i ", ich, -1);
    fprintf(f, "    %12E", 0.);
    fprintf(f, "    %12E", 0.);
    fprintf(f, "    %12E", 0.);
    fprintf(f, "    %12E", 0.);
    fprintf(f, "    %12E", 0.);
    fprintf(f, "\n");
    for (int iroc = 0; iroc != 3; ++iroc) {
      fprintf(f, "%2i  %2i ", ich, iroc);
      fprintf(f, "                ");
      fprintf(f, "    %12E", 0.);
      fprintf(f, "    %12E", 0.);
      fprintf(f, "    %12E", 0.);
      fprintf(f, "    %12E", 3.77 * iroc);
      fprintf(f, "\n");
    }
    fprintf(f, "\n");
  }



  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [OutFileName]" << std::endl;
    return 1;
  }

  MakeStraightAlignment(argv[1]);

  return 0;
}
