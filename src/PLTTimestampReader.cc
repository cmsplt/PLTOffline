#include "PLTTimestampReader.h"
#include <iostream>
#include <fstream>
#include <sstream>

PLTTimestampReader::PLTTimestampReader(const std::string infile) {
  // Read in the input file.
  std::ifstream timestampFile(infile.c_str());
  if (!timestampFile.is_open()) {
    std::cerr << "Failed to open input file " << infile << std::endl;
    exit(1);
  }
  
  int maxTimestamps = -1;
  std::string line;
  while (1) {
    std::getline(timestampFile, line);
    if (timestampFile.eof()) break;
    if (line.empty()) continue; // skip blank lines
    if (line.at(0) == '#') continue; // skip comment lines

    // Break line into fields
    std::stringstream ss(line);
    std::string field;
    std::vector<std::string> fields;

    while (std::getline(ss, field, ' '))
      fields.push_back(field);

    if (fields.size() == 1 && fTimestamps.size() == 0) {
      // Allow for the presence of a header field containing the total number of timestamps in the file.
      std::stringstream headerString(fields[0]);
      headerString >> maxTimestamps;
    } else if (fields.size() >= 2) {
      // If there's more than two fields (e.g. a VdM scan file with separation information), we can just
      // ignore the rest.
      uint32_t firstTime, lastTime;
      std::stringstream firstTimeString(fields[0]);
      std::stringstream lastTimeString(fields[1]);
      firstTimeString >> firstTime;
      lastTimeString >> lastTime;

      if (firstTime <= lastTime) {
	fTimestamps.push_back(std::make_pair(firstTime, lastTime));
      } else {
	std::cerr << "Bad line in input file, start time is after end time! " << line << std::endl;
      }
    } else {
      std::cerr << "Malformed line in input file: " << line << std::endl;
    }
    if (maxTimestamps != -1 && (int)fTimestamps.size() >= maxTimestamps)
      break;
  }

  if (fTimestamps.size() == 0) {
    std::cerr << "Failed to read any timestamps from file!" << std::endl;
    exit(1);
  }

  if (maxTimestamps != -1 && (int)fTimestamps.size() < maxTimestamps) {
    std::cerr << "Warning: header specified " << maxTimestamps << " timestamps but only " << fTimestamps.size() << " found in file" << std::endl;
  }

  // Set the first and last time. The timestamps SHOULD be sorted, but let's make sure...
  fFirstTime = fTimestamps[0].first;
  fLastTime = fTimestamps[fTimestamps.size()-1].second;
  for (unsigned int i=0; i<fTimestamps.size(); ++i) {
    if (fTimestamps[i].first < fFirstTime) fFirstTime = fTimestamps[i].first;
    if (fTimestamps[i].second > fLastTime) fLastTime = fTimestamps[i].second;
  }

  timestampFile.close();
}

PLTTimestampReader::~PLTTimestampReader() {
}

// Find the timestamp interval corresponding to a given time, or -1 if not in any interval.
int PLTTimestampReader::findTimestamp(uint32_t t) {
  for (unsigned int i=0; i<fTimestamps.size(); ++i) {
    if (t >= fTimestamps[i].first && t <= fTimestamps[i].second)
      return i;
  }
  return -1;
}

// Earliest and latest time in the file.
uint32_t PLTTimestampReader::firstTime() { return fFirstTime; }
uint32_t PLTTimestampReader::lastTime() { return fLastTime; }

// Return vector of all timestamps.
std::vector<std::pair<uint32_t, uint32_t> > PLTTimestampReader::getTimestamps() { return fTimestamps; }

unsigned int PLTTimestampReader::getSize() { return fTimestamps.size(); }

// Print out contents of timestamp file.
void PLTTimestampReader::printTimestamps() {
  std::cout << fTimestamps.size() << " timestamps in file" << std::endl;
  for (unsigned int i=0; i<fTimestamps.size(); ++i)
    std::cout << "start: " << fTimestamps[i].first << " end: " << fTimestamps[i].second << std::endl;
}
