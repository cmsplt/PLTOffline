#ifndef GUARD_PLTPlane_h
#define GUARD_PLTPlane_h

#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "PLTCluster.h"

#include "TH2F.h"


class PLTPlane
{
  public:
    PLTPlane ();
    PLTPlane (int const, int const);
    ~PLTPlane ();

    void    AddHit (PLTHit*);
    float   Charge ();
    int     Channel ();
    bool    AddClusterFromSeed (PLTHit*);
    bool    IsBiggestHitIn3x3 (PLTHit*, bool const);
    int     NNeighbors (PLTHit*);
    void    Clusterize (bool const);
    TH2F*   DrawHits2D ();
    size_t  NHits ();
    PLTHit* Hit (size_t);
    int     ROC ();
    size_t  NClusters ();
    PLTCluster* Cluster (size_t);
    float   TZ ();


    void SetChannel (int const);
    void SetROC (int const);

  private:
    int fChannel;
    int fROC;
    std::vector<PLTHit*> fHits;
    std::vector<PLTHit*> fClusterizedHits;
    std::vector<PLTCluster*> fClusters;

};










#endif
