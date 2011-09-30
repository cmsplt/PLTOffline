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



bool PLTPlane::AddClusterFromSeedNxN (PLTHit* Hit, int const mCol, int const mRow)
{
  // Add a cluster from a seed using some very basic clustering.  Not perfect, but better one
  // on the way

  // Clustering currently does not work so well.  This needs to be fixed.

  // Sanity check
  if ( std::count(fClusterizedHits.begin(), fClusterizedHits.end(), Hit) ) {
    //std::cerr << "ERROR: seed has already been used" << std::endl;
    //std::cerr << Hit->Column() << " " << Hit->Row() << std::endl;
    return false;
  }

  // New cluster
  PLTCluster* Cluster = new PLTCluster();

  // Add the seed
  Cluster->AddHit(Hit);
  fClusterizedHits.push_back(Hit);

  // Which clustering do you want?
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (fHits[i]->Row() == Hit->Row() && fHits[i]->Column() == Hit->Column()) {
      continue;
    }
    if (abs(fHits[i]->Row() - Hit->Row()) <= mRow && abs(fHits[i]->Column() - Hit->Column()) <= mCol) {
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


bool PLTPlane::IsBiggestHitInNxN(PLTHit* Hit, int const mRow, int const mCol)
{
  // Just check if a hit is the biggest in a 3x3 around itself

  // If we have a gaincal do this by charge, otherwise do this by number of neighbors
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (fHits[i]->Row() == Hit->Row() && fHits[i]->Column() == Hit->Column()) {
      continue;
    }
    if (abs(fHits[i]->Row() - Hit->Row()) <= mRow && abs(fHits[i]->Column() - Hit->Column()) <= mCol && fHits[i]->Charge() > Hit->Charge()) {
      return false;
    }
  }

  return true;
}



int PLTPlane::NNeighbors (PLTHit* Hit)
{
  int N = 0;
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (fHits[i]->Row() == Hit->Row() && fHits[i]->Column() == Hit->Column()) {
      continue;
    }
    if (abs(fHits[i]->Row() - Hit->Row()) <= 2 && abs(fHits[i]->Column() - Hit->Column()) <= 2) {
      ++N;
    }
  }

  return N;
}



void PLTPlane::Clusterize (Clustering const Clust)
{
  // Cluster hits given one of the methods.
  // I sort hits here so that the largest charge hits are picked up first.
  // Use that if you want, otherwise unsorted..

  switch (Clust) {
    case kClustering_Seed_3x3:
      std::sort(fHits.begin(), fHits.end(), PLTPlane::CompareChargeReverse);
      ClusterizeFromSeedNxN(1, 1);
      break;
    case kClustering_Seed_5x5:
      std::sort(fHits.begin(), fHits.end(), PLTPlane::CompareChargeReverse);
      ClusterizeFromSeedNxN(2, 2);
      break;
    case kClustering_NNeighbors:
      //ClusterizeNNeighbors();
      std::cerr << "PLTPlane::ClusterizeNNeighbors not written yet" << std::endl;
      throw;
      break;
    case kClustering_AllTouching:
      ClusterizeAllTouching();
      break;
    case kClustering_NoClustering:
      // Dont to any clustering
      break;
    default:
      std::cerr << "ERROR in PLTPlane::Clusterize: no such clustering alg exists" << std::endl;
      return;
  }

  return;
}



void PLTPlane::ClusterizeFromSeedNxN (int const mCol, int const mRow)
{

  // Loop over hits and find biggest..then use as seeds..
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (IsBiggestHitInNxN(fHits[i], mCol, mRow)) {
      AddClusterFromSeedNxN(fHits[i], mCol, mRow);
    }
  }
}



void PLTPlane::AddAllHitsTouching (PLTCluster* Cluster, PLTHit* Hit)
{
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (std::find(fClusterizedHits.begin(), fClusterizedHits.end(), fHits[i]) != fClusterizedHits.end()) {
      continue;
    }
    if (fHits[i] == Hit) {
      continue;
    }

    if ( abs(fHits[i]->Row() - Hit->Row()) <= 1 && abs(fHits[i]->Column() - Hit->Column()) <= 1) {
      Cluster->AddHit(fHits[i]);
      fClusterizedHits.push_back(fHits[i]);
      AddAllHitsTouching(Cluster, fHits[i]);
    }
  }

  return;
}


void PLTPlane::ClusterizeAllTouching ()
{
  // Loop over hits and find biggest..then use as seeds..
  for (size_t i = 0; i != fHits.size(); ++i) {
    if (std::find(fClusterizedHits.begin(), fClusterizedHits.end(), fHits[i]) != fClusterizedHits.end()) {
      continue;
    }
    PLTCluster* Cluster = new PLTCluster();
    Cluster->AddHit(fHits[i]);
    fClusterizedHits.push_back(fHits[i]);
    AddAllHitsTouching(Cluster, fHits[i]);
    fClusters.push_back(Cluster);
  }

  return;
}



float PLTPlane::TZ ()
{
  // Get the z-coord of this plane in the telescope
  return Cluster(0)->TZ();
}



bool PLTPlane::CompareChargeReverse (PLTHit* a, PLTHit* b)
{
  return a->Charge() > b->Charge();
}



bool PLTPlane::IsFiducial (FiducialRegion const FidR, PLTHit* Hit)
{
  switch (FidR) {
    case kFiducialRegion_All:
      return true;
      break;
    case kFiducialRegion_Diamond:
      if (Hit->Row() >= PLTU::FIRSTROW &&
          Hit->Row() <= PLTU::LASTROW &&
          Hit->Column() >= PLTU::FIRSTCOL &&
          Hit->Column() <= PLTU::LASTCOL) {
        return true;
      }
      break;
    case kFiducialRegion_m2_m2:
      if (Hit->Row() >= PLTU::FIRSTROW + 2 &&
          Hit->Row() <= PLTU::LASTROW - 2 &&
          Hit->Column() >= PLTU::FIRSTCOL + 2 &&
          Hit->Column() <= PLTU::LASTCOL - 2) {
        return true;
      }
      break;
    default:
      std::cerr << "ERROR in PLTPlane::IsFiducial" << std::endl;
      return false;
  };
  return false;
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
