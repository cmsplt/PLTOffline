////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri May 20 10:53:48 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>


int GetEventsFromBinary ()
{
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  GetEventsFromBinary();

  return 0;
}
