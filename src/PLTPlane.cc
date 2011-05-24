#include "PLTPlane.h"


PLTPlane::PLTPlane ()
{
}


PLTPlane::~PLTPlane ()
{
  // The Clusters belong to the Plane so we need to delete them
  for (size_t i = 0; i != fClusters.size(); ++i) {
    delete fClusters[i];
  }
}



void PLTPlane::AddHit (PLTHit* Hit)
{
  fHits.push_back(Hit);
  fChannel = Hit->Channel();
  fROC = Hit->ROC();
  return;
}


float PLTPlane::Charge ()
{
  double Sum = 0;
  for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
    Sum += (*it)->Charge();
  }
  return Sum;
}



int PLTPlane::Channel ()
{
  return fChannel;
}



bool PLTPlane::AddClusterFromSeed (PLTHit* Hit)
{
  PLTCluster* Cluster = new PLTCluster();
  Cluster->AddHit(Hit);
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (abs(fHits[i]->Row() - Hit->Row()) == 1 && abs(fHits[i]->Column() - Hit->Column()) == 1) {
      Cluster->AddHit(fHits[i]);
    }
  }

  fClusters.push_back(Cluster);

  return true;
}



bool PLTPlane::IsBiggestHitIn3x3(PLTHit* Hit)
{
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (abs(fHits[i]->Row() - Hit->Row()) == 1 && abs(fHits[i]->Column() - Hit->Column()) <= 1 && fHits[i]->Charge() > Hit->Charge()) {
      return false;
    }
    if (abs(fHits[i]->Column() - Hit->Column()) == 1 && abs(fHits[i]->Row() - Hit->Row()) <= 1 && fHits[i]->Charge() > Hit->Charge()) {
      return false;
    }
  }

  return true;
}



void PLTPlane::Clusterize ()
{
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (IsBiggestHitIn3x3(fHits[i])) {
      AddClusterFromSeed(fHits[i]);
    }
  }
  return;
}



TH2F* PLTPlane::DrawHits2D ()
{
  TString Name = "Plane_Channel_";
  Name += fChannel;
  Name += "_ROC_";
  Name += fROC;
  TH2F* h = new TH2F(Name.Data(), Name.Data(), 30, 10, 40, 40, 40, 80);
  for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
    h->Fill((*it)->Column(), (*it)->Row(), (*it)->Charge());
  }
  return h;
}


size_t PLTPlane::NHits ()
{
  return fHits.size();
}


PLTHit* PLTPlane::Hit (size_t const i)
{
  return fHits[i];
}

int PLTPlane::ROC ()
{
  return fROC;
}

