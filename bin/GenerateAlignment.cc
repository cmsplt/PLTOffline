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
  TCanvas canvas1;

  TH1F histo = TH1F("histo", "x residuals ROC 0", 200, -0.4, 0.4);
  TH1F histo1 = TH1F("histo1", "x residuals ROC 1", 200, -0.4, 0.4);
  TH1F histo2 = TH1F("histo2", "x residuals ROC 2", 200, -0.4, 0.4);
  TH1F histoy = TH1F("histoy", "y residuals ROC 0", 200, -0.4, 0.4);
  TH1F histoy1 = TH1F("histoy1", "y residuals ROC 1", 200, -0.4, 0.4);
  TH1F histoy2 = TH1F("histoy2", "y residuals ROC 2", 200, -0.4, 0.4);


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
        for (int iroc = 0; iroc <= 2; ++iroc){

          float myLResidualX;
          myLResidualX = Track->LResidualX(iroc);
          float myLResidualY;
          myLResidualY = Track->LResidualY(iroc);
          float xslope, yslope;
          xslope = (Track->TX(7.54)-Track->TX(0.0))/3;
          yslope = (Track->TY(7.54)-Track->TY(0.0))/3; 


          //          std::cout<< "Plane: "<<iroc<< " slopeX  " << xslope << " slopeY  " << yslope << " real x " << Track->Cluster(iroc)->TX()<< " real y " << Track->Cluster(iroc)->TY()<< " real z " << Track->Cluster(iroc)->TZ()<< " ResidualY  " << myLResidualY <<" residualX "<< myLResidualX<< std::endl;
          if (iroc==0){
            histoy.Fill(myLResidualY);
            histo.Fill(myLResidualX);
          }
          if (iroc==1){
            histoy1.Fill(myLResidualY);
            histo1.Fill(myLResidualX);
          }
          if (iroc==2){
            histoy2.Fill(myLResidualY);
            histo2.Fill(myLResidualX);
          }
        }
      }
    }
  }
  canvas1.Divide(1,2);

  gStyle->SetOptStat(1111);
  canvas1.cd(1);
  histo.Draw(); 
  canvas1.cd(2);
  histoy.Draw();
  canvas1.SaveAs("Residual_ROC0.gif");

  canvas1.cd(1);
  histo1.Draw(); 
  canvas1.cd(2);
  histoy1.Draw();
  canvas1.SaveAs("Residual_ROC1.gif");

  canvas1.cd(1);
  histo2.Draw(); 
  canvas1.cd(2);
  histoy2.Draw();
  canvas1.SaveAs("Residual_ROC2.gif");
  

  std::vector<float> x_position;
  std::vector<float> y_position;
  std::vector<float> r_position;

  for (int iroc = 0; iroc<=2; ++iroc){
    x_position.push_back(0);
    y_position.push_back(0);
    r_position.push_back(0);
    
    PLTAlignment();


  }    
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
