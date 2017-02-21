#ifndef GUARD_PLTGainCal_h
#define GUARD_PLTGainCal_h

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

#include "TString.h"
#include "TMath.h"
#include "TF1.h"

#include "PLTHit.h"
#include "PLTCluster.h"
#include "PLTPlane.h"
#include "PLTU.h"






class PLTGainCal
{
  public:
    PLTGainCal ();
    PLTGainCal (std::string const, int const NParams = 5);
    ~PLTGainCal ();


    static int const DEBUGLEVEL = 0;

    void  SetCharge (PLTHit&);
    float GetCharge(int const ch, int const roc, int const col, int const row, int adc);
    void  ReadGainCalFile (std::string const GainCalFileName);
    void  ReadGainCalFile3 (std::string const GainCalFileName);
    void  ReadGainCalFile5 (std::string const GainCalFileName);
    void  ReadGainCalFileExt (std::string const GainCalFileName);
    void  ReadTesterGainCalFile (std::string const GainCalFileName);

    void CheckGainCalFile (std::string const GainCalFileName, int const Channel);

    void PrintGainCal5 ();

    static int RowIndex (int const);
    static int ColIndex (int const);
    static int ChIndex (int const);
    static int RocIndex (int const);

    float GetCoef(int const, int const, int const, int const, int const);

    bool IsGood () { return fIsGood; }

    int GetHardwareID (int const);
    int GetFEDChannel(int mFec, int mFecCh, int hubId);

  private:
    bool fIsGood;
    bool fIsExternalFunction;

    int  fNParams; // how many parameters for this gaincal
    TF1 fFitFunction;

    static int const MAXCHNS =   36;
    static int const MAXROWS =   80;
    static int const MAXCOLS =   52;
    static int const MAXROCS =    3;

    static int const NCHNS =  36;
    static int const NROWS =  PLTU::NROW;
    static int const NCOLS =  PLTU::NCOL;
    static int const NROCS =   3;


    // ch,roc,col,row [3]
    //float GC[MAXCHNS][MAXROCS][MAXCOLS][MAXROWS][3];
    typedef float GCOnTheHeap[NROCS][NCOLS][NROWS][6];
    GCOnTheHeap* GC;

    // Map for hardware locations by fed channel
    std::map<int, int> fHardwareMap;

};















#endif
