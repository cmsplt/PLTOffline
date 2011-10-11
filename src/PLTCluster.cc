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

PLTHit* PLTCluster::SeedHit ()
{
  return fHits[0];
}


int PLTCluster::PX ()
{
  return PCenter().first;
}


int PLTCluster::PY ()
{
  return PCenter().second;
}


int PLTCluster::PZ ()
{
  return SeedHit()->ROC();
}


std::pair<int, int> PLTCluster::PCenter ()
{
  return std::make_pair<int, int>(SeedHit()->Row(), SeedHit()->Column());
}


int PLTCluster::Channel ()
{
  return SeedHit()->Channel();
}


int PLTCluster::ROC ()
{
  return SeedHit()->ROC();
}




float PLTCluster::LX ()
{
  return LCenter().first;
}


float PLTCluster::LY ()
{
  return LCenter().second;
}



std::pair<float, float> PLTCluster::LCenter ()
{
  return std::make_pair<float, float>(SeedHit()->LX(), SeedHit()->LY());
}


float PLTCluster::TX ()
{
  return TCenter().first;
}


float PLTCluster::TY ()
{
  return TCenter().second;
}


float PLTCluster::TZ ()
{
  return SeedHit()->TZ();
}


std::pair<float, float> PLTCluster::TCenter ()
{
  return std::make_pair<float, float>(SeedHit()->TX(), SeedHit()->TY());
}

float PLTCluster::GX ()
{
  return GCenter().first;
}


float PLTCluster::GY ()
{
  return GCenter().second;
}


float PLTCluster::GZ ()
{
  return SeedHit()->GZ();
}


std::pair<float, float> PLTCluster::GCenter ()
{
  return std::make_pair<float, float>(SeedHit()->GX(), SeedHit()->GY());
}

