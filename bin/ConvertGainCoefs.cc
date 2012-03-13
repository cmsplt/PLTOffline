////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Jan 25 18:39:23 CET 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>


int ConvertGainCoefs (std::string const InFileName, std::string const OutFileName)
{
  std::ifstream InFile(InFileName.c_str());
  FILE* OutFile = fopen(OutFileName.c_str(), "w");

  // Valuse from file
  int mFec, mFecChannel, hubAddress, roc, col, row;
  float val;
  std::vector<float> Val;

  // Container for conversion
  std::map<int, int> M;
  //M[ mFec * 1000 + mFecChannel * 100 + hubAddress] = Channel;
  M[ 8229 ] = 20;
  M[ 8221 ] = 21;
  M[ 8213 ] = 23;
  M[ 8205 ] = 24;
  fprintf(OutFile, "8 2 29 20\n");
  fprintf(OutFile, "8 2 21 21\n");
  fprintf(OutFile, "8 2 13 23\n");
  fprintf(OutFile, "8 2  5 24\n");
  fprintf(OutFile, "\n");


  // stringstream to be used for each line of input data file
  std::stringstream s;

  // Loop over linesin the input data file
  for (std::string line; std::getline(InFile, line); ) {
    Val.clear();
    s.clear();
    s.str(line);
    s >> mFec
      >> mFecChannel
      >> hubAddress
      >> roc
      >> col
      >> row;
    while (!s.eof()) {
      s >> val;
      Val.push_back(val);
    }

    printf("%2i %1i %2i %2i\n", M[mFec*1000 + mFecChannel*100 + hubAddress], roc, col, row);
    fprintf(OutFile, "%2i %1i %2i %2i ", M[mFec*1000 + mFecChannel*100 + hubAddress], roc, col, row);
    for (std::vector<float>::iterator it = Val.begin(); it != Val.end(); ++it) {
      fprintf(OutFile, " %12E", *it);
    }
    fprintf(OutFile, "\n");
  }


  InFile.close();
  fclose(OutFile);

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFile] [OutFile]" << std::endl;
    return 1;
  }

  ConvertGainCoefs(argv[1], argv[2]);

  return 0;
}
