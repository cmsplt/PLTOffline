////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri Jun 24 20:20:40 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PLTDACs.h"


int TestDACs ()
{
  PLTDAC D("DACS", "mFec8_mFecChannel2_hubAddress29_roc0.dacs1");
  PLTDACs DD;
  DD.LoadAllDACSFiles();
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  TestDACs();

  return 0;
}
