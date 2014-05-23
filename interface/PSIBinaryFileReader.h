#ifndef GUARD_PSIBinaryFileReader_h
#define GUARD_PSIBinaryFileReader_h

#include <fstream>
#include <set>

#include "PLTTelescope.h"
#include "PLTGainCal.h"
#include "PLTAlignment.h"
#include "PLTTracking.h"

class PSIBinaryFileReader : public PLTTelescope, public PLTTracking
{
  public:
    PSIBinaryFileReader (std::string const);
    PSIBinaryFileReader (std::string const, std::string const);
    ~PSIBinaryFileReader ();

    std::string fBinaryFileName;
    bool OpenFile ();
    void Clear ();

    void ReadPixelMask (std::string const);
    bool IsPixelMasked (int const);

    bool ReadAddressesFromFile (std::string const);


    unsigned short readBinaryWordFromFile ();
    int nextBinaryHeader ();
    int decodeBinaryData ();
    int GetNextEvent ();
    int CalculateLevels (int const NMaxEvents = 10000, TString const OutDir = "plots/");
    int LevelInfo (int const Value, int const iroc);
    std::pair<int, int> fill_pixel_info(int* evt , int ctr, int iroc);
    void DecodeHits ();

    void DrawTracksAndHits (std::string const);
    void DrawWaveform(TString const);

    size_t NHits ();
    PLTHit* Hit (size_t);

    PLTGainCal* GetGainCal ()
    {
      return &fGainCal;
    }



  private:
    int fHeader;
    int fNextHeader;
    static int const NMAXROCS = 6;
    static int const MAXNDATA = 2000;
    int fBuffer[MAXNDATA];
    int fBufferSize;
    bool fEOF;
    std::ifstream fInputBinaryFile;
    unsigned int fUpperTime;
    unsigned int fLowerTime;
    long long fTime;
    int fData[MAXNDATA];
    static int const UBLevel = -680;
    float fLevelsROC[NMAXROCS][6];
    std::set<int> fPixelMask;
    std::vector<PLTHit*> fHits;

    PLTGainCal fGainCal;
    PLTAlignment fAlignment;

    std::map<int, PLTPlane> fPlaneMap;












};












#endif
