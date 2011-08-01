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

    float LResidualX (size_t const);
    float LResidualY (size_t const);
    std::pair<float, float> LResiduals (PLTCluster&, PLTAlignment&);

    bool IsFiducial (PLTPlane*, PLTAlignment&);
    bool IsFiducial (int const, int const, PLTAlignment&);

    float TX (float const);
    float TY (float const);

  private:
    std::vector<PLTCluster*> fClusters;

    // Vector in *telescope* coords
    float fTVX;
    float fTVY;
    float fTVZ;

    // Origin in *telescope* coords as defined by ROC-0
    float fTOX;
    float fTOY;
    float fTOZ;

    // Residuals for each roc in X and Y in terms of pixels
    float fLResidualX[3];
    float fLResidualY[3];

};








#endif
