#ifndef GUARD_PLTAlignment_h
#define GUARD_PLTAlignment_h

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>

#include "PLTHit.h"
#include "PLTCluster.h"
#include "PLTU.h"

class PLTAlignment
{
  public:
    PLTAlignment ();
    ~PLTAlignment ();

    void ReadAlignmentFile (std::string const);
    void PrintAlignmentFile (std::string const);
    void AlignHit (PLTHit&);
    bool IsGood ();

    float TtoLX (float const, float const, int const, int const);
    float TtoLY (float const, float const, int const, int const);
    std::pair<float, float> TtoLXY (float, float, int const, int const);


    void LtoTXYZ (std::vector<float>&, float const, float const, int const, int const);
    void TtoGXYZ (std::vector<float>&, float const, float const, float const, int const, int const);
    void GtoTXYZ (std::vector<float>&, float const, float const, float const, int const, int const);
    void VTtoVGXYZ (std::vector<float>&, float const, float const, float const, int const, int const);
    float GetTZ (int const, int const);

    float PXtoLX (int const);
    float PYtoLY (int const);


    int PXfromLX (float const);
    int PYfromLY (float const);
    std::pair<int, int> PXYfromLXY (std::pair<float, float> const&);
    std::pair<float, float> PXYDistFromLXYDist (std::pair<float, float> const&);


    // GtoT

    // Need a function for G->T, G->L
    // Need to make T in cm?  G in cm?
    // Function: IsTTrackFiducial(plane)



    //GetPXPY (LX, LY, ch, ROC);

    float LR  (int const, int const);
    float LX  (int const, int const);
    float LY  (int const, int const);
    float LZ  (int const, int const);
    float GRZ (int const, int const);
    float GRY (int const, int const);
    float GX  (int const, int const);
    float GY  (int const, int const);
    float GZ  (int const, int const);

    struct CP {
      // All constants refer to planes as though we were looking FROM the IP
      float LR;  // Local clockwise rotation
      float LX;  // Local X translation
      float LY;  // Local Y translation
      float LZ;  // Local Z translation
      float GRZ; // Global clockwise rotation about Z
      float GRY; // Global clockwise rotation about Y
      float GX;  // Global X translation
      float GY;  // Global Y translation
      float GZ;  // Global Z translation
    };

    CP* GetCP (int const, int const);
    CP* GetCP (std::pair<int, int> const&);

    std::vector< std::pair<int, int> > GetListOfChannelROCs ();

    // Mini struct to be used only in reading alignment file
    struct TelescopeAlignmentStruct {
      float GRZ, GRY, GX, GY, GZ;
    };



  private:
    std::map< std::pair<int, int>, CP > fConstantMap;
    bool fIsGood;

    static bool const DEBUG = false;

};









#endif
