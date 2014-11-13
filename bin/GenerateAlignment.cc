////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri Mar 16 12:43:08 CET 2012
// Edited by Grant Riley <Grant.Riley@cern.ch>
// Thrus Nov 13 2014
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>
#include "PLTAlignment.h"
#include "PLTEvent.h"
#include "PLTU.h"
#include "PLTTrack.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"



int GenerateAlignment (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  TCanvas canvas2;

  TH1F histo = TH1F("histo", "x residuals", 200, -0.4, 0.4);
  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_Diamond);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_Diamond);

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }


    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      if (Telescope->NTracks() == 1 && Telescope->NClusters() == 3) {

        // Grab the track
        PLTTrack* Track = Telescope->Track(0);


        float myLResidualX;
        myLResidualX = Track->LResidualX(2);
        float myLResidualY;
        myLResidualY = Track->LResidualY(2);
        float xtrack, ytrack, xhit, yhit,z;
        float xresidual = fabs(xtrack-xhit);
        float yresidual = fabs(ytrack-yhit);
        std::cout<< "Telescope: " << Telescope->Channel() << " myLResidualX  " << myLResidualX << " myLResidualY " << myLResidualY << std::endl;

        histo.Fill(myLResidualX);
        }


      }


    }
  gStyle->SetOptStat(1111);
  canvas2.cd();
  
  histo.Draw(); 
  canvas2.SaveAs("myLresidualX.gif");

  return 0;



}


int main (int argc, char* argv[]){
if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << "  " << "[GainCalFileName.dat]" << "  " << "[AlignmentFileName.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName= argv[2];
  std::string const AlignmentFileName= argv[3];

  GenerateAlignment(DataFileName, GainCalFileName, AlignmentFileName);


return 0;

}
