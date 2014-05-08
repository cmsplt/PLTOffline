////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Sun Jun 12 16:39:14 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTAlignment.h"

int TestAlignment (std::string const InFileName)
{
  PLTAlignment A;
  A.ReadAlignmentFile(InFileName);
  A.GetCP(1, 5);
  //A.WriteAlignmentFile("data/Alignment_Test.dat");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  std::string const InFileName = argv[1];

  TestAlignment(InFileName);

  return 0;
}
