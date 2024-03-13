#ifndef GUARD_PLTTimestampReader_h
#define GUARD_PLTTimestampReader_h

// A class to hold functions for dealing with timestamp files. Construct it with the name of the timestamp
// file, and then call findTimestamp() to find which timestamp interval a given timestamp falls in (or -1 if
// it's not in any interval).

#include <string>
#include <vector>
#include <stdint.h>

class PLTTimestampReader {
public:
  PLTTimestampReader(const std::string infile);
  ~PLTTimestampReader();

  int findTimestamp(uint32_t t);
  std::vector<std::pair<uint32_t, uint32_t> > getTimestamps();
  unsigned int getSize();

  uint32_t firstTime();
  uint32_t lastTime();

  void printTimestamps();
  
private:
  std::vector<std::pair<uint32_t, uint32_t> > fTimestamps;
  uint32_t fFirstTime;
  uint32_t fLastTime;
};

#endif
