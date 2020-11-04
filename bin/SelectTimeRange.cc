// SelectTimeRange -- given a starting and ending time (in PLT format), this will take all the events between
// those times and write them out to a separate file. Based on TestBinaryFileReader.cc (but obviously much
// simplified).
// Note that this assumes you're running on a non-buggy file. -- PJL 2020-11-04

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <stdlib.h>

bool readEvent(std::ifstream& infile, std::ofstream& outfile, int startTime, int endTime) {
  static int fLastTime = -1;
  static int fTimeMult = 0;
  int eventTime;
  uint32_t n1=0, n2=0;
 
  bool bheader = true;

  std::vector<uint32_t> eventData;

  while (bheader) {
    // Read 64-bit word
    infile.read((char*) &n2, sizeof n2);
    infile.read((char*) &n1, sizeof n1);
    eventData.push_back(n2);
    eventData.push_back(n1);

    if (infile.eof()) {
      return false;
    }

    if ((n1 == 0x53333333) && (n2 == 0x53333333)) {
      //tdc buffer, special handling
      for (int ih = 0; ih < 100; ih++) {
	infile.read((char *) &n1, sizeof n1);
	if ((n1 & 0xf0000000) == 0xa0000000) {
	  infile.read((char *) &n2, sizeof n2);
	  break;
	}
	if (infile.eof()) {
	  return false;
	}
      }

    } else if ( ((n1 & 0xff000000) == 0x50000000 && (n2 & 0xff) == 0 ) || ((n2 & 0xff000000) == 0x50000000 && (n1 & 0xff) == 0)) {
      // Found header
      bheader = true;

      while (bheader) {
	infile.read((char *) &n2, sizeof n2);
	infile.read((char *) &n1, sizeof n1);
	eventData.push_back(n2);
	eventData.push_back(n1);

	if (infile.eof()) {
	  return false;
	}

	if ((n1 & 0xff000000) == 0xa0000000 || (n2 & 0xff000000) == 0xa0000000) {
	  // Trailer
	  bheader = false;
	  if ((n1 & 0xff000000) == 0xa0000000) {
	    eventTime = n2;
	  } else {
	    eventTime = n1;
	  }
	  if ((int)eventTime < fLastTime) {
	    ++fTimeMult;
	  }

	  fLastTime = eventTime;
	  eventTime = eventTime + 86400000 * fTimeMult;
	} else {
	  // Normal hit data, nothing to actually do here
	}
      } // inside the event
    }
  } // end of event

  // Now that we've read the whole event, see if it's actually one that we want to write out.
  if (eventTime > endTime)
    return false; // nope and we're past the end time, so we can stop
  if (eventTime < startTime)
    return true; // nope but keep going
  
  // yes (and keep going (of course)
  for (unsigned int i=0; i<eventData.size(); ++i) {
    outfile.write((char*)(&eventData[i]), sizeof(uint32_t));
  }
  return true;
}

int main(const int argc, const char** argv) {
  if (argc < 5) {
    std::cerr << "Usage: " << argv[0] << " <infile> <outfile> <start time> <end time>" << std::endl;
    exit(1);
  }
  const char *fileName = argv[1];
  const char *outFileName = argv[2];
  int startTime, endTime;
  std::stringstream startTimeStr(argv[3]);
  std::stringstream endTimeStr(argv[4]);
  startTimeStr >> startTime;
  endTimeStr >> endTime;

  std::cout << "Opening input file " << fileName << std::endl;
  std::ifstream infile(fileName, std::ios::in | std::ios::binary);
  if (!infile.is_open()) {
    std::cerr << "Error opening input file " << fileName << std::endl;
    exit(1);
  }

  std::cout << "Opening output file " << outFileName << std::endl;
  std::ofstream outfile(outFileName, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!outfile.is_open()) {
    std::cerr << "Error opening output file " << outFileName << std::endl;
    exit(1);
  }

  int nEvents = 0;
  while (!infile.eof()) {
    bool beforeEndTime = readEvent(infile, outfile, startTime, endTime);
    if (!beforeEndTime)
      break;
    nEvents++;
    if (nEvents % 100000 == 0)
      std::cout << "Processed " << nEvents << " events" << std::endl;
  }
  infile.close();
  outfile.close();
  std::cout << "Events written to " << outFileName << "." << std::endl;
}
