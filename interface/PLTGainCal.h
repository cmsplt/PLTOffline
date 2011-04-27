#ifndef GUARD_PLTGainCal_h
#define GUARD_PLTGainCal_h

#include <iostream>
#include <fstream>
#include <sstream>

#include "TString.h"
#include "TMath.h"


class PLTGainCal
{
  public:
    PLTGainCal (TString const GainCalFileName);
    ~PLTGainCal ();


    static int const DEBUGLEVEL = 0;

    float GetCharge(int const ch, int const roc, int const col, int const row, int adc);
    void ReadGainCalFile (TString const GainCalFileName);

    int RowIndex (int const);
    int ColIndex (int const);
    int ChIndex (int const);
    int RocIndex (int const);

    float GetCoef(int const, int const, int const, int const, int const);



  private:
    static int const MAXCHNS =  48;
    static int const MAXROWS = 100;
    static int const MAXCOLS = 100;
    static int const MAXROCS =   3;

    static int const NCHNS =  48;
    static int const NROWS =  40;
    static int const NCOLS =  26;
    static int const NROCS =   3;

    static int const IROWMIN = 40;
    static int const IROWMAX = 79;
    static int const ICOLMIN = 13;
    static int const ICOLMAX = 38;

    // ch,roc,col,row [3]
    //float GC[MAXCHNS][MAXROCS][MAXCOLS][MAXROWS][3];
    float GC[NCHNS][NROCS][NCOLS][NROWS][3];

};















#endif
