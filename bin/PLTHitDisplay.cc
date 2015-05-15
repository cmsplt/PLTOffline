////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Wed Apr 22 2015
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TEveTrack.h"
#include "TEvePointSet.h"
#include "TEveStraightLineSet.h"
#include <TColor.h>
#include <TRandom.h>

#include "TGeoManager.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoCompositeShape.h"
#include "TGeoPcon.h"
#include "TGeoPgon.h"
#include "TGeoCone.h"
#include "TGeoBoolNode.h"
#include "TGeoTube.h"
#include "TGeoArb8.h"
#include "TGeoTrd2.h"
#include "TGeoTorus.h"
#include "TEveTrackPropagator.h"
#include "TStyle.h"
#include "TEveManager.h"
#include "TGeoManager.h"
#include "TEveGeoNode.h"
#include "TSystem.h"
#include "TApplication.h"



//Setup the Geometry of hits and planes
void SetupGeometry (TGeoManager* GeoManager, PLTAlignment& Alignment, PLTEvent& Event)
{
  // Define some material and media
  TGeoMaterial *MatVacuum = new TGeoMaterial("Vacuum", 0,0,0);
  TGeoMaterial *MatAl = new TGeoMaterial("Al", 26.98,13,2.7);
  TGeoMedium *Vacuum = new TGeoMedium("Vacuum",1, MatVacuum);
  TGeoMedium *Al = new TGeoMedium("Root Material",2, MatAl);


  TGeoVolume *Top = GeoManager->MakeBox("TOP", Vacuum, 27., 27., 120.);
  GeoManager->SetTopVolume(Top);
  TGeoVolume *Replica = GeoManager->MakeBox("REPLICA", Vacuum,400,400,400);
  Replica->SetVisibility(kFALSE);
  TGeoVolume *CP= GeoManager->MakeBox("CP", Vacuum, 110., 50., 5.);
  CP->SetVisibility(kFALSE);
  /*
  // Setup plane geometry
  std::vector< std::pair<int, int> > ChannelROCs = Alignment.GetListOfChannelROCs();
  for (std::vector< std::pair<int, int> >::iterator It = ChannelROCs.begin(); It != ChannelROCs.end(); ++It) {
    PLTAlignment::CP* C = Alignment.GetCP(*It);
    
    // Aperture is defined by 2nd rock.
    double aperture = It->second == 1 ? 0. : 0.2;
    
    // Make a plane. Here, dx & dy are half lengths in X & Y
    TGeoVolume *plane = GeoManager->MakeBox("plane", Al, 0.4+aperture, 0.4+aperture, 0.005);    
    plane->SetLineColor(kBlue);
    
    // Translation and rotation of this plane in global space
    // Rotation angle comes from the alignment file: Tele. GRZ
    // C->GZ=171.41;
    // C->LZ=0(roc 0), 3.77 (roc1) or 7.54 (roc 2);    
    // C->LY=0(roc 0), 0.102 (roc1) or 0.204 (roc 2);        
    TGeoRotation    *Rotation = new TGeoRotation(TString::Format("RotationZ%i", 10*It->first + It->second), C->GRZ * 180. / TMath::Pi(), 0, 0.);    
    TGeoCombiTrans  *TransRot = new TGeoCombiTrans(C->GX+C->LX, C->GY+C->LY, C->GZ + C->LZ, Rotation);
    if (C->GRY < 1.0) {
      TransRot = new TGeoCombiTrans(C->GX+C->LX, C->GY+C->LY, (C->GZ + C->LZ), Rotation);
    } else {
      // uncomment the line below to get planes @ -Z
      //      TransRot = new TGeoCombiTrans(-C->GX, -C->GY, (-C->GZ - C->LZ), Rotation);
    }
    
    CP->AddNode(plane,  10*It->first + It->second, TransRot);
  }
  */
  Replica->AddNode(CP, 1);
  Top->AddNode(Replica, 1);
  
  //--- close the geometry
  GeoManager->CloseGeometry();
  GeoManager->SetVisLevel(4);

  // varialbes for tracks
  TEveTrackList *list = new TEveTrackList();
  TEveTrackPropagator* prop = list->GetPropagator();
  prop->SetFitDaughters(kFALSE);
  prop->SetMaxZ(185);

  TEveRecTrackD *rc = new TEveRecTrackD();
  TEveTrack *track = 0;
  std::map<int, int> NTrackMap;

  // variables hits
  TEveManager::Create();
  if (!gRandom)
    gRandom = new TRandom(0);
  TRandom& r= *gRandom;
  
  TEvePointSet* ps = new TEvePointSet();
  ps->SetOwnIds(kTRUE);
  
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    
//    // For the Tracks:
//    
//    // Loop over all planes with hits in event
//    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
//      
//      // THIS telescope is
//      PLTTelescope* Telescope = Event.Telescope(it);
//      
//      if (NTrackMap[Telescope->Channel()] > 30) continue;
//      
//      if(Telescope->Channel() > 24){
//        for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
//        PLTTrack* T = Telescope->Track(itrack);
//
//        std::pair<float, float> TrXY = T->GXYatGZ(0, Alignment);
//        rc->fV.Set(TrXY.first, TrXY.second, 0);
//        rc->fP.Set(T->fGVX, T->fGVY, T->fGVZ);
//        rc->fSign = 0;
//        
//        TEveTrack* track = new TEveTrack(rc, prop);
//        track->SetLineColor(0);
//        track->MakeTrack();
//        gEve->AddElement(track);
//        ++NTrackMap[Telescope->Channel()];
//        }
//      }
//      gEve->Redraw3D(kTRUE);
//      gSystem->ProcessEvents();
//
//    }
    
    // For Hits      
    // Loop over all planes with hits in event
    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {
           
      // THIS plane is
      PLTPlane* Plane = Event.Plane(ip);             
      if (Plane->ROC() > 2) {
        std::cerr << "WARNING: ROC > 2 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->ROC() < 0) {
        std::cerr << "WARNING: ROC < 0 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->Channel() > 99) {
        std::cerr << "WARNING: Channel > 99 found: " << Plane->Channel() << std::endl;
        continue;
      }
      
      for (size_t icluster = 0; icluster != Plane->NClusters(); ++icluster) {
        PLTCluster* Cluster = Plane->Cluster(icluster);
        
        // Loop over all hits on this plane          
        for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {
          
          // THIS hit is
          PLTHit* Hit = Plane->Hit(ihit);
          
          // ID thise plane and roc by 3 digit number          
          int const id = 10 * Plane->Channel() + Plane->ROC();
          // Only interested in +z/forward channels for now.
          if(Hit->Channel()<24){
              ps->SetNextPoint(Hit->GX(),Hit->GY(),Hit->GZ());
              ps->SetPointId(new TNamed(Form("Point %d", id), ""));
          }            
        }
      }
    }
  }
  
  ps->SetMarkerColor(TMath::Nint(r.Uniform(2, 5)));
  ps->SetMarkerSize(r.Uniform(0, 1));
  ps->SetMarkerStyle(0);
  
  gEve->AddElement(ps);  
  gEve->Redraw3D();
