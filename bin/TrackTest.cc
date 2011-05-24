////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue May 24 10:38:26 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"

// FUNCTION DEFINITIONS HERE
int TrackTest (std::string const);







// CODE BELOW


int TrackTest (std::string const DataFileName, std::string const GainCalFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName);

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      for (size_t ip = 0; ip != Telescope->NPlanes(); ++ip) {

        // THIS plane is
        PLTPlane* Plane = Telescope->Plane(ip);

        // Loop over all hits on this plane
        for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {

          // THIS hit is
          PLTHit* Hit = Plane->Hit(ihit);

          // Just print some example info
          printf("Channel: %2i  ROC: %1i  col: %2i  row: %2i  adc: %3i  Charge: %9.3E\n", Telescope->Channel(), Plane->ROC(), Hit->Column(), Hit->Row(), Hit->ADC(), Hit->Charge());
        }
      }
    }
  }

  return 0;
}


int main (int argc, char* argv[])
{

  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat] [GainCal.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  TrackTest(DataFileName, GainCalFileName);

  return 0;
}
