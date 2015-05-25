////////////////////////////////////////////////////////////////////
//
//
// Working version Created by Grant Riley <Grant.Riley@cern.ch>
// Thrus Nov 13 2014
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <utility>
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
  TCanvas canvas2("Canvas2","Canvas2",800,600);
  TCanvas canvas3;
  canvas3.Divide(2,1);
  PLTAlignment* InitialAlignment = Event.GetAlignment();
  gStyle->SetOptStat(1111);
  std::map<int, TH1F*>  h_xResiduals;
  std::map<int, TH1F*>  h_yResiduals;
  std::map<int, std::pair<TH1F*, TH1F*> > h_Slopes; 
  int TracksN = 0;
  std::vector<int> Channel = InitialAlignment->GetListOfChannels(); 
  int ChN = 1;
  for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
    ++ChN;
    if (h_Slopes.count(*ich) == 0){
      const char* BUFF =  Form ("Y_Slope_Ch%02i", *ich);
      h_Slopes[*ich].second = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      const char* BUFF2 =  Form ("X_Slope_Ch%02i", *ich);
      h_Slopes[*ich].first= new TH1F( BUFF2, BUFF2, 100, -0.2, 0.2);

    }
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
    if (ientry % 100 == 0) {
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
        float xslope, yslope;
        xslope = (Track->TX(7.54)-Track->TX(0.0))/3;
        yslope = (Track->TY(7.54)-Track->TY(0.0))/3;
        h_Slopes[channel].first->Fill(xslope);
        h_Slopes[channel].second->Fill(yslope);
        for (int iroc = 0; iroc <= 2; ++iroc){
          PLTCluster* Cluster = Track->Cluster(iroc);
          float myLResidualX;
          float myLX;
          myLX = Cluster->LX();
          myLResidualX = myLX - Track->Cluster(0)->LX();
          float myLResidualY;
          float myLY;
          myLY = Cluster->LY();
          myLResidualY = myLY - Track->Cluster(0)->LY();
          int PlaneID = 10*channel+iroc;
          h_xResiduals[PlaneID]->Fill(myLResidualX);
          h_yResiduals[PlaneID]->Fill(myLResidualY);
        }
      }
    }
  }    
  //              std::cout<< "Plane: "<<iroc<< " slopeX  " << xslope << " slopeY  " << yslope << " real x " << Track->Cluster(iroc)->TX()<< " real y " << Track->Cluster(iroc)->TY()<< " real z " << Track->Cluster(iroc)->TZ()<< " ResidualY  " << myLResidualY <<" residualX "<< myLResidualX<< std::endl;
  for (std::map<int, TH1F*>::iterator it = h_xResiduals.begin(); it !=h_xResiduals.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    if (it->second->GetEntries() != 0){

      const char* BUFF = Form("./plots/Alignment/X_Residual_Ch%02i_ROC%i_.gif",Channel,ROC); 
      canvas1.cd();
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
      const char* BUFF = Form("./plots/Alignment/Y_Residual_Ch%02i_ROC%i_.gif",Channel,ROC); 
      canvas1.cd();
      h_yResiduals[id]->Draw();
      canvas1.SaveAs(BUFF);
    }
    else continue;
  }
  for (std::map<int, std::pair<TH1F*,TH1F*> >::iterator it = h_Slopes.begin(); it !=h_Slopes.end(); ++it){
    int const Channel = it->first;
    if (it->second.second->GetEntries() != 0){
      const char* BUFF = Form("./plots/Alignment/X_Y_Slopes_Ch%02i_.gif",Channel); 
      canvas3.cd(1);
      h_Slopes[Channel].first->Draw();
      canvas3.cd(2);
      h_Slopes[Channel].second->Draw();
      canvas3.SaveAs(BUFF);
    }
    else continue;
  }
  std::cout << "Number of Tracks accepted by PLTTracking: "<< TracksN << std::endl;
  //****************************************************************************************
  //First Corrections --rotational
  //input "InitialAlignment" output "RotatedAlignment"

  PLTAlignment TempAlignment = *InitialAlignment;
  gStyle->SetOptStat(1111);
  std::map<int, TH1F*>  h_xResiduals1;
  std::map<int, TH1F*>  h_yResiduals1;
  std::map<int, TH2F*>  h_xdyResiduals1;
  std::map<int, TH2F*>  h_ydxResiduals1;
  std::map<int, std::pair<TH1F*, TH1F*> > h_Slopes1; 

  TracksN = 0;
  Channel.clear();
  Channel = TempAlignment.GetListOfChannels(); 
  //Channel = OldAlignment->GetListOfChannels(); 

  ChN = 1;
  for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
    ++ChN;
    if (h_Slopes1.count(*ich) == 0){
      const char* BUFF =  Form ("Y_Slope_Ch%02i_First", *ich);
      h_Slopes1[*ich].second = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      const char* BUFF2 =  Form ("X_Slope_Ch%02i_First", *ich);
      h_Slopes1[*ich].first = new TH1F( BUFF2, BUFF2, 100, -0.2, 0.2);
    }
    for (int iroc=0; iroc<3; ++iroc){
      int id = 10 *(*ich) + iroc;
      if (h_xResiduals1.count(id) == 0){
        const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i_First", *ich, iroc);
        h_xResiduals1[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      }
      if (h_yResiduals1.count(id) == 0){
        const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i_First", *ich, iroc);
        h_yResiduals1[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      }
      if (h_xdyResiduals1.count(id) == 0){
        const char* BUFF =  Form ("XdY_Residual_Ch%02i_ROC%1i_First", *ich, iroc);
        h_xdyResiduals1[id] = new TH2F( BUFF, BUFF, 133, -1, 0.995, 100, -0.5, 0.5);
      }
      if (h_ydxResiduals1.count(id) == 0){
        const char* BUFF =  Form ("YdX_Residual_Ch%02i_ROC%1i_First", *ich, iroc);
        h_ydxResiduals1[id] = new TH2F( BUFF, BUFF, 133, -1, 0.995, 100, -0.5, 0.5);
      }
    }
  } 
  // Loop over all events in file
  std::map<int,float> x_position;
  std::map<int,float> y_position;
  std::map<int,float> r_position;
  std::map<int,float> angle;
  PLTAlignment RotatedAlignment = TempAlignment;
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 100 == 0) {
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
        float xslope, yslope;
        xslope = (Track->TX(7.54)-Track->TX(0.0))/3;
        yslope = (Track->TY(7.54)-Track->TY(0.0))/3;
        h_Slopes1[channel].first->Fill(xslope);
        h_Slopes1[channel].second->Fill(yslope);
        for (int iroc = 0; iroc <= 2; ++iroc){
          PLTCluster* Cluster = Track->Cluster(iroc);
          float myLResidualX;
          float myLX;
          myLX = Cluster->LX();
          myLResidualX = myLX - Track->Cluster(0)->LX();
          float myLResidualY;
          float myLY;
          myLY = Cluster->LY();
          myLResidualY = myLY - Track->Cluster(0)->LY();
          int PlaneID = 10*channel+iroc;
          h_xResiduals1[PlaneID]->Fill(myLResidualX);
          h_yResiduals1[PlaneID]->Fill(myLResidualY);
          h_xdyResiduals1[PlaneID]->Fill(myLX,myLResidualY);
          h_ydxResiduals1[PlaneID]->Fill(myLY,myLResidualX);
        }
      }
    }
    for (std::map<int, TH1F*>::iterator it = h_xResiduals1.begin(); it !=h_xResiduals1.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){

        const char* BUFF = Form("./plots/Alignment/X_Residual_Ch%02i_ROC%i_First.gif",Channel,ROC); 
        canvas1.cd();
        h_xResiduals1[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, TH1F*>::iterator it = h_yResiduals1.begin(); it !=h_yResiduals1.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/Y_Residual_Ch%02i_ROC%i_First.gif",Channel,ROC); 
        canvas1.cd();
        h_yResiduals1[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, TH2F*>::iterator it = h_xdyResiduals1.begin(); it !=h_xdyResiduals1.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/XdY_Residual_Ch%02i_ROC%i_First.gif",Channel,ROC); 
        canvas1.cd(1);
        h_xdyResiduals1[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, TH2F*>::iterator it = h_ydxResiduals1.begin(); it !=h_ydxResiduals1.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/YdX_Residual_Ch%02i_ROC%i_First.gif",Channel,ROC); 
        canvas1.cd(1);
        h_ydxResiduals1[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, std::pair<TH1F*,TH1F*> >::iterator it = h_Slopes1.begin(); it !=h_Slopes1.end(); ++it){
      int const Channel = it->first;
      if (it->second.second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/X_Y_Slopes_Ch%02i_First.gif",Channel); 
        canvas3.cd(1);
        h_Slopes1[Channel].first->Draw();
        canvas3.cd(2);
        h_Slopes1[Channel].second->Draw();
        canvas3.SaveAs(BUFF);
      }
      else continue;
    }
    //do rotational correction
    for (std::map<int, TH2F*>::iterator it = h_xdyResiduals1.begin(); it != h_xdyResiduals1.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (r_position.count(id)==0) {r_position[id] = (h_xdyResiduals1[id]->GetCorrelationFactor()); angle[id] = atan(r_position[id]); }
      std::cout << "ChannelRoc: " << Channel<< ":" << ROC<< " Correlation Factor: " << r_position[id] << " angle: " << angle[id]<< std::endl;
      //std::cout << &NewAlignment << " vs " << OldAlignment << std::endl;
      PLTAlignment::CP* ConstMap2 = RotatedAlignment.GetCP(Channel,ROC);  
      ConstMap2->LR = ConstMap2->LR + angle[id]/3;

    }    
    std::string RotAlignmentFileName =  "./ROTATED_Alignment.dat";
    RotatedAlignment.WriteAlignmentFile( RotAlignmentFileName );

    for (std::map<int, TH1F*>::iterator it = h_xResiduals1.begin(); it !=h_xResiduals1.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, TH1F*>::iterator it = h_yResiduals1.begin(); it !=h_yResiduals1.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, TH2F*>::iterator it = h_xdyResiduals1.begin(); it !=h_xdyResiduals1.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, TH2F*>::iterator it = h_ydxResiduals1.begin(); it !=h_ydxResiduals1.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, std::pair<TH1F*, TH1F*> >::iterator it = h_Slopes1.begin(); it != h_Slopes1.end(); ++it){
      delete it->second.first; 
      delete it->second.second; 
    }
    //********************************************************************************************************************
    //Second Step --translation
    //input "RotatedAlignment" output "TransAlignment"
    std::cout<<"S E C O N D   T I E M M M M M M  M M M  M  M M M M  M  M M "<<std::endl;
    PLTEvent Event2(DataFileName, GainCalFileName, RotAlignmentFileName);
    Event2.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
    Event2.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
    std::cout << "DataFileName:    " << DataFileName << std::endl;
    std::cout << "AlignmentFileName:    " << RotAlignmentFileName<< std::endl;
    PLTAlignment* TransAlignment = Event2.GetAlignment();
    x_position.clear();
    y_position.clear();
    r_position.clear();
    angle.clear();
    std::map<int, TH1F*>  h_xResiduals2;
    std::map<int, TH1F*>  h_yResiduals2;
    std::map<int, TH2F*>  h_xdyResiduals2;
    std::map<int, TH2F*>  h_ydxResiduals2;
    std::map<int, std::pair<TH1F*, TH1F*> > h_Slopes2; 

    for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
      ++ChN;
      if (h_Slopes2.count(*ich) == 0){
        const char* BUFF =  Form ("Y_Slope_Ch%02i_Second", *ich);
        h_Slopes2[*ich].second = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
        const char* BUFF2 =  Form ("X_Slope_Ch%02i_Second", *ich);
        h_Slopes2[*ich].first = new TH1F( BUFF2, BUFF2, 100, -0.2, 0.2);
      }
      for (int iroc=0; iroc<3; ++iroc){
        int id = 10 *(*ich) + iroc;
        if (h_xResiduals2.count(id) == 0){
          const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i_Second", *ich, iroc);
          h_xResiduals2[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
        }
        if (h_yResiduals2.count(id) == 0){
          const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i_Second", *ich, iroc);
          h_yResiduals2[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
        }
        if (h_xdyResiduals2.count(id) == 0){
          const char* BUFF =  Form ("XdY_Residual_Ch%02i_ROC%1i_Second", *ich, iroc);
          h_xdyResiduals2[id] = new TH2F( BUFF, BUFF, 133, -1, 0.995, 100, -0.5, 0.5);
        }
        if (h_ydxResiduals2.count(id) == 0){
          const char* BUFF =  Form ("YdX_Residual_Ch%02i_ROC%1i_Second", *ich, iroc);
          h_ydxResiduals2[id] = new TH2F( BUFF, BUFF, 133, -1, 0.995, 100, -0.5, 0.5);
        }
      }
    } 
    for (int ientry = 0; Event2.GetNextEvent() >= 0; ++ientry) {
      if (ientry % 10000 == 0) {
        std::cout << "Processing entry: " << ientry << std::endl;
      }


      // Loop over all planes with hits in event
      for (size_t it = 0; it != Event2.NTelescopes(); ++it) {
        std::cout<<"Events"<<std::endl;
        // THIS telescope is
        PLTTelescope* Telescope = Event2.Telescope(it);
        if (Telescope->NTracks() == 1 && Telescope->NClusters() == 3) {
          int channel = Telescope->Channel();
          // Grab the track
          ++TracksN;
          PLTTrack* Track = Telescope->Track(0);
          float xslope, yslope;
          xslope = (Track->TX(7.54)-Track->TX(0.0))/3;
          yslope = (Track->TY(7.54)-Track->TY(0.0))/3;
          std::cout<<"slopes"<<std::endl;      
          h_Slopes2[channel].first->Fill(xslope);
          h_Slopes2[channel].second->Fill(yslope);
          for (int iroc = 0; iroc <= 2; ++iroc){
            PLTCluster* Cluster = Track->Cluster(iroc);
            std::cout<<"cluster"<<std::endl;
            float myLResidualX;
            float myLX;
            myLX = Cluster->LX();
            myLResidualX = myLX - Track->Cluster(0)->LX();
            float myLResidualY;
            float myLY;
            myLY = Cluster->LY();
            myLResidualY = myLY - Track->Cluster(0)->LY();
            int PlaneID = 10*channel+iroc;
            h_xResiduals2[PlaneID]->Fill(myLResidualX);
            h_yResiduals2[PlaneID]->Fill(myLResidualY);
            h_xdyResiduals2[PlaneID]->Fill(myLX,myLResidualY);
            h_ydxResiduals2[PlaneID]->Fill(myLY,myLResidualX);
            //                    std::cout<< "Plane: "<<iroc<< " slopeX  " << xslope << " slopeY  " << yslope << " real x " << Track->Cluster(iroc)->TX()<< " real y " << Track->Cluster(iroc)->TY()<< " real z " << Track->Cluster(iroc)->TZ()<< " ResidualY  " << myLResidualY <<" residualX "<< myLResidualX<< std::endl;
          }
        }
      }
    }

    std::cout << "Number of Tracks accepted by PLTTracking: "<< TracksN << std::endl;
    for (std::map<int, TH1F*>::iterator it = h_xResiduals2.begin(); it !=h_xResiduals2.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){

        const char* BUFF = Form("./plots/Alignment/X_Residual_Ch%02i_ROC%i_Second.gif",Channel,ROC); 
        canvas1.cd();
        h_xResiduals2[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, TH1F*>::iterator it = h_yResiduals2.begin(); it !=h_yResiduals2.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/Y_Residual_Ch%02i_ROC%i_Second.gif",Channel,ROC); 
        canvas1.cd();
        h_yResiduals2[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, std::pair<TH1F*, TH1F*> >::iterator it = h_Slopes2.begin(); it !=h_Slopes2.end(); ++it){
      int const Channel = it->first;
      if (it->second.second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/X_Y_Slopes_Ch%02i.gif",Channel); 
        canvas3.cd(1);
        h_Slopes2[Channel].first->Draw();
        canvas3.cd(2);
        h_Slopes2[Channel].second->Draw();
        canvas3.SaveAs(BUFF);
      }
      else continue;
    }
    //do Translational Correction
    for (std::map<int, TH1F*>::iterator it = h_xResiduals2.begin(); it !=h_xResiduals2.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (x_position.count(id)==0){x_position[id] = (h_xResiduals2[id]->GetMean());}
      if (y_position.count(id)==0){y_position[id] = (h_yResiduals2[id]->GetMean());}
      PLTAlignment::CP* ConstMap = TransAlignment->GetCP(Channel,ROC);  
      ConstMap->LX = ConstMap->LX + x_position[id];
      ConstMap->LY = ConstMap->LY + y_position[id];
    }    
    std::string TransAlignmentFileName =  "./Trans_Alignment.dat";
    TransAlignment->WriteAlignmentFile( TransAlignmentFileName );

    for (std::map<int, TH1F*>::iterator it = h_xResiduals2.begin(); it !=h_xResiduals2.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, TH1F*>::iterator it = h_yResiduals2.begin(); it !=h_yResiduals2.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, std::pair<TH1F*, TH1F*> >::iterator it = h_Slopes2.begin(); it != h_Slopes2.end(); ++it){
      delete it->second.first; 
      delete it->second.second; 
    }




    //****************************************************************
    //
    //Third time, just for final plots of residuals. no correction here
    PLTEvent Event3(DataFileName, GainCalFileName, TransAlignmentFileName);
    Event3.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
    Event3.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

    PLTAlignment* FinAlignment = Event3.GetAlignment();
    gStyle->SetOptStat(1111);
    std::map<int, TH1F*>  h_xResiduals3;
    std::map<int, TH1F*>  h_yResiduals3;
    std::map<int, TH2F*>  h_xdyResiduals3;
    std::map<int, TH2F*>  h_ydxResiduals3;
    std::map<int, std::pair<TH1F*, TH1F*> > h_Slopes3; 
    TracksN = 0;
    Channel.clear();
    Channel = FinAlignment->GetListOfChannels(); 
    ChN = 1;
    for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
      ++ChN;
      if (h_Slopes3.count(*ich) == 0){
        const char* BUFF =  Form ("Y_Slope_Ch%02i_Third", *ich);
        h_Slopes3[*ich].second = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
        const char* BUFF2 =  Form ("X_Slope_Ch%02i_Third", *ich);
        h_Slopes3[*ich].first = new TH1F( BUFF2, BUFF2, 100, -0.2, 0.2);
      }
      for (int iroc=0; iroc<3; ++iroc){
        int id = 10 *(*ich) + iroc;
        if (h_xResiduals3.count(id) == 0){
          const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
          h_xResiduals3[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
        }
        if (h_yResiduals3.count(id) == 0){
          const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
          h_yResiduals3[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
        }
        if (h_xdyResiduals3.count(id) == 0){
          const char* BUFF =  Form ("XdY_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
          h_xdyResiduals3[id] = new TH2F( BUFF, BUFF, 133, -1, 0.995, 100, -0.5, 0.5);
        }
        if (h_ydxResiduals3.count(id) == 0){
          const char* BUFF =  Form ("YdX_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
          h_ydxResiduals3[id] = new TH2F( BUFF, BUFF, 133, -1, 0.995, 100, -0.5, 0.5);
        }
      }
    } 
    // Loop over all events in file
    for (int ientry = 0; Event3.GetNextEvent() >= 0; ++ientry) {
      if (ientry % 10000 == 0) {
        std::cout << "Processing entry: " << ientry << std::endl;
      }

      // Loop over all planes with hits in event
      for (size_t it = 0; it != Event3.NTelescopes(); ++it) {

        // THIS telescope is
        PLTTelescope* Telescope = Event3.Telescope(it);
        if (Telescope->NTracks() == 1 && Telescope->NClusters() == 3) {
          int channel = Telescope->Channel();
          // Grab the track
          ++TracksN;
          PLTTrack* Track = Telescope->Track(0);
          float xslope, yslope;
          xslope = (Track->TX(7.54)-Track->TX(0.0))/3;
          yslope = (Track->TY(7.54)-Track->TY(0.0))/3;
          h_Slopes3[channel].first->Fill(xslope);
          h_Slopes3[channel].second->Fill(yslope);
          for (int iroc = 0; iroc <= 2; ++iroc){
            PLTCluster* Cluster = Track->Cluster(iroc);
            float myLResidualX;
            float myLX;
            myLX = Cluster->LX();
            myLResidualX = myLX - Track->Cluster(0)->LX();
            float myLResidualY;
            float myLY;
            myLY = Cluster->LY();
            myLResidualY = myLY - Track->Cluster(0)->LY();
            int PlaneID = 10*channel+iroc;
            h_xResiduals3[PlaneID]->Fill(myLResidualX);
            h_yResiduals3[PlaneID]->Fill(myLResidualY);
            h_xdyResiduals3[PlaneID]->Fill(myLX,myLResidualY);
            h_ydxResiduals3[PlaneID]->Fill(myLY,myLResidualX);
            //          std::cout<< "Plane: "<<iroc<< " slopeX  " << xslope << " slopeY  " << yslope << " real x " << Track->Cluster(iroc)->TX()<< " real y " << Track->Cluster(iroc)->TY()<< " real z " << Track->Cluster(iroc)->TZ()<< " ResidualY  " << myLResidualY <<" residualX "<< myLResidualX<< std::endl;
          }
        }
      }
    }

    std::cout << "Number of Tracks accepted by PLTTracking: "<< TracksN << std::endl;
    x_position.clear();
    y_position.clear();
    r_position.clear();
    angle.clear();

    for (std::map<int, TH1F*>::iterator it = h_xResiduals3.begin(); it !=h_xResiduals3.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){

        const char* BUFF = Form("./plots/Alignment/X_Residual_Ch%02i_ROC%i_Third.gif",Channel,ROC); 
        canvas1.cd();
        h_xResiduals3[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, TH1F*>::iterator it = h_yResiduals3.begin(); it !=h_yResiduals3.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/Y_Residual_Ch%02i_ROC%i_Third.gif",Channel,ROC); 
        canvas1.cd();
        h_yResiduals3[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, TH2F*>::iterator it = h_xdyResiduals3.begin(); it !=h_xdyResiduals3.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/XdY_Residual_Ch%02i_ROC%i_Third.gif",Channel,ROC); 
        canvas1.cd(1);
        h_xdyResiduals3[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, TH2F*>::iterator it = h_ydxResiduals3.begin(); it !=h_ydxResiduals3.end(); ++it){
      int const Channel = it->first / 10;
      int const ROC     = it->first % 10;
      int const id      = it->first;
      if (it->second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/YdX_Residual_Ch%02i_ROC%i_Third.gif",Channel,ROC); 
        canvas1.cd(1);
        h_ydxResiduals3[id]->Draw();
        canvas1.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, std::pair<TH1F*,TH1F*> >::iterator it = h_Slopes3.begin(); it !=h_Slopes3.end(); ++it){
      int const Channel = it->first;
      if (it->second.second->GetEntries() != 0){
        const char* BUFF = Form("./plots/Alignment/X_Y_Slopes_Ch%02i_Third.gif",Channel); 
        canvas3.cd(1);
        h_Slopes3[Channel].first->Draw();
        canvas3.cd(2);
        h_Slopes3[Channel].second->Draw();
        canvas3.SaveAs(BUFF);
      }
      else continue;
    }
    for (std::map<int, TH1F*>::iterator it = h_xResiduals3.begin(); it !=h_xResiduals3.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, TH1F*>::iterator it = h_yResiduals3.begin(); it !=h_yResiduals3.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, TH2F*>::iterator it = h_xdyResiduals3.begin(); it !=h_xdyResiduals3.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, TH2F*>::iterator it = h_ydxResiduals3.begin(); it !=h_ydxResiduals3.end(); ++it){
      delete it->second; 
    }
    for (std::map<int, std::pair<TH1F*, TH1F*> >::iterator it = h_Slopes3.begin(); it != h_Slopes3.end(); ++it){
      delete it->second.first; 
      delete it->second.second; 
    }
    return(0);
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
