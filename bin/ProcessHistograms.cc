////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Apr 10 19:48:45 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <fstream>

int const NBUCKETS = 3564;

int ProcessHistograms (std::string const InFileName)
{
  std::ifstream InFile(InFileName.c_str(), std::ios::in | std::ios::binary);
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    throw;
  }


  uint32_t OrbitTime;
  uint32_t Orbit;
  uint32_t Channel;
  uint32_t BigBuff[NBUCKETS];

  bool GotIt;

  while (true) {

  GotIt = false;
  while (!GotIt) {
    InFile.read( (char*) &OrbitTime, sizeof(uint32_t));
    if (InFile.eof()) {
      usleep(200000);
      InFile.clear();
    } else {
      GotIt = true;
    }
  }

  GotIt = false;
  while (!GotIt) {
    InFile.read( (char*) &Orbit, sizeof(uint32_t));
    if (InFile.eof()) {
      usleep(200000);
      InFile.clear();
    } else {
      GotIt = true;
    }
  }

  GotIt = false;
  while (!GotIt) {
    InFile.read( (char*) &Channel, sizeof(uint32_t));
    if (InFile.eof()) {
      usleep(200000);
      InFile.clear();
    } else {
      GotIt = true;
    }
  }


  int iBucket = 0;
  while (iBucket < NBUCKETS) {
    GotIt = false;
    while (!GotIt) {
      InFile.read( (char*) &BigBuff[iBucket], sizeof(uint32_t));
      if (InFile.eof()) {
        GotIt = false;
        usleep(200000);
        InFile.clear();
      } else {
        GotIt = true;
        ++iBucket;
      }
    }
  }

  uint64_t TotalInChannel = 0;
  for (int ib = 0; ib < NBUCKETS; ++ib) {
    TotalInChannel += (BigBuff[ib] & 0xfff);
  }

  printf("Channel: %2i  OrbitTime: %15lu  Orbit: %15lu   Total: %15lu\n", (int) Channel, (unsigned long) OrbitTime, (unsigned long) Orbit, (unsigned long) TotalInChannel);

  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  ProcessHistograms(argv[1]);

  return 0;
}
