#ifndef GUARD_PLTPlane_h
#define GUARD_PLTPlane_h

#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "PLTCluster.h"
#include "PLTU.h"

#include "TH2F.h"


class PLTPlane
{
  public:
    PLTPlane ();
    PLTPlane (int const, int const);
    ~PLTPlane ();

    // Which clustering alg do you want to use?
    enum Clustering {
      kClustering_Seed_3x3,
      kClustering_Seed_5x5,
      kClustering_Seed_9x9,
      kClustering_NNeighbors,
      kClustering_AllTouching,
      kClustering_NoClustering
    };

    // Which seeding alg to use?
    enum ClusterSeed {
      kClusterSeed_Charge,
      kClusterSeed_NNeighbors
    };

    // Define the good region of the diamond we will consider hits from
    enum FiducialRegion {
      kFiducialRegion_All,
      kFiducialRegion_Diamond,
      kFiducialRegion_m1_m1,
      kFiducialRegion_m2_m2,
      kFiducialRegion_m3_m3,
      kFiducialRegion_m4_m4,
      kFiducialRegion_m5_m5
    };

    void    AddHit (PLTHit*);
    float   Charge ();
    int     Channel ();
    bool    AddClusterFromSeedNxN (PLTHit*, int const, int const);
    bool    IsBiggestHitInNxN (PLTHit*, int const, int const);
    int     NNeighbors (PLTHit*);
    void    Clusterize (Clustering const, FiducialRegion const);
    void    ClusterizeFromSeedNxN (int const, int const, FiducialRegion const);
    void    AddAllHitsTouching (PLTCluster*, PLTHit*, FiducialRegion const);
    void    ClusterizeAllTouching (FiducialRegion const);
    TH2F*   DrawHits2D ();
    size_t  NHits ();
    PLTHit* Hit (size_t);
    int     ROC ();
    size_t  NClusters ();
    PLTCluster* Cluster (size_t);
    size_t  NUnclusteredHits ();
    PLTHit* UnclusteredHit (size_t);
    float   TZ ();
    float   GZ ();
    static bool CompareChargeReverse (PLTHit*, PLTHit*);

    static bool IsFiducial (FiducialRegion const, PLTHit*);
    static bool IsFiducial (FiducialRegion const, int const, int const);

    void SetChannel (int const);
    void SetROC (int const);


  private:
    int fChannel;
    int fROC;
    std::vector<PLTHit*> fHits;
    std::vector<PLTHit*> fClusterizedHits;
    std::vector<PLTHit*> fUnclusteredHits;
    std::vector<PLTCluster*> fClusters;

};










#endif
