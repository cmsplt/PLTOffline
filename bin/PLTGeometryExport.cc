////////////////////////////////////////////////////////////////////
//
// Krishna Thapa, Grant Riley
//
// Created on: Tue Mar 7, 2015
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


  TGeoVolume *Top = GeoManager->MakeBox("TOP", Vacuum, 27., 27., 120.);
  GeoManager->SetTopVolume(Top);
  TGeoVolume *Replica = GeoManager->MakeBox("REPLICA", Vacuum,400,400,400);
  Replica->SetVisibility(kFALSE);
  TGeoVolume *CP= GeoManager->MakeBox("CP", Vacuum, 110., 50., 5.);
  CP->SetVisibility(kFALSE);


  std::vector< std::pair<int, int> > ChannelROCs = Alignment.GetListOfChannelROCs();
  for (std::vector< std::pair<int, int> >::iterator It = ChannelROCs.begin(); It != ChannelROCs.end(); ++It) {
    PLTAlignment::CP* C = Alignment.GetCP(*It);

    // Aperture is defined by 2nd rock.
    double aperture = It->second == 1 ? 0. : 0.2;
    
    // Make a plane
    TGeoVolume *plane = GeoManager->MakeBox("plane", Al, 0.4+aperture, 0.4+aperture, 0.005);
    plane->SetLineColor(kBlue);

    // Translation and rotation of this plane in global space
    TGeoRotation    *Rotation = new TGeoRotation(TString::Format("RotationZ%i", 10*It->first + It->second), C->GRZ * 180. / TMath::Pi(), 0, 0.);
    TGeoCombiTrans  *TransRot;// = new TGeoCombiTrans(C->GX, C->GY, C->GZ + C->LZ, Rotation);
    if (C->GRY < 1.0) {
      TransRot = new TGeoCombiTrans(C->GX, C->GY, (C->GZ + C->LZ), Rotation);
    } else {
      //      TransRot = new TGeoCombiTrans(-C->GX, -C->GY, (-C->GZ - C->LZ), Rotation);
      //      exit(0);
    }

    CP->AddNode(plane,  10*It->first + It->second, TransRot);
    printf("Rotations: %15.3E  %15.3E\n", C->GRZ, C->GRY);
  }


  Replica->AddNode(CP, 1);
  Top->AddNode(Replica, 1);


  //--- close the geometry
  GeoManager->CloseGeometry();

  //--- draw the ROOT box.
  GeoManager->SetVisLevel(4);
  GeoManager->Export("Alignment.root");
  GeoManager->Export("Alignment.xml");

   return;
}




int PLTEventDisplay (std::string const AlignmentFileName)
{

  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Geometry manager..
  TGeoManager* GeoManager = new TGeoManager("PLT", "PLT Geometry");

  /* TODO
  std::string* filenmeRoot, filenmeXml;
  filenmeRoot="./ALIGNMENT/GeometryExport/" + AlignmentFileName + ".root";
  filenmeXml="./ALIGNMENT/GeometryExport/" + AlignmentFileName + ".xml";
  */

  SetupGeometry(GeoManager, Alignment);
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [AlignmentFile.dat]" << std::endl;
    return 1;
  }

  std::string const AlignmentFileName = argv[1];

  TApplication theApp("PLT", &argc, argv);

  gSystem->ResetSignal(kSigBus);
  gSystem->ResetSignal(kSigSegmentationViolation);
  gSystem->ResetSignal(kSigIllegalInstruction);
  gSystem->ResetSignal(kSigSystem);
  gSystem->ResetSignal(kSigPipe);
  gSystem->ResetSignal(kSigFloatingException);


  PLTEventDisplay(AlignmentFileName);

  return 0;
}
