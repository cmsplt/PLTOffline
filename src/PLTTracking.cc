#include "PLTTracking.h"

PLTTracking::PLTTracking ()
{
  // Default constructor
}


PLTTracking::PLTTracking (PLTAlignment* Alignment, TrackingAlgorithm const Algorithm)
{
  SetTrackingAlignment(Alignment);
  SetTrackingAlgorithm(Algorithm);
}


PLTTracking::~PLTTracking ()
{
  // Destructor!
}


void PLTTracking::SetTrackingAlignment (PLTAlignment* Alignment)
{
  if (!Alignment) {
    std::cerr << "ERROR: PLTTracking::SetTrackingAlignment(): Alignment object does not exist: is 0x0" << std::endl;
    exit(1);
  }

  fAlignment = Alignment;
  return;
}


void PLTTracking::SetTrackingAlgorithm (TrackingAlgorithm const Algorithm)
{
  fTrackingAlgorithm = Algorithm;
  return;
}


int PLTTracking::GetTrackingAlgorithm ()
{
  return fTrackingAlgorithm;
}



void PLTTracking::RunTracking (PLTTelescope& Telescope)
{
  switch (fTrackingAlgorithm) {
    case kTrackingAlgorithm_NoTracking:
      break;
    case kTrackingAlgorithm_01to2_All:
      TrackFinder_01to2_All(Telescope);
      break;
    default:
      std::cerr << "ERROR: PLTTracking::RunTracking() has no idea what tracking algorithm you want to use" << std::endl;
      throw;
  }

  return;
}


bool PLTTracking::CompareTrackD2 (PLTTrack* lhs, PLTTrack* rhs)
{
  return lhs->D2() < rhs->D2();
}



void PLTTracking::TrackFinder_01to2_All (PLTTelescope& Telescope)
{
  // Find tracks in this telescope which are more or less parallel to beam
  // Beamspot assumed to be at 0,0,0.

  // Check that tracks haven't already been filled
  if (Telescope.NTracks() != 0) {
    std::cerr << "ERROR: It looks like tracks have already been filled here: PLTTelescope::TrackFinderParallelTracks()" << std::endl;
    return;
  }

  // If there isn't a hit in each plane skip it for now
  if (Telescope.HitPlaneBits() != 0x7) {
    return;
  }

  std::vector<int> UsedHits;

  // Shorthand for each plane
  PLTPlane* P0 = Telescope.Plane(0);
  PLTPlane* P1 = Telescope.Plane(1);
  PLTPlane* P2 = Telescope.Plane(2);

  if (P0->NClusters() == 0 || P1->NClusters() == 0 || P2->NClusters() == 0) {
    return;
  }

  // Vector to keep track of tracks that we're interested in
  std::vector<PLTTrack*> MyTracks;

  // Start seeding with clusters in the 0th plane
  for (size_t iCL0 = 0; iCL0 != P0->NClusters(); ++iCL0) {


    // Loop over two other planes/clusters.  Pick a cluster in 1st plane,
    // and see if any patch behind in the 2nd plane.  One can rank these by Chi2 and pick best
    for (size_t iCL1 = 0; iCL1 != P1->NClusters(); ++iCL1) {

      PLTTrack Track01;
      Track01.AddCluster(P0->Cluster(iCL0));
      Track01.AddCluster(P1->Cluster(iCL1));
      Track01.MakeTrack(*fAlignment);

      float const ProjectionX2 = Track01.TX(P2->TZ());
      float const ProjectionY2 = Track01.TY(P2->TZ());

      for (size_t iCL2 = 0; iCL2 != P2->NClusters(); ++iCL2) {
        float const X = P2->Cluster(iCL2)->TX();
        float const Y = P2->Cluster(iCL2)->TY();

        float const ResidualX = ProjectionX2 - X;
        float const ResidualY = ProjectionY2 - Y;

        float const Distance = sqrt(ResidualX*ResidualX + ResidualY*ResidualY);

        if (DEBUG) {
          printf("Distance: %12.3E\n", Distance);
        }


        // If it's not too far off, keep it!
        //        if (Distance < 0.2000) {
          // Keep as possible track..
          PLTTrack* Track012 = new PLTTrack();
          Track012->AddCluster(P0->Cluster(iCL0));
          Track012->AddCluster(P1->Cluster(iCL1));
          Track012->AddCluster(P2->Cluster(iCL2));
          Track012->MakeTrack(*fAlignment);
          MyTracks.push_back(Track012);
          //        }

      }
    }

  }

  int const NTracksBefore = (int) MyTracks.size();

  // Grab the best tracks first and don't let there be overlap..
  SortOutTracksNoOverlapBestD2(MyTracks);

  if (DEBUG) {
    printf("Found NTracks possible: %4i   Kept NTracks: %4i\n", NTracksBefore, (int) MyTracks.size());
  }

  for (size_t i = 0; i != MyTracks.size(); ++i) {
    Telescope.AddTrack(MyTracks[i]);
  }

  return;
}







void PLTTracking::SortOutTracksNoOverlapBestD2 (std::vector<PLTTrack*>& MyTracks)
{
  // Idea of this function is to start with the tracks with the best test-stat
  // and grab those tracks first..  then for the remaining tracks only keep them
  // if they have unique clusters.

  // Order tracks with lowest test statistic first
  std::sort(MyTracks.begin(), MyTracks.end(), &PLTTracking::CompareTrackD2);

  // Containers for tracks and clsters
  std::vector<PLTTrack*> UsedTracks;
  std::vector<PLTTrack*> SkippedTracks;
  std::set<PLTCluster*> UsedClusters;

  // Loop over all tracks so far
  for (std::vector<PLTTrack*>::iterator Track = MyTracks.begin(); Track != MyTracks.end(); ++Track) {
    if ( UsedClusters.count((*Track)->Cluster(0)) == 0 &&
         UsedClusters.count((*Track)->Cluster(1)) == 0 &&
         UsedClusters.count((*Track)->Cluster(2)) == 0) {

      // If it's a track with all unused clusters so far let's keep it
      UsedClusters.insert((*Track)->Cluster(0));
      UsedClusters.insert((*Track)->Cluster(1));
      UsedClusters.insert((*Track)->Cluster(2));
      UsedTracks.push_back(*Track);
    } else {
      // This track has a cluster which has already been used, remember it
      // so we can delete it later
      SkippedTracks.push_back(*Track);
    }
  }

  // Replace input vector of tracks with accepted tracks
  MyTracks = UsedTracks;

  // Delete the unused tracks
  for (size_t i = 0; i != SkippedTracks.size(); ++i) {
    delete SkippedTracks[i];
  }

  return;
}
