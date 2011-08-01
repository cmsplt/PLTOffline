#ifndef GUARD_PLTTextFileReader_h
#define GUARD_PLTTextFileReader_h

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>

#include "PLTHit.h"



class PLTTextFileReader
{
  public:
    PLTTextFileReader ();
    PLTTextFileReader (std::string const);
    ~PLTTextFileReader ();

    bool Open (std::string const);

    int  GetEvent ();
    int  ReadEventHits (std::vector<PLTHit>&, unsigned long&);
    int  ReadEventHits (std::ifstream&, std::vector<PLTHit>&, unsigned long&);


  private:
    std::string fFileName;
    std::ifstream fInfile;
};



#endif
