#include "PLTPlane.h"


PLTPlane::PLTPlane ()
{
  // Make me, I dare you
}


PLTPlane::PLTPlane (int const Channel, int const ROC)
{
  // Con me
  fChannel = Channel;
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
  // Add a hit.. stored as pointers.. I am not the owner...you are
  fHits.push_back(Hit);
  fChannel = Hit->Channel();
  fROC = Hit->ROC();
  return;
}


float PLTPlane::Charge ()
{
  // Compute the charge on the entire plane
  double Sum = 0;
  for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
    Sum += (*it)->Charge();
  }
  return (float) Sum;
}



int PLTPlane::Channel ()
{
  // Which channel is this coming from?
  return fChannel;
}



bool PLTPlane::AddClusterFromSeed (PLTHit* Hit)
{
  // Add a cluster from a seed using some very basic clustering.  Not perfect, but better one
  // on the way

  // Clustering currently does not work so well.  This needs to be fixed.

  // New cluster
  PLTCluster* Cluster = new PLTCluster();

  // Sanity check
  if ( std::count(fClusterizedHits.begin(), fClusterizedHits.end(), Hit) ) {
    std::cerr << "ERROR: seed has already been used" << std::endl;
    return false;
  }

  // Add the seed
  Cluster->AddHit(Hit);

  // Check all buddy hits within 3x3
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (abs(fHits[i]->Row() - Hit->Row()) == 1 && abs(fHits[i]->Column() - Hit->Column()) == 1) {
      if ( std::count(fClusterizedHits.begin(), fClusterizedHits.end(), fHits[i]) == 0 ) {
        Cluster->AddHit(fHits[i]);
        fClusterizedHits.push_back(fHits[i]);
      }
    }
  }

  // Better add it to the list so we don't forget to delete
  fClusters.push_back(Cluster);

  return true;
}


bool PLTPlane::IsBiggestHitIn3x3(PLTHit* Hit, bool const IsGainCalGood)
{
  // Just check if a hit is the biggest in a 3x3 around itself

  // If we have a gaincal do this by charge, otherwise do this by number of neighbors
  if (IsGainCalGood) {
    for (size_t i = 0; i != fHits.size(); ++i) {
      if (abs(fHits[i]->Row() - Hit->Row()) == 1 && abs(fHits[i]->Column() - Hit->Column()) <= 1 && fHits[i]->Charge() > Hit->Charge()) {
        return false;
      }
      if (abs(fHits[i]->Column() - Hit->Column()) == 1 && abs(fHits[i]->Row() - Hit->Row()) <= 1 && fHits[i]->Charge() > Hit->Charge()) {
        return false;
      }
    }
  } else {
    // this has a flaw, and that is when there is a 3x4 or larger.. you get screwed
    for (size_t i = 0; i != fHits.size(); ++i) {
      if (abs(fHits[i]->Row() - Hit->Row()) == 1 && abs(fHits[i]->Column() - Hit->Column()) <= 1 && NNeighbors(fHits[i]) >  NNeighbors(fHits[i])) {
        return false;
      }
      if (abs(fHits[i]->Column() - Hit->Column()) == 1 && abs(fHits[i]->Row() - Hit->Row()) <= 1 &&  NNeighbors(fHits[i]) >  NNeighbors(fHits[i])) {
        return false;
      }
    }
  }

  return true;
}



int PLTPlane::NNeighbors (PLTHit* Hit)
{
  int N = 0;
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (abs(fHits[i]->Row() - Hit->Row()) == 1 && abs(fHits[i]->Column() - Hit->Column()) <= 1) {
      ++N;
    } else if (abs(fHits[i]->Column() - Hit->Column()) == 1 && abs(fHits[i]->Row() - Hit->Row()) <= 1) {
      ++N;
    }
  }

  return N;
}



void PLTPlane::Clusterize (bool const IsGainCalGood)
{
  // Loop over hits and find biggest..then use as seeds..
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (IsBiggestHitIn3x3(fHits[i], IsGainCalGood)) {
      AddClusterFromSeed(fHits[i]);
    }
  }

  return;
}



float PLTPlane::TZ ()
{
  // Get the z-coord of this plane in the telescope
  return Cluster(0)->TZ();
}



TH2F* PLTPlane::DrawHits2D ()
{
  // Draw the plane

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
  // Number of hits
  return fHits.size();
}


PLTHit* PLTPlane::Hit (size_t const i)
{
  // get a specific hit
  return fHits[i];
}


size_t PLTPlane::NClusters ()
{
  return fClusters.size();
}


PLTCluster* PLTPlane::Cluster (size_t const i)
{
  return fClusters[i];
}

int PLTPlane::ROC ()
{
  // Which roc is this
  return fROC;
}


void PLTPlane::SetChannel (int const in)
{
  fChannel = in;
  return;
}


void PLTPlane::SetROC (int const in)
{
  fROC = in;
  return;
}
