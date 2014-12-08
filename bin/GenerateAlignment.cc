////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
// Created on: Fri Mar 16 12:43:08 CET 2012
//
// Working version Created by Grant Riley <Grant.Riley@cern.ch>
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
#include "TGraph.h"


int GenerateAlignment (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName:    " << GainCalFileName << std::endl;
  //system( "head /data/dhidas/PLTMC/GainCalFits_Test.dat");
  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

  TCanvas canvas1;
  PLTAlignment* OldAlignment = Event.GetAlignment();
  gStyle->SetOptStat(1111);
  std::map<int, TH1F*>  h_xResiduals;
  std::map<int, TH1F*>  h_yResiduals;
  int TracksN = 0;
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
        ++TracksN;
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
          //          std::cout<< "Plane: "<<iroc<< " slopeX  " << xslope << " slopeY  " << yslope << " real x " << Track->Cluster(iroc)->TX()<< " real y " << Track->Cluster(iroc)->TY()<< " real z " << Track->Cluster(iroc)->TZ()<< " ResidualY  " << myLResidualY <<" residualX "<< myLResidualX<< std::endl;
        }
      }
    }
  }
  std::cout << "Number of Tracks accepted by PLTTracking: "<< TracksN << std::endl;
  std::map<int,float> x_position;
  std::map<int,float> y_position;
  PLTAlignment NewAlignment = *OldAlignment;

  for (std::map<int, TH1F*>::iterator it = h_xResiduals.begin(); it !=h_xResiduals.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    if (it->second->GetEntries() != 0){

      const char* BUFF = Form("./plots/Alignment/X_Residual_Ch%02i_ROC%i.gif",Channel,ROC); 
      canvas1.cd(1);
      h_xResiduals[id]->Draw();
      canvas1.SaveAs(BUFF);
    }
    else continue;
  }
  for (std::map<int, TH1F*>::iterator it = h_yResiduals.begin(); it !=h_yResiduals.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    if (it->second->GetEntries() != 0){
      const char* BUFF = Form("./plots/Alignment/Y_Residual_Ch%02i_ROC%i.gif",Channel,ROC); 
      canvas1.cd(1);
      h_yResiduals[id]->Draw();
      canvas1.SaveAs(BUFF);
    }
    else continue;
  }
  for (std::map<int, TH1F*>::iterator it = h_xResiduals.begin(); it !=h_xResiduals.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    if (x_position.count(id)==0){x_position[id] = (h_xResiduals[id]->GetMean());}
    if (y_position.count(id)==0){y_position[id] = (h_yResiduals[id]->GetMean());}
    //std::cout << &NewAlignment << " vs " << OldAlignment << std::endl;
    PLTAlignment::CP* ConstMap = NewAlignment.GetCP(Channel,ROC);  
    ConstMap->LX = ConstMap->LX + x_position[id];
    ConstMap->LY = ConstMap->LY + y_position[id];
    std::cout << "const map updated" << std::endl;
  }    
  std::cout << "exit loop" << std::endl;
  std::string NewAlignmentFileName =  "./NEW_Alignment.dat";
  NewAlignment.WriteAlignmentFile( NewAlignmentFileName );
  std::cout << "dafuq" << std::endl;
