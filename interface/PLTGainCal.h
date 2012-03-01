#ifndef GUARD_PLTGainCal_h
#define GUARD_PLTGainCal_h

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "TString.h"
#include "TMath.h"

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

    void CheckGainCalFile (std::string const GainCalFileName, int const Channel);

    void PrintGainCal5 ();

    int RowIndex (int const);
    int ColIndex (int const);
    int ChIndex (int const);
    int RocIndex (int const);

    float GetCoef(int const, int const, int const, int const, int const);

    bool IsGood () { return fIsGood; }



  private:
    bool fIsGood;

    int  fNParams; // how many parameters for this gaincal

    static int const MAXCHNS =  48;
    static int const MAXROWS = 100;
    static int const MAXCOLS = 100;
    static int const MAXROCS =   3;

    static int const NCHNS =  36;
    static int const NROWS =  40;
    static int const NCOLS =  26;
    static int const NROCS =   3;

    static int const IROWMIN = 39;
    static int const IROWMAX = 79;
    static int const ICOLMIN = 13;
    static int const ICOLMAX = 38;

    // ch,roc,col,row [3]
    //float GC[MAXCHNS][MAXROCS][MAXCOLS][MAXROWS][3];
    float GC[NCHNS][NROCS][NCOLS][NROWS][5];

};















#endif
