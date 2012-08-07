////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Apr  2 12:48:21 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <fstream>

#include "PLTEvent.h"

int SkimEvents (std::string const DataFileName, std::string const OutFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Output file
  std::ofstream OutFile(OutFileName.c_str());
  if (!OutFile.is_open()) {
    std::cerr << "ERROR: cannot open output file: " << OutFileName << std::endl;
    return 1;
  }

  // Grab the plt event reader
  PLTEvent Event(DataFileName);

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    //if (ientry > 10000) break;

    Event.WriteEventText(OutFile);
  }

  OutFile.close();


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFileName] [OutFileName]" << std::endl;
    return 1;
  }

  SkimEvents(argv[1], argv[2]);

  return 0;
}
