////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri Jul 15 15:28:52 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"

int PrintEventNumbers (std::string const FileName)
{
  // Setup PLTEvent object
  PLTEvent Event(FileName);

  while (Event.GetNextEvent() >= 0) {
    std::cout << Event.EventNumber() << std::endl;
  }


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [FileName]" << std::endl;
    return 1;
  }

  std::string const FileName = argv[1];

  PrintEventNumbers(FileName);

  return 0;
}
