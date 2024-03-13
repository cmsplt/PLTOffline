////////////////////////////////////////////////////////////////////
//
//  MeasureMissRate -- a utility to attempt to simulate the
//    measurement of the "missing triplet" rate using the Slink data.
//    This tries several different algorithms for what the ROCs might
//    consider to be adjacent columns in the data to try to reproduce
//    the actual results that we see from the transparent buffer data.
//    
//    Paul Lujan, December 2, 2015
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <iomanip>
#include <assert.h>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TFile.h"
#include "PLTTimestampReader.h"

int MeasureMissRate(const std::string DataFileName, const std::string TimestampFileName) {
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  if (TimestampFileName != "")
    std::cout << "TimestampFileName: " << TimestampFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();
  gStyle->SetOptStat(1111);

  // Read timestamp file, if it exists.
  bool useTimestamps = false;
  int nSteps = 1;
  PLTTimestampReader *tsReader = nullptr;
  std::vector<std::pair<uint32_t, uint32_t> > timestamps;
  if (TimestampFileName != "") {
    tsReader = new PLTTimestampReader(TimestampFileName);
    timestamps = tsReader->getTimestamps();
    nSteps = tsReader->getSize();
    useTimestamps = true;
  }

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  //Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering);
  PLTPlane::FiducialRegion MyFiducialRegion = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(MyFiducialRegion);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  std::vector<int> nEvents(nSteps);
  // Four different algorithms for handling adjacent DCs.
  // algorithm 1 (dumbest) -- ignore adjacent DCs; all DCs are counted
  // algorithm 2 -- a chunk of N adjacent DCs is treated as ceil(N/2) hits
  // algorithm 3 (what Krishna uses, and also what the documentation that
  // Andreas found claims is the actual algorithm) -- a chunk of N adjacent
  // DCs is treated as 1 hit regardless of N
  // algorithm 4 -- the DCs themselves are divided into pairs. If both DCs in
  // a pair are hit then it only counts as 1, but if two adjacent DCs in different
  // pairs (e.g. 1/2) are hit then it counts as 2.
  std::vector<int> nTriplesAlg1(nSteps);
  std::vector<int> nTriplesMissedAlg1(nSteps);
  std::vector<int> nTriplesAlg2(nSteps);
  std::vector<int> nTriplesMissedAlg2(nSteps);
  std::vector<int> nTriplesAlg3(nSteps);
  std::vector<int> nTriplesMissedAlg3(nSteps);
  std::vector<int> nTriplesAlg4(nSteps);
  std::vector<int> nTriplesMissedAlg4(nSteps);

  int stepNumber = 0;
  uint32_t currentStepStart = 0;
  // const uint32_t stepLength = 60000; // 1 minute
  const uint32_t stepLength = 300000; // 5 minutes
  
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 100000 == 0) {
      int nsec = Event.Time()/1000;
      int hr = nsec/3600;
      int min = (nsec-(hr*3600))/60;
      int sec = nsec % 60;
      std::cout << "Processing event: " << ientry << " at " << std::setfill('0') << std::setw(2)
		<< (hr%24) << ":" << std::setw(2) << min << ":" << std::setw(2) << sec << "."
		<< std::setw(3) << Event.Time()%1000 << std::endl;
    }
    //if (ientry>=2000000){break;}

    if (useTimestamps) {
      if (Event.Time() > tsReader->lastTime()) break;
      stepNumber = tsReader->findTimestamp(Event.Time());
      if (stepNumber == -1) continue;
    } else {
      if (stepNumber == 0 && currentStepStart == 0) currentStepStart = Event.Time();
      if ((Event.Time() - currentStepStart) > stepLength) {
	//std::cout << "starting new step @" << Event.Time() << " from " << currentStepStart << std::endl;
	timestamps.push_back(std::make_pair(currentStepStart, Event.Time()-1));
	nEvents.push_back(0);
	nTriplesAlg1.push_back(0);
	nTriplesMissedAlg1.push_back(0);
	nTriplesAlg2.push_back(0);
	nTriplesMissedAlg2.push_back(0);
	nTriplesAlg3.push_back(0);
	nTriplesMissedAlg3.push_back(0);
	nTriplesAlg4.push_back(0);
	nTriplesMissedAlg4.push_back(0);
	currentStepStart = Event.Time();
	stepNumber++;
	nSteps++;
      }
    }
    nEvents[stepNumber]++;

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      
      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      // Loop over planes in scope.

      assert(Telescope->NPlanes() == 3);
      int nGoodPlanesAlg1 = 0, nGoodPlanesAlg2 = 0, nGoodPlanesAlg3 = 0, nGoodPlanesAlg4 = 0;
      int nBadPlanesAlg1 = 0, nBadPlanesAlg2 = 0, nBadPlanesAlg3 = 0, nBadPlanesAlg4 = 0;
      for (size_t iPlane = 0; iPlane < 3; ++iPlane) {
	PLTPlane* plane = Telescope->Plane(iPlane);
	
	std::vector<int> hitDCs;
	// loop over hits
	for (size_t iHit = 0; iHit < plane->NHits(); ++iHit) {
	  PLTHit* hit = plane->Hit(iHit);
	  int thisDC = hit->Column()/2;

	  // If we haven't seen this DC yet, add it to the array.
	  if (std::find(hitDCs.begin(), hitDCs.end(), thisDC) == hitDCs.end()) {
	    hitDCs.push_back(thisDC);
	  }
	} // loop over hits
      
	std::sort(hitDCs.begin(), hitDCs.end());

	// Simple algorithm is simple!
	if (hitDCs.size() > 0 && hitDCs.size() < 3) nGoodPlanesAlg1++;
	if (hitDCs.size() >= 3) nBadPlanesAlg1++;

	// Complicated algorithms are complicated!
	int nNonAdjacentDCsAlg2 = 0;
	int nNonAdjacentDCsAlg3 = 0;
	int nNonAdjacentDCsAlg4 = 0;
	int lastDC = -99;
	bool lastCounted = true;
	for (std::vector<int>::const_iterator it = hitDCs.begin(); it != hitDCs.end(); ++it) {
	  if ((*it) == lastDC+1) {
	    // IS adjacent to last DC. Doesn't count at all for alg3. Only counts every other
	    // one for alg2. Counts only if this column is EVEN for alg4.
	    if (lastCounted == false) {
	      nNonAdjacentDCsAlg2++;
	    }
	    if (*it % 2 == 0) nNonAdjacentDCsAlg4++;
	    lastCounted = !lastCounted;
	  } else {
	    // is NOT adjacent to last DC. Counts for alg3 and alg2.
	    nNonAdjacentDCsAlg2++;
	    nNonAdjacentDCsAlg3++;
	    nNonAdjacentDCsAlg4++;
	    lastCounted = true;
	  }
	  lastDC = *it;
	} // loop over hit DCs

// 	static int i=0;
// 	if (i<15 && hitDCs.size()>3) {
// 	  std::cout << "Hit DCs: ";
// 	  for (std::vector<int>::const_iterator it = hitDCs.begin(); it != hitDCs.end(); ++it) {
// 	    std::cout << *it << " ";
// 	  }
// 	  std::cout << std::endl << "N=" << hitDCs.size() << ";" << nNonAdjacentDCsAlg2
// 		    << ";" << nNonAdjacentDCsAlg3 << ";" << nNonAdjacentDCsAlg4 << std::endl;
// 	  ++i;
// 	}

	if (nNonAdjacentDCsAlg2 > 0 && nNonAdjacentDCsAlg2 < 3) nGoodPlanesAlg2++;
	if (nNonAdjacentDCsAlg2 >= 3) nBadPlanesAlg2++;

	if (nNonAdjacentDCsAlg3 > 0 && nNonAdjacentDCsAlg3 < 3) nGoodPlanesAlg3++;
	if (nNonAdjacentDCsAlg3 >= 3) nBadPlanesAlg3++;

	if (nNonAdjacentDCsAlg4 > 0 && nNonAdjacentDCsAlg4 < 3) nGoodPlanesAlg4++;
	if (nNonAdjacentDCsAlg4 >= 3) nBadPlanesAlg4++;
      }

      if (nGoodPlanesAlg1 == 3) nTriplesAlg1[stepNumber]++;
      else if (nGoodPlanesAlg1 + nBadPlanesAlg1 == 3) nTriplesMissedAlg1[stepNumber]++;
      if (nGoodPlanesAlg2 == 3) nTriplesAlg2[stepNumber]++;
      else if (nGoodPlanesAlg2 + nBadPlanesAlg2 == 3) nTriplesMissedAlg2[stepNumber]++;
      if (nGoodPlanesAlg3 == 3) nTriplesAlg3[stepNumber]++;
      else if (nGoodPlanesAlg3 + nBadPlanesAlg3 == 3) nTriplesMissedAlg3[stepNumber]++;
      if (nGoodPlanesAlg4 == 3) nTriplesAlg4[stepNumber]++;
      else if (nGoodPlanesAlg4 + nBadPlanesAlg4 == 3) nTriplesMissedAlg4[stepNumber]++;

    }

  } // event loop
  // properly catch the last step
  timestamps.push_back(std::make_pair(currentStepStart, Event.Time()));

  // write out results
  FILE *outf = fopen("MissingHitsPixel.txt", "w");
  fprintf(outf, "%d\n", nSteps);
  for (int i=0; i<nSteps; ++i) {
    std::cout << " == step " << i << " ==" << std::endl;
    std::cout << "Alg1: " << nTriplesAlg1[i] << " good and " << nTriplesMissedAlg1[i] << " missed (";
    if (nTriplesMissedAlg1[i] > 0)
      std::cout << (float)nTriplesMissedAlg1[i]*100.0/nTriplesAlg1[i] << "%)" << std::endl;
    else
      std::cout << "--)" << std::endl;
    std::cout << "Alg2: " << nTriplesAlg2[i] << " good and " << nTriplesMissedAlg2[i] << " missed (";
    if (nTriplesMissedAlg2[i] > 0)
      std::cout << (float)nTriplesMissedAlg2[i]*100.0/nTriplesAlg2[i] << "%)" << std::endl;
    else
      std::cout << "--)" << std::endl;
    std::cout << "Alg3: " << nTriplesAlg3[i] << " good and " << nTriplesMissedAlg3[i] << " missed (";
    if (nTriplesMissedAlg3[i] > 0)
      std::cout << (float)nTriplesMissedAlg3[i]*100.0/nTriplesAlg3[i] << "%)" << std::endl;
    else
      std::cout << "--)" << std::endl;
    std::cout << "Alg4: " << nTriplesAlg4[i] << " good and " << nTriplesMissedAlg4[i] << " missed (";
    if (nTriplesMissedAlg4[i] > 0)
      std::cout << (float)nTriplesMissedAlg4[i]*100.0/nTriplesAlg4[i] << "%)" << std::endl;
    else
      std::cout << "--)" << std::endl;
    fprintf(outf, "%d %d %d %d %d %d %d %d %d %d %d\n", timestamps[i].first, timestamps[i].second,
            nEvents[i], nTriplesAlg1[i], nTriplesMissedAlg1[i], nTriplesAlg2[i], nTriplesMissedAlg2[i],
	    nTriplesAlg3[i], nTriplesMissedAlg3[i], nTriplesAlg4[i], nTriplesMissedAlg4[i]);
  }
  fclose(outf);
  
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: " << argv[0] << " DataFile.dat [TimestampFile.dat]" << std::endl;
    return 1;
  }

  const std::string DataFileName = argv[1];
  const std::string TimestampFileName = (argc == 3) ? argv[2] : "";

  MeasureMissRate(DataFileName, TimestampFileName);

  return 0;
}
