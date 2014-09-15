#ifndef GUARD_PLTTracking_h
#define GUARD_PLTTracking_h

#include <vector>
#include <iostream>
#include <set>
#include <math.h>


#include "PLTTelescope.h"
#include "PLTAlignment.h"
#include "PLTU.h"


class PLTTracking
{
  public:
    enum TrackingAlgorithm {
      kTrackingAlgorithm_NoTracking = 0,
      kTrackingAlgorithm_01to2_All,
      kTrackingAlgorithm_2PlaneTracks_All,
      kTrackingAlgorithm_6PlanesHit
    };

    PLTTracking ();
    PLTTracking (PLTAlignment*, TrackingAlgorithm const);
    ~PLTTracking ();

    void SetTrackingAlignment (PLTAlignment*);
    void SetTrackingAlgorithm (TrackingAlgorithm const);
    int  GetTrackingAlgorithm ();
    static bool CompareTrackD2 (PLTTrack*, PLTTrack*);

    void SetAllPlanes();  // 222222
    void SetPlaneUnderTest(int); // 330333
    void SetPlaneUnderTestSandwich(int); // 303000

    void RunTracking (PLTTelescope&);

    void TrackFinder_01to2_All (PLTTelescope&);
    void TrackFinder_6PlanesHit (PLTTelescope&);

    void SortOutTracksNoOverlapBestD2(std::vector<PLTTrack*>&);

    bool DoingSinglePlaneEfficiency(){ return fDoSinglePlaneEfficiency;};


  private:
    PLTAlignment* fAlignment;
    TrackingAlgorithm fTrackingAlgorithm;

    // Which planes to use for tracking
    // One int per plane (0..5)
    // Decoding:
    //  0: ignore plane for tracking (use for DUT)
    //  1: use plane for tracking (a cluster is optional)
    //  2: use plane for tracking (a cluster is required)
    //  3: use plane for tracking (exactle one cluster is required)
    // Initialize with default values for convenience 222222, can change using
    //  SetPlanesForTracking
    // The defaults require at least one hit per plane. For the standard analysis there
    //  is an additional step in the caller that makes sure exactly one hit per plane
    //  using the hitBit to speed things up.
    // For the planeEfficiency analysis this additional condition is not there and
    //  we check the single hit requirement in the tracking code directly.
    int fUsePlanesForTracking[6];
    bool fDoSinglePlaneEfficiency;

    static bool const DEBUG = false;
};



#endif
