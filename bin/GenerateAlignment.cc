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
#include <vector>
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
  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_Diamond);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_Diamond);

  TCanvas canvas1;
  PLTAlignment* OldAlignment = Event.GetAlignment();
  gStyle->SetOptStat(1111);
  std::map<int, TH1F*>  h_xResiduals;
  std::map<int, TH1F*>  h_yResiduals;
  std::map<int, TH2F*>  h_xdyResiduals;

  std::vector<int> Channel = OldAlignment->GetListOfChannels(); 
  int ChN = 0;
  for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
    ++ChN;
    for (int iroc=0; iroc<3; ++iroc){
      int id = 10 *(*ich) + iroc;
      if (h_xResiduals.count(id) == 0){
        const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i", *ich, iroc);
        h_xResiduals[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      }
      if (h_yResiduals.count(id) == 0){
        const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i", *ich, iroc);
        h_yResiduals[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      }
      if (h_xdyResiduals.count(id) == 0){
        const char* BUFF =  Form ("XdY_Residual_Ch%02i_ROC%1i", *ich, iroc);
        h_xdyResiduals[id] = new TH2F( BUFF, BUFF, 133, -1, 0.995, 100, -0.5, 0.5);
      }
    }
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
          int channel = Telescope->Channel();
          // Grab the track
          PLTTrack* Track = Telescope->Track(0);
          for (int iroc = 0; iroc <= 2; ++iroc){
            PLTCluster* Cluster = Track->Cluster(iroc);
            float myLResidualX;
            float myLX;
            myLX = Cluster->LX();
            myLResidualX = Track->LResidualX(iroc);
            float myLResidualY;
            float myLY;
            myLY = Cluster->LY();
            myLResidualY = Track->LResidualY(iroc);
            float xslope, yslope;
            xslope = (Track->TX(7.54)-Track->TX(0.0))/3;
            yslope = (Track->TY(7.54)-Track->TY(0.0))/3;
            int PlaneID = 10*channel+iroc;
            h_xResiduals[PlaneID]->Fill(myLResidualX);
            h_yResiduals[PlaneID]->Fill(myLResidualY);
            h_xdyResiduals[PlaneID]->Fill(myLX,myLResidualY);

            //          std::cout<< "Plane: "<<iroc<< " slopeX  " << xslope << " slopeY  " << yslope << " real x " << Track->Cluster(iroc)->TX()<< " real y " << Track->Cluster(iroc)->TY()<< " real z " << Track->Cluster(iroc)->TZ()<< " ResidualY  " << myLResidualY <<" residualX "<< myLResidualX<< std::endl;
          }
        }
      }
    }

    std::map<int,float> x_position;
    std::map<int,float> y_position;
    std::map<int,float> r_position;
    PLTAlignment NewAlignment = *OldAlignment;

    for (std::map<int, TH1F*>::iterator it = h_xResiduals.begin(); it !=h_xResiduals.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      const char* BUFF = Form("./plots/Alignment/X_Residual_Ch%02i_ROC%i.gif",Channel,ROC); 
      canvas1.cd(1);
      h_xResiduals[id]->Draw();
      canvas1.SaveAs(BUFF);
    }
    for (std::map<int, TH1F*>::iterator it = h_yResiduals.begin(); it !=h_yResiduals.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      const char* BUFF = Form("./plots/Alignment/Y_Residual_Ch%02i_ROC%i.gif",Channel,ROC); 
      canvas1.cd(1);
      h_yResiduals[id]->Draw();
      canvas1.SaveAs(BUFF);
    }
    for (std::map<int, TH2F*>::iterator it = h_xdyResiduals.begin(); it !=h_xdyResiduals.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      const char* BUFF = Form("./plots/Alignment/XdY_Residual_Ch%02i_ROC%i.gif",Channel,ROC); 
      canvas1.cd(1);
      h_xdyResiduals[id]->Draw();
      canvas1.SaveAs(BUFF);
    }
    for (std::map<int, TH1F*>::iterator it = h_xResiduals.begin(); it !=h_xResiduals.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (x_position.count(id)==0){x_position[id] = (h_xResiduals[id]->GetMean());}
      if (y_position.count(id)==0){y_position[id] = (h_yResiduals[id]->GetMean());}
      if (r_position.count(id)==0){r_position[id] = (h_xdyResiduals[id]->GetCorrelationFactor());}

      //std::cout << &NewAlignment << " vs " << OldAlignment << std::endl;
      PLTAlignment::CP* ConstMap = NewAlignment.GetCP(Channel,ROC);  
      ConstMap->LX = ConstMap->LX + x_position[id];
      ConstMap->LY = ConstMap->LY + y_position[id];
    }    
    std::string NewAlignmentFileName =  "./NEW_Alignment.dat";
    NewAlignment.WriteAlignmentFile( NewAlignmentFileName );
    


}
return(0);
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
