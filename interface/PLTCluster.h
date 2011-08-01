#ifndef GUARD_PLTCluster_h
#define GUARD_PLTCluster_h

#include "PLTHit.h"

#include <vector>

class PLTCluster
{
  public:
    PLTCluster ();
    ~PLTCluster ();

    void AddHit (PLTHit*);
    float Charge ();
    size_t NHits ();
    PLTHit* Hit (size_t const);
    PLTHit* SeedHit ();



    int PX ();
    int PY ();
    int PZ ();
    std::pair<int, int> PCenter ();
    int Channel ();
    int ROC ();

    // Local w.r.t. center of damond
    float LX ();
    float LY ();
    std::pair<float, float> LCenter ();

    // Telescope cords
    float TX ();
    float TY ();
    float TZ ();
    std::pair<float, float> TCenter ();

    // Global cords
    float GX ();
    float GY ();
    float GZ ();
    std::pair<float, float> GCenter ();


  private:
    std::vector<PLTHit*> fHits;  // The seed hit needs to be 0 in this vector

};




#endif
