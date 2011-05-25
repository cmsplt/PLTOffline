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

  private:
    std::vector<PLTHit*> fHits;

};




#endif
