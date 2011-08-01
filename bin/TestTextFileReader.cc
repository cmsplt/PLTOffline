////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Jun 22 10:44:26 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <vector>

#include "PLTHit.h"
#include "PLTTextFileReader.h"


int TestTextFileReader (std::string const InFileName)
{
  PLTTextFileReader TFR(InFileName);

  std::vector<PLTHit> Hits;

  unsigned long crap;

  for (int ientry = 0; TFR.ReadEventHits(Hits, crap) >= 0; ++ientry) {
    for (std::vector<PLTHit>::iterator Hit = Hits.begin(); Hit != Hits.end(); ++Hit) {
      printf("ientry: %10i     EventNumber: %10lu    NHits: %5i\n", ientry, crap, (int) Hits.size());
    }
    Hits.clear();
  }


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  std::string const InFileName = argv[1];

  TestTextFileReader(InFileName);

  return 0;
}
