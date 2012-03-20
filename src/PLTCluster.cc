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
  return std::make_pair<int, int>(SeedHit()->Column(), SeedHit()->Row());
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
  return LCenterOfMass();
  //return std::make_pair<float, float>(SeedHit()->LX(), SeedHit()->LY());
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
  return GCenterOfMass();
  //return std::make_pair<float, float>(SeedHit()->GX(), SeedHit()->GY());
}


std::pair<float, float> PLTCluster::LCenterOfMass ()
{
  // Return the Local coords based on a charge weighted average of pixel hits

  // X, Y, and Total Charge
  float X = 0.0;
  float Y = 0.0;
  float ChargeSum = 0.0;

  // Loop over each hit in the cluster
  for (std::vector<PLTHit*>::iterator It = fHits.begin(); It != fHits.end(); ++It) {
    X += (*It)->LX() * (*It)->Charge();
    Y += (*It)->LY() * (*It)->Charge();
    ChargeSum += (*It)->Charge();
  }



  // Just for printing diagnostics
  //printf("LCenterOfMass: %12E  %12E\n", X / ChargeSum, Y / ChargeSum);

  // If charge sum is zero return average
  if (ChargeSum <= 0.0) {
    // std::cerr << "WARNING: ChargeSum <= 0 in PLTCluster::LCenterOfMass()" << std::endl;
    std::make_pair<float, float>(X / (float) NHits(), Y / (float) NHits());
  }

  return std::make_pair<float, float>(X / ChargeSum, Y / ChargeSum);
}



std::pair<float, float> PLTCluster::GCenterOfMass ()
{
  // Return the Global coords based on a charge weighted average of pixel hits

  // X, Y, and Total Charge
  float X = 0.0;
  float Y = 0.0;
  float ChargeSum = 0.0;

  // Loop over each hit in the cluster
  for (std::vector<PLTHit*>::iterator It = fHits.begin(); It != fHits.end(); ++It) {
    X += (*It)->GX() * (*It)->Charge();
    Y += (*It)->GY() * (*It)->Charge();
    ChargeSum += (*It)->Charge();
  }

  // If charge sum is zero or less return average
  if (ChargeSum <= 0.0) {
    //std::cerr << "WARNING: ChargeSum <= 0 in PLTCluster::GCenterOfMass()" << std::endl;
    std::make_pair<float, float>(X / (float) NHits(), Y / (float) NHits());
  }

  // Put fiducial warning here?

  // Just for printing diagnostics
  //printf("GCenterOfMass: %12E  %12E\n", X / ChargeSum, Y / ChargeSum);

  return std::make_pair<float, float>(X / ChargeSum, Y / ChargeSum);
}
