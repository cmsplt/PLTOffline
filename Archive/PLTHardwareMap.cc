#include "PLTHardwareMap.h"

#include <iostream>


PLTHardwareMap::PLTHardwareMap ()
{
  int id = 1000 * 8 + 100 * 2 + 29;
  fFEDFECMap[id] = 11;
}


PLTHardwareMap::~PLTHardwareMap ()
{
}


int PLTHardwareMap::GetFEDChannel (int const mFec, int const mFecChannel, int const hubAddress)
{
  int id = 1000 * mFec + 100 * mFecChannel + hubAddress;

  if (fFEDFECMap.count(id)) {
  return fFEDFECMap[id];
  }

  std::cerr << "ERROR: did not find fed channel in map: " << mFec << " " << mFecChannel << " " << hubAddress << std::endl;
  return 0;
}
