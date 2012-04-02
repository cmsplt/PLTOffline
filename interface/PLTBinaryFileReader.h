#ifndef GUARD_PLTBinaryFileReader_h
#define GUARD_PLTBinaryFileReader_h

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdint.h>

#include "PLTHit.h"
#include "PLTPlane.h"



class PLTBinaryFileReader
{
  public:
    PLTBinaryFileReader ();
    PLTBinaryFileReader (std::string const, bool const IsText = false);
    ~PLTBinaryFileReader ();

    bool Open (std::string const);
    bool OpenBinary (std::string const);
    bool OpenTextFile (std::string const);
    void SetIsText (bool const);



    int  convPXL (int);
    bool DecodeSpyDataFifo (uint32_t unsigned, std::vector<PLTHit*>&);
    int  ReadEventHits (std::vector<PLTHit*>&, unsigned long&, uint32_t unsigned&);
    int  ReadEventHits (std::ifstream&, std::vector<PLTHit*>&, unsigned long&, uint32_t unsigned&);
    int  ReadEventHitsText (std::ifstream&, std::vector<PLTHit*>&, unsigned long&, uint32_t unsigned&);

    void SetPlaneFiducialRegion (PLTPlane::FiducialRegion);

    PLTPlane::FiducialRegion fPlaneFiducialRegion;

  private:
    std::string fFileName;
    std::ifstream fInfile;
    bool fIsText;
};



#endif
