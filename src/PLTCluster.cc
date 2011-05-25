#include "PLTCluster.h"


PLTCluster::PLTCluster ()
{
  // Make it real good
}


PLTCluster::~PLTCluster ()
{
  // Makeun me
}



void PLTCluster::AddHit (PLTHit* Hit)
{
  // Add a hit
  fHits.push_back(Hit);
  return;
}


float PLTCluster::Charge ()
{
  // Compute charge of this cluster
  float Sum = 0;
  for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
    Sum += (*it)->Charge();
  }
  return Sum;
}


size_t PLTCluster::NHits ()
{
  return fHits.size();
}


PLTHit* PLTCluster::Hit (size_t const i)
{
  return fHits[i];
}
