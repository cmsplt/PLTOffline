#ifndef GUARD_PLTBinaryFileReader_h
#define GUARD_PLTBinaryFileReader_h

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "PLTHit.h"



class PLTBinaryFileReader
{
  public:
    PLTBinaryFileReader ();
    PLTBinaryFileReader (std::string const);
    ~PLTBinaryFileReader ();

    bool Open (std::string const);

    int  GetEvent ();
    int  convPXL (int);
    bool DecodeSpyDataFifo (unsigned long, std::vector<PLTHit>&);
    int  ReadEventHits (std::vector<PLTHit>&, unsigned long&);
    int  ReadEventHits (std::ifstream&, std::vector<PLTHit>&, unsigned long&);


  private:
    std::string fFileName;
    std::ifstream fInfile;
};



#endif
