#include "PLTCluster.h"


PLTCluster::PLTCluster ()
{
}


PLTCluster::~PLTCluster ()
{
}



void PLTCluster::AddHit (PLTHit* Hit)
{
  fHits.push_back(Hit);
  return;
}


float PLTCluster::Charge ()
{
  float Sum = 0;
  for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
    Sum += (*it)->Charge();
  }
  return Sum;
}
