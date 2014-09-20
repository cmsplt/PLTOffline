#ifndef GUARD_PSIBinaryFileReader_h
#define GUARD_PSIBinaryFileReader_h

#include <fstream>
#include <set>

#include "PLTTelescope.h"
#include "PLTGainCal.h"
#include "PSIGainInterpolator.h"
#include "PLTAlignment.h"
#include "PLTTracking.h"

class PSIBinaryFileReader : public PLTTelescope, public PLTTracking
{
  public:
    PSIBinaryFileReader (std::string const, int const );
    PSIBinaryFileReader (std::string const, 
			 std::string const, 
			 std::string const,
			 int const );
    ~PSIBinaryFileReader ();

    std::string fBinaryFileName;
    bool OpenFile ();
    void ResetFile ();
    void Clear ();

    void ReadPixelMask (std::string const);
    void AddToPixelMask( int, int, int, int);
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

    PLTAlignment* GetAlignment()
    {
      return &fAlignment;
    }

    const std::set<int> * GetPixelMask(){
      return &fPixelMask;
    }

    long long GetTime () {return fTime;}


  private:
    int fHeader;
    int fNextHeader;
    const int  NMAXROCS;
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
    std::vector< std::vector< float > > fLevelsROC;
    std::set<int> fPixelMask;
    std::vector<PLTHit*> fHits;

    PLTGainCal fGainCal;
    PSIGainInterpolator fGainInterpolator;
    PLTAlignment fAlignment;

    std::map<int, PLTPlane> fPlaneMap;

    std::string fBaseCalDir;
    std::string fCalibrationFile[4];
    std::string fRawCalibrationFile[4];

};












#endif
