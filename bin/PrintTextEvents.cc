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
#include "PLTU.h"


int PrintTextEvents(std::string const DataFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;


  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    for (size_t iplane = 0; iplane != Event.NPlanes(); ++iplane) {
      PLTPlane* Plane = Event.Plane(iplane);
      for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {
        PLTHit* Hit = Plane->Hit(ihit);
        printf("%2i %1i %2i %2i %4i %12i\n", Hit->Channel(), Hit->ROC(), Hit->Column(), Hit->Row(), Hit->ADC(), Event.EventNumber());
      }
    }


  }






  return 0;
}


int main (int argc, char* argv[])
{

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];

  PrintTextEvents(DataFileName);

  return 0;
}
