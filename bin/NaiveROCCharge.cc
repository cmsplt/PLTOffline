////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Apr  4 16:23:17 CDT 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include "TFile.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TMath.h"

#include "PLTEvent.h"





int NaiveROCCharge (std::string const GainCalFileName, std::string const DataFileName)
{
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;
  std::cout << "DataFileName:    " << DataFileName << std::endl;


  unsigned long ievent = 0;

  PLTEvent Event(DataFileName, GainCalFileName);

  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (Event.NHits() > 0) {
      printf("Event: %12lu  Hits:%5i\n", Event.EventNumber(), (int) Event.NHits());
    }
  }



  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [GainCalFileName] [DataFileName]" << std::endl;
    return 1;
  }

  std::string const GainCalFileName = argv[1];
  std::string const DataFileName = argv[2];
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  NaiveROCCharge(GainCalFileName, DataFileName);

  return 0;
}
