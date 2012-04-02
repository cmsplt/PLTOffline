#ifndef GUARD_PLTTrack_h
#define GUARD_PLTTrack_h

#include <vector>
#include <iostream>
#include <math.h>


#include "PLTCluster.h"
#include "PLTAlignment.h"
#include "PLTPlane.h"
#include "PLTU.h"


class PLTTrack
{
  public:
    PLTTrack ();
    ~PLTTrack ();

    void AddCluster (PLTCluster*);
    int  MakeTrack (PLTAlignment&);

    size_t NClusters ();
    size_t NHits ();

    PLTCluster* Cluster (size_t const);

    float LResidualX (size_t const);
    float LResidualY (size_t const);
    std::pair<float, float> LResiduals (PLTCluster&, PLTAlignment&);

    bool IsFiducial (PLTPlane*, PLTAlignment&, PLTPlane::FiducialRegion);
    bool IsFiducial (int const, int const, PLTAlignment&, PLTPlane::FiducialRegion);

    float TX (float const);
    float TY (float const);

    std::pair<float, float> GXYatGZ (float const, PLTAlignment&);

    float ChiSquare ();

  private:
    std::vector<PLTCluster*> fClusters;

  public:
    // Vector in *telescope* and *global* coords
    float fTVX;
    float fTVY;
    float fTVZ;

    float fGVX;
    float fGVY;
    float fGVZ;

    // Origin in *telescope* and *global* coords as defined by ROC-0
    float fTOX;
    float fTOY;
    float fTOZ;

    float fGOX;
    float fGOY;
    float fGOZ;

    // Where the track passes through the X=0(=0), Y=0(=1), and Z=0 planes
    // Three corrds just because that's easy enough 
    float fPlaner[3][3];

    // Residuals for each roc in X and Y in terms of pixels
    float fLResidualX[3];
    float fLResidualY[3];

    float fChiSquare;

    static bool const DEBUG = false;

};








#endif
