////////////////////////////////////////////////////////////////////
//
// DumpErrors
// Paul Lujan
// April 19, 2017
//
// A simple script to test the new PLTError functionality by
// dumping the errors in an input file.
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
//#/include <string>
//#include <map>
//#include <numeric>

#include "PLTEvent.h"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE

// CODE BELOW

int DumpErrors(const std::string DataFileName) {
  // Set up the PLTEvent reader
  PLTEvent Event(DataFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 50000 == 0) {
      int nsec = Event.Time()/1000;
      int hr = nsec/3600;
      int min = (nsec-(hr*3600))/60;
      int sec = nsec % 60;
      std::cout << "Processing event: " << ientry << " at " << std::setfill('0') << std::setw(2)
		<< hr << ":" << std::setw(2) << min << ":" << std::setw(2) << sec << "."
		<< std::setw(3) << Event.Time()%1000 << std::endl;
    }
    //if (ientry > 10000) break;

    const std::vector<PLTError>& errors = Event.GetErrors();
    if (errors.size() > 0) {
      std::cout << "Event " << Event.EventNumber() << " number of errors: " << errors.size() << std::endl;
      for (std::vector<PLTError>::const_iterator it = errors.begin(); it != errors.end(); ++it) {
	it->Print();
      }
    }
  }

  return 0;
}


int main (int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  DumpErrors(DataFileName);

  return 0;
}
