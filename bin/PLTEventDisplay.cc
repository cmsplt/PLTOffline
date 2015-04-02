////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Sat Mar 17 12:33:14 CET 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TEveTrack.h"
#include "TEveTrack.h"
#include "TEvePointSet.h"
#include "TEveStraightLineSet.h"

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




void SetupGeometry (TGeoManager* GeoManager, PLTAlignment& Alignment)
{
  // Define some material and media
  TGeoMaterial *MatVacuum = new TGeoMaterial("Vacuum", 0,0,0);
  TGeoMaterial *MatAl = new TGeoMaterial("Al", 26.98,13,2.7);
  TGeoMedium *Vacuum = new TGeoMedium("Vacuum",1, MatVacuum);
  TGeoMedium *Al = new TGeoMedium("Root Material",2, MatAl);


  // Beampipe
//  TGeoVolume* BeamPipe = GeoManager->MakeTube("BeamPipe", Al, 0.8, 1.0, 10);

  TGeoVolume *Top = GeoManager->MakeBox("TOP", Vacuum, 27., 27., 120.);
  GeoManager->SetTopVolume(Top);
  TGeoVolume *Replica = GeoManager->MakeBox("REPLICA", Vacuum,400,400,400);
  Replica->SetVisibility(kFALSE);
  TGeoVolume *CP= GeoManager->MakeBox("CP", Vacuum, 110., 50., 5.);
  CP->SetVisibility(kFALSE);


  std::vector< std::pair<int, int> > ChannelROCs = Alignment.GetListOfChannelROCs();
  for (std::vector< std::pair<int, int> >::iterator It = ChannelROCs.begin(); It != ChannelROCs.end(); ++It) {
    PLTAlignment::CP* C = Alignment.GetCP(*It);


    // Make a plane
    TGeoVolume *plane = GeoManager->MakeBox("plane", Al, 0.4, 0.4, 0.03);
    plane->SetLineColor(kBlue);
		
    // Translation and rotation of this plane in global space
    TGeoRotation    *Rotation = new TGeoRotation(TString::Format("RotationZ%i", 10*It->first + It->second), C->GRZ * 180. / TMath::Pi(), 0, 0.);
    TGeoCombiTrans  *TransRot = new TGeoCombiTrans(C->GX, C->GY, C->GZ + C->LZ, Rotation);
    if (C->GRY < 3.0) {
      TransRot = new TGeoCombiTrans(C->GX, C->GY, (C->GZ + C->LZ), Rotation);
    } 
    else {
      TransRot = new TGeoCombiTrans(C->GX, C->GY, (C->GZ - C->LZ), Rotation);
    }

    CP->AddNode(plane,  10*It->first + It->second, TransRot);
    printf("Rotations: %15.3E  %15.3E\n", C->GRZ, C->GRY);
  }


  Replica->AddNode(CP, 1);
  Top->AddNode(Replica, 1);
//  Top->AddNode(BeamPipe, 1);


  //--- close the geometry
  GeoManager->CloseGeometry();

  //--- draw the ROOT box.
  GeoManager->SetVisLevel(4);

  TEveManager::Create();
  TGeoNode* node = GeoManager->GetTopNode();
  TEveGeoTopNode* en = new TEveGeoTopNode(GeoManager, node);
  en->SetVisLevel(4);
  en->GetNode()->GetVolume()->SetVisibility(kFALSE);

  //gEve->AddElement(list);
  //list->AddElement(track);
  gEve->AddGlobalElement(en);

  gEve->Redraw3D(kTRUE);
  gSystem->ProcessEvents();


  en->ExpandIntoListTreesRecursively();


  return;
}




int PLTEventDisplay (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_Diamond;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_m1_m1;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Geometry manager..
  TGeoManager* GeoManager = new TGeoManager("PLT", "PLT Geometry");

  SetupGeometry(GeoManager, Alignment);


  TEveTrackList *list = new TEveTrackList();
  TEveTrackPropagator* prop = list->GetPropagator();
  prop->SetFitDaughters(kFALSE);
  prop->SetMaxZ(185);

  TEveRecTrackD *rc = new TEveRecTrackD();


  std::map<int, int> NTrackMap;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }


    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      if (NTrackMap[Telescope->Channel()] > 10) continue;


      printf("Number of tracks: %i\n", Telescope->NTracks());
      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        PLTTrack* T = Telescope->Track(itrack);

        std::pair<float, float> TrXY = T->GXYatGZ(0, Alignment);
        rc->fV.Set(TrXY.first, TrXY.second, 0);
        rc->fP.Set(T->fGVX, T->fGVY, T->fGVZ);
        rc->fSign = 0;

        printf("Track: O V - %15.3E %15.3E %15.3E -  %15.3E %15.3E %15.3E\n", T->fGOX, T->fGOY, T->fGOZ, T->fGVX, T->fGVY, T->fGVZ);

        //list->Delete();
        TEveTrack* track = new TEveTrack(rc, prop);
        track->SetLineColor(0);
        track->MakeTrack();
        //gEve->AddElement(list);
        //list->AddElement(track);
        gEve->AddElement(track);
        ++NTrackMap[Telescope->Channel()];
      }

      gEve->Redraw3D(kTRUE);
      gSystem->ProcessEvents();


    }


  }

  return 0;
}


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


  PLTEventDisplay(DataFileName, GainCalFileName, AlignmentFileName);

  theApp.Run();


  TEveManager::Terminate();
  theApp.Terminate();

  return 0;
}
