// A very simple program to test the functionality of the PLTTimestampReader class.

#include <iostream>
#include "PLTTimestampReader.h"

int main(const int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " timestampFile" << std::endl;
    return 1;
  }

  PLTTimestampReader ts(argv[1]);
  ts.printTimestamps();

  return 0;
}
