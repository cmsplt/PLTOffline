#ifndef GUARD_PLTBinaryFileReader_h
#define GUARD_PLTBinaryFileReader_h

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdint.h>
#include <set>

#include "PLTHit.h"
#include "PLTPlane.h"
#include "PLTGainCal.h"
#include "PLTError.h"

typedef enum InputTypeEnum {
  kBinaryFile,
  kTextFile,
  kBuffer
} InputType;

class PLTBinaryFileReader
{
  public:
    PLTBinaryFileReader ();
    PLTBinaryFileReader (std::string const, InputType inputType = kBinaryFile);
    ~PLTBinaryFileReader ();

    bool Open (std::string const);
    bool OpenBinary (std::string const);
    bool OpenTextFile (std::string const);
    void SetInputType (InputType inputType);

    int  convPXL (int);
    bool DecodeSpyDataFifo (uint32_t, std::vector<PLTHit*>&, std::vector<PLTError>&, std::vector<int>&);
    int  ReadEventHits (uint32_t*, uint32_t, std::vector<PLTHit*>&, std::vector<PLTError>&, unsigned long&, uint32_t&, uint32_t&, std::vector<int>&);
    int  ReadEventHitsBinary (std::vector<PLTHit*>&, std::vector<PLTError>&, unsigned long&, uint32_t&, uint32_t&, std::vector<int>&);
    int  ReadEventHitsText (std::vector<PLTHit*>&, unsigned long&);
    int  ReadEventHitsBuffer (uint32_t*, uint32_t, std::vector<PLTHit*>&, std::vector<PLTError>&, unsigned long&, uint32_t&, uint32_t&, std::vector<int>&);

    void ReadPixelMask (std::string const);
    void ReadOnlinePixelMask(const std::string maskFileName, const PLTGainCal& gainCal);
    bool IsPixelMasked (int const);

    void SetPlaneFiducialRegion (PLTPlane::FiducialRegion);

    PLTPlane::FiducialRegion fPlaneFiducialRegion;
    const std::set<int>& PixelMask(){return fPixelMask;}

  private:
    std::string fFileName;
    std::ifstream fInfile;
    InputType fInputType;
    uint32_t fLastTime;
    int fTimeMult;
    int fFEDID;

    std::set<int> fPixelMask;
};



#endif