//std::cout << count << std::endl;  
    
  TGeoNode* node = GeoManager->GetTopNode();
  TEveGeoTopNode* en = new TEveGeoTopNode(GeoManager, node);
  en->SetVisLevel(4);
  en->GetNode()->GetVolume()->SetVisibility(kFALSE);
  
  gEve->AddGlobalElement(en);
  gEve->Redraw3D(kTRUE);
  gSystem->ProcessEvents();
  
  en->ExpandIntoListTreesRecursively();
  
  
  return;
}


//Setup events for geometry
int PLTHitDisplay (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)    
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName,false);
    //  PLTEvent Event(DataFileName);  

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_Diamond;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_m1_m1;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Geometry for Planes and the Hits.
  TGeoManager* GeoManager = new TGeoManager("PLT", "PLT Geometry");
  SetupGeometry(GeoManager, Alignment,Event);

  
  return 0;
}


// Needs data file, gaincal file, and alignment file as an arugument
int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]" << std::endl;
    return 1;
  }
  
  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];
  
  TApplication theApp("PLT", &argc, argv);
  
  gSystem->ResetSignal(kSigBus);
  gSystem->ResetSignal(kSigSegmentationViolation);
  gSystem->ResetSignal(kSigIllegalInstruction);
  gSystem->ResetSignal(kSigSystem);
  gSystem->ResetSignal(kSigPipe);
  gSystem->ResetSignal(kSigFloatingException);
  
  
  PLTHitDisplay(DataFileName, GainCalFileName, AlignmentFileName);
  
  theApp.Run();
  
  
  TEveManager::Terminate();
  theApp.Terminate();

  return 0;
}