//  //Rotational fixes applied after translational fixes, tracks generated with new alignment
//  std::cout << " before second read" <<std::endl;
//  PLTEvent Event2(DataFileName, GainCalFileName, NewAlignmentFileName);
//  Event2.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
//  Event2.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
//  std::cout << "after second read " << std::endl;
//  PLTAlignment* NEWAlignment = Event2.GetAlignment();
//  gStyle->SetOptStat(1111);
//  std::map<int, TH1F*>  h_xResiduals2;
//  std::map<int, TH1F*>  h_yResiduals2;
//  std::map<int, TGraph*> g_xdyResiduals2;
//  TracksN = 0;
//  Channel = NEWAlignment->GetListOfChannels(); 
//  ChN = 0;
//  for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
//    ++ChN;
//    for (int iroc=0; iroc<3; ++iroc){
//      int id = 10 *(*ich) + iroc;
//      if (h_xResiduals2.count(id) == 0){
//        const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i_Second", *ich, iroc);
//        h_xResiduals2[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
//      }
//      if (h_yResiduals2.count(id) == 0){
//        const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i_Second", *ich, iroc);
//        h_yResiduals2[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
//      }
//      if (g_xdyResiduals2.count(id) == 0){
//        const char* BUFF =  Form ("XdY_ResidualGraph_Ch%02i_ROC%1i_Second", *ich, iroc);
//        g_xdyResiduals2[id] = new TGraph( BUFF );
//      }
//    }
//  } 
//  // Loop over all events in file
//  for (int ientry = 0; Event2.GetNextEvent() >= 0; ++ientry) {
//    if (ientry % 10000 == 0) {
//      std::cout << "Processing entry: " << ientry << std::endl;
//    }
//
//
//    // Loop over all planes with hits in event
//    for (size_t it = 0; it != Event2.NTelescopes(); ++it) {
//
//      // THIS telescope is
//      PLTTelescope* Telescope = Event2.Telescope(it);
//      if (Telescope->NTracks() == 1 && Telescope->NClusters() == 3) {
//        int channel = Telescope->Channel();
//        // Grab the track
//        ++TracksN;
//        PLTTrack* Track = Telescope->Track(0);
//        for (int iroc = 0; iroc <= 2; ++iroc){
//          PLTCluster* Cluster = Track->Cluster(iroc);
//          float myLResidualX;
//          float myLX;
//          myLX = Cluster->LX();
//          myLResidualX = Track->LResidualX(iroc);
//          float myLResidualY;
//          float myLY;
//          myLY = Cluster->LY();
//          myLResidualY = Track->LResidualY(iroc);
//          float xslope, yslope;
//          xslope = (Track->TX(7.54)-Track->TX(0.0))/3;
//          yslope = (Track->TY(7.54)-Track->TY(0.0))/3;
//          int PlaneID = 10*channel+iroc;
//          h_xResiduals2[PlaneID]->Fill(myLResidualX);
//          h_yResiduals2[PlaneID]->Fill(myLResidualY);
//          g_xdyResiduals2[PlaneID]->SetPoint(g_xdyResiduals2[PlaneID]->GetN(), myLX, myLResidualY );
//          //          std::cout<< "Plane: "<<iroc<< " slopeX  " << xslope << " slopeY  " << yslope << " real x " << Track->Cluster(iroc)->TX()<< " real y " << Track->Cluster(iroc)->TY()<< " real z " << Track->Cluster(iroc)->TZ()<< " ResidualY  " << myLResidualY <<" residualX "<< myLResidualX<< std::endl;
//        }
//      }
//    }
//  }
//  std::cout << "Number of Tracks accepted by PLTTracking: "<< TracksN << std::endl;
//  x_position.clear();
//  y_position.clear();
//  std::map<int,float> r_position;
//  PLTAlignment RotAlignment = *NEWAlignment;
//
//  for (std::map<int, TH1F*>::iterator it = h_xResiduals2.begin(); it !=h_xResiduals2.end(); ++it){
//    int const Channel = it->first / 10;
//    int const ROC     = it->first % 10;
//    int const id      = it->first;
//    if (it->second->GetEntries() != 0){
//
//      const char* BUFF = Form("./plots/Alignment/X_Residual_Ch%02i_ROC%i_Second.gif",Channel,ROC); 
//      canvas1.cd(1);
//      h_xResiduals2[id]->Draw();
//      canvas1.SaveAs(BUFF);
//    }
//    else continue;
//  }
//  for (std::map<int, TH1F*>::iterator it = h_yResiduals2.begin(); it !=h_yResiduals2.end(); ++it){
//    int const Channel = it->first / 10;
//    int const ROC     = it->first % 10;
//    int const id      = it->first;
//    if (it->second->GetEntries() != 0){
//      const char* BUFF = Form("./plots/Alignment/Y_Residual_Ch%02i_ROC%i_Second.gif",Channel,ROC); 
//      canvas1.cd(1);
//      h_yResiduals2[id]->Draw();
//      canvas1.SaveAs(BUFF);
//    }
//    else continue;
//  }
//  for (std::map<int, TGraph*>::iterator it = g_xdyResiduals2.begin(); it !=g_xdyResiduals2.end(); ++it){
//    int const Channel = it->first / 10;
//    int const ROC     = it->first % 10;
//    int const id      = it->first;
//    // if (x_position.count(id)==0){x_position[id] = (h_xResiduals[id]->GetMean());}
//    // if (y_position.count(id)==0){y_position[id] = (h_yResiduals[id]->GetMean());}
//    if (it->second->GetN() != 0){
//      TF1 linear_fun = TF1("","[0]+[1]*x");
//      g_xdyResiduals2[id]->Fit(&linear_fun);
//      PLTAlignment::CP* ConstMap = RotAlignment.GetCP(Channel,ROC);  
//      float other_angle = atan(linear_fun.GetParameter(1));
//      const char* BUFF = Form("./plots/Alignment/XdY_ResidualGraph_Ch%02i_ROC%i_Second.gif",Channel,ROC); 
//      canvas1.cd(1);
//      g_xdyResiduals2[id]->Draw();
//      canvas1.SaveAs(BUFF);
//      ConstMap->LR = ConstMap->LR + other_angle/3.;
//      std::cout << "ROC: " << id << " Other Angle:" << other_angle << std::endl;
//      std::cout << "Channel: " << Channel << " ROC: " << ROC << " rotation: " << other_angle/3 << std::endl;
//    }
//    else continue;
//  }    
//  std::string RotAlignmentFileName =  "./ROT_Alignment.dat";
//  RotAlignment.WriteAlignmentFile( RotAlignmentFileName );
//  //Third time, just for final plots of residuals. no correction here
//
//  PLTEvent Event3(DataFileName, GainCalFileName, RotAlignmentFileName);
//  Event3.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
//  Event3.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
//
//  PLTAlignment* ROTAlignment = Event3.GetAlignment();
//  gStyle->SetOptStat(1111);
//  std::map<int, TH1F*>  h_xResiduals3;
//  std::map<int, TH1F*>  h_yResiduals3;
//  std::map<int, TH2F*>  h_xdyResiduals3;
//  std::map<int, TGraph*> g_xdyResiduals3;
//  TracksN = 0;
//  Channel = ROTAlignment->GetListOfChannels(); 
//  ChN = 0;
//  for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
//    ++ChN;
//    for (int iroc=0; iroc<3; ++iroc){
//      int id = 10 *(*ich) + iroc;
//      if (h_xResiduals3.count(id) == 0){
//        const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
//        h_xResiduals3[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
//      }
//      if (h_yResiduals3.count(id) == 0){
//        const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
//        h_yResiduals3[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
//      }
//      if (h_xdyResiduals3.count(id) == 0){
//        const char* BUFF =  Form ("XdY_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
//        h_xdyResiduals3[id] = new TH2F( BUFF, BUFF, 133, -1, 0.995, 100, -0.5, 0.5);
//      }
//      if (g_xdyResiduals3.count(id) == 0){
//        const char* BUFF =  Form ("XdY_ResidualGraph_Ch%02i_ROC%1i_Third", *ich, iroc);
//        g_xdyResiduals3[id] = new TGraph( BUFF );
//      }
//    }
//  } 
//  // Loop over all events in file
//  for (int ientry = 0; Event3.GetNextEvent() >= 0; ++ientry) {
//    if (ientry % 10000 == 0) {
//      std::cout << "Processing entry: " << ientry << std::endl;
//    }
//
//
//    // Loop over all planes with hits in event
//    for (size_t it = 0; it != Event3.NTelescopes(); ++it) {
//
//      // THIS telescope is
//      PLTTelescope* Telescope = Event3.Telescope(it);
//      if (Telescope->NTracks() == 1 && Telescope->NClusters() == 3) {
//        int channel = Telescope->Channel();
//        // Grab the track
//        ++TracksN;
//        PLTTrack* Track = Telescope->Track(0);
//        for (int iroc = 0; iroc <= 2; ++iroc){
//          PLTCluster* Cluster = Track->Cluster(iroc);
//          float myLResidualX;
//          float myLX;
//          myLX = Cluster->LX();
//          myLResidualX = Track->LResidualX(iroc);
//          float myLResidualY;
//          float myLY;
//          myLY = Cluster->LY();
//          myLResidualY = Track->LResidualY(iroc);
//          float xslope, yslope;
//          xslope = (Track->TX(7.54)-Track->TX(0.0))/3;
//          yslope = (Track->TY(7.54)-Track->TY(0.0))/3;
//          int PlaneID = 10*channel+iroc;
//          h_xResiduals3[PlaneID]->Fill(myLResidualX);
//          h_yResiduals3[PlaneID]->Fill(myLResidualY);
//          h_xdyResiduals3[PlaneID]->Fill(myLX,myLResidualY);
//          g_xdyResiduals3[PlaneID]->SetPoint(g_xdyResiduals3[PlaneID]->GetN(), myLX, myLResidualY );
//          //          std::cout<< "Plane: "<<iroc<< " slopeX  " << xslope << " slopeY  " << yslope << " real x " << Track->Cluster(iroc)->TX()<< " real y " << Track->Cluster(iroc)->TY()<< " real z " << Track->Cluster(iroc)->TZ()<< " ResidualY  " << myLResidualY <<" residualX "<< myLResidualX<< std::endl;
//        }
//      }
//    }
//  }
//
//  std::cout << "Number of Tracks accepted by PLTTracking: "<< TracksN << std::endl;
//  x_position.clear();
//  y_position.clear();
//  r_position.clear();
//  PLTAlignment FinalAlignment = *ROTAlignment;
//
//  for (std::map<int, TH1F*>::iterator it = h_xResiduals3.begin(); it !=h_xResiduals3.end(); ++it){
//    int const Channel = it->first / 10;
//    int const ROC     = it->first % 10;
//    int const id      = it->first;
//    if (it->second->GetEntries() != 0){
//
//      const char* BUFF = Form("./plots/Alignment/X_Residual_Ch%02i_ROC%i_Third.gif",Channel,ROC); 
//      canvas1.cd(1);
//      h_xResiduals3[id]->Draw();
//      canvas1.SaveAs(BUFF);
//    }
//    else continue;
//  }
//  for (std::map<int, TH1F*>::iterator it = h_yResiduals3.begin(); it !=h_yResiduals3.end(); ++it){
//    int const Channel = it->first / 10;
//    int const ROC     = it->first % 10;
//    int const id      = it->first;
//    if (it->second->GetEntries() != 0){
//      const char* BUFF = Form("./plots/Alignment/Y_Residual_Ch%02i_ROC%i_Third.gif",Channel,ROC); 
//      canvas1.cd(1);
//      h_yResiduals3[id]->Draw();
//      canvas1.SaveAs(BUFF);
//    }
//    else continue;
//  }
//  for (std::map<int, TH2F*>::iterator it = h_xdyResiduals3.begin(); it !=h_xdyResiduals3.end(); ++it){
//    int const Channel = it->first / 10;
//    int const ROC     = it->first % 10;
//    int const id      = it->first;
//    if (it->second->GetEntries() != 0){
//      const char* BUFF = Form("./plots/Alignment/XdY_Residual_Ch%02i_ROC%i_Third.gif",Channel,ROC); 
//      canvas1.cd(1);
//      h_xdyResiduals3[id]->Draw();
//      canvas1.SaveAs(BUFF);
//    }
//    else continue;
//  }
//  for (std::map<int, TGraph*>::iterator it = g_xdyResiduals3.begin(); it !=g_xdyResiduals3.end(); ++it){
//    int const Channel = it->first / 10;
//    int const ROC     = it->first % 10;
//    int const id      = it->first;
//    // if (x_position.count(id)==0){x_position[id] = (h_xResiduals[id]->GetMean());}
//    // if (y_position.count(id)==0){y_position[id] = (h_yResiduals[id]->GetMean());}
//    if (it->second->GetN() != 0){
//      TF1 linear_fun = TF1("","[0]+[1]*x");
//      g_xdyResiduals3[id]->Fit(&linear_fun);
//      float other_angle = atan(linear_fun.GetParameter(1));
//      std::cout << "Channel:Roc: " << Channel << " : " << ROC  << " angle: " << other_angle << std::endl;
//      const char* BUFF = Form("./plots/Alignment/XdY_ResidualGraph_Ch%02i_ROC%i_Third.gif",Channel,ROC); 
//      canvas1.cd(1);
//      g_xdyResiduals3[id]->Draw();
//      canvas1.SaveAs(BUFF);
//    }
//    else continue;
//  }    
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
