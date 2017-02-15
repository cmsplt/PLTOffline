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
#include "TProfile.h"
#include "TFile.h"
//Calculate alignment takes a data file of hopefully straight tracks and aligns the planes in all telescopes behind the first plane in the ideal way.
//produces ROTATED_Alignment.dat after the fixes to Local Rotation and then Trans_Alignment after fixing the X and Y offsets remaining after rot fix.

int GenerateAlignment (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName:    " << GainCalFileName << std::endl;
  //system( "head /data/dhidas/PLTMC/GainCalFits_Test.dat");
  
  // Set some basic style
  
  PLTU::SetStyle();
  TFile *f = new TFile("histo_calculatealignment.root","RECREATE");
  TCanvas canvas1;
  TCanvas canvas2("Canvas2","Canvas2",800,600);
  TCanvas canvas3;
  canvas3.Divide(2,1);
  int TracksN = 0;
  std::vector<int> Channel ;
  int ChN = 1;

  // Grab the plt event reader
  //****************************************************************************************
  //First Corrections --rotational
  //input "InitialAlignment" output "RotatedAlignment"
  PLTEvent Event1(DataFileName, GainCalFileName, AlignmentFileName);
  Event1.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event1.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "AlignmentFileName:    " << AlignmentFileName<< std::endl;

  PLTAlignment* RotatedAlignment = Event1.GetAlignment();
  gStyle->SetOptStat(1111);
  std::map<int, TH1F*>  h_xResiduals1;
  std::map<int, TH1F*>  h_yResiduals1;
  std::map<int, TProfile*>  h_xdyResiduals1;
  std::map<int, TProfile*>  h_ydxResiduals1;
  std::map<int, std::pair<TH1F*, TH1F*> > h_Slopes1; 

  TracksN = 0;
  Channel.clear();
  Channel = RotatedAlignment->GetListOfChannels(); 
  //Channel = OldAlignment->GetListOfChannels(); 

  ChN = 1;
  for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
    ++ChN;
    if (h_Slopes1.count(*ich) == 0){
      const char* BUFF =  Form ("Y_Slope_Ch%02i_First", *ich);
      h_Slopes1[*ich].second = new TH1F( BUFF, BUFF, 50, -0.1, 0.1);
      const char* BUFF2 =  Form ("X_Slope_Ch%02i_First", *ich);
      h_Slopes1[*ich].first = new TH1F( BUFF2, BUFF2, 50, -0.1, 0.1);
    }
    for (int iroc=0; iroc<3; ++iroc){
      int id = 10 *(*ich) + iroc;
      if (h_xResiduals1.count(id) == 0){
        const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i_First", *ich, iroc);
        h_xResiduals1[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      }
      if (h_yResiduals1.count(id) == 0){
        const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i_First", *ich, iroc);
        h_yResiduals1[id] = new TH1F( BUFF, BUFF, 100, -0.05, 0.35);
      }
      if (h_xdyResiduals1.count(id) == 0){
        const char* BUFF =  Form ("XdY_NormalizedResidual_Ch%02i_ROC%1i_First", *ich, iroc);
        h_xdyResiduals1[id] = new TProfile( BUFF, BUFF, 40, -0.5, 0.5, -0.5, 0.5);
      }
      if (h_ydxResiduals1.count(id) == 0){
        const char* BUFF =  Form ("YdX_NormalizedResidual_Ch%02i_ROC%1i_First", *ich, iroc);
        h_ydxResiduals1[id] = new TProfile( BUFF, BUFF, 40, -0.5, 0.5, -0.5, 0.5);
      }
    }
  } 
  // Loop over all events in file
  std::map<int,float> x_position;
  std::map<int,float> y_position;
  std::map<int,float> r_position;
  std::map<int,float> angle;
  for (int ientry1 = 0; Event1.GetNextEvent() >= 0; ++ientry1) {
    if (ientry1 % 100000 == 0) {
      std::cout << "Processing entry: " << ientry1 << std::endl;
    }
    if (ientry1>=3000000){break;}

    // skip miniscans (fill 4892)
    // if (Event1.Time()>61320000 && Event1.Time()<61920000) continue;
    // if (Event1.Time()>13200000+24*3600*1000 && Event1.Time()<13620000+24*3600*1000) continue;
    // skip miniscans (fill 4895)
    // if (Event1.Time()>47400000 && Event1.Time()<47820000) continue;
    // if (Event1.Time()>72840000 && Event1.Time()<73260000) continue;

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event1.NTelescopes(); ++it) {
      // THIS telescope is
      PLTTelescope* Telescope = Event1.Telescope(it);
      if (Telescope->NTracks() == 1 && Telescope->NClusters() == 3) {
        int channel = Telescope->Channel();
        // Grab the track
        ++TracksN;
        PLTTrack* Track = Telescope->Track(0);
        float xslope, yslope;
        xslope = (Track->Cluster(2)->TX()-Track->Cluster(0)->TX())/7.54;
        yslope = (Track->Cluster(2)->TY()-Track->Cluster(0)->TY())/7.54;
        h_Slopes1[channel].first->Fill(xslope);
        h_Slopes1[channel].second->Fill(yslope);
        for (int iroc = 0; iroc <= 2; ++iroc){
          PLTCluster* Cluster = Track->Cluster(iroc);
          float myLResidualX;
          float myTX;
          myTX = Cluster->TX();
          myLResidualX = myTX - Track->Cluster(0)->TX();
          float myLResidualY;
          float myTY;
          float myLY;
          myLY = 0.102 * iroc;
          myTY = Cluster->TY();
          myLResidualY = myTY - (Track->Cluster(0)->TY());
          int PlaneID = 10*channel+iroc;
          h_xResiduals1[PlaneID]->Fill(myLResidualX);
          h_yResiduals1[PlaneID]->Fill(myLResidualY);
          h_xdyResiduals1[PlaneID]->Fill(myTX,myLResidualY-myLY);
          h_ydxResiduals1[PlaneID]->Fill(myTY,myLResidualX);
        }
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
  std::map<int,TF1*> func;
  for (std::map<int, TProfile*>::iterator it = h_xdyResiduals1.begin(); it !=h_xdyResiduals1.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    if (it->second->GetEntries() != 0){
      const char* BUFF = Form("./plots/Alignment/XdY_Residual_Ch%02i_ROC%i_First.gif",Channel,ROC); 
      canvas1.cd(1);
      h_xdyResiduals1[id]->Fit("pol1","","",-0.3,0.3); 
      h_xdyResiduals1[id]->Draw();
      canvas1.SaveAs(BUFF);
      func[id] = h_xdyResiduals1[id]->GetFunction("pol1");
    }
    else continue;
  }
  for (std::map<int, TProfile*>::iterator it = h_ydxResiduals1.begin(); it !=h_ydxResiduals1.end(); ++it){
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
  for (std::map<int, TProfile*>::iterator it = h_xdyResiduals1.begin(); it != h_xdyResiduals1.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    angle[id] = 0;
    if (r_position.count(id)==0 && it->second->GetEntries() != 0 && ROC != 0 ) {
      r_position[id] = func[id]->GetParameter(1);
      angle[id] = atan(r_position[id]); 
    std::cout << "ChannelRoc: " << Channel<< ":" << ROC<< "  "<<func[id]->GetParName(1)<<" " << r_position[id] << " angle: " << angle[id]<< std::endl;
    }
    f->Write();
    //std::cout << &NewAlignment << " vs " << OldAlignment << std::endl;
//    PLTAlignment::CP* ConstMap2 = RotatedAlignment->GetCP(Channel,ROC);  
    RotatedAlignment->AddToLR(Channel,ROC,-angle[id]);

  }    
  std::string RotAlignmentFileName =  "./ROTATED_Alignment.dat";
  RotatedAlignment->WriteAlignmentFile( RotAlignmentFileName );
  for (std::map<int, TH1F*>::iterator it = h_xResiduals1.begin(); it !=h_xResiduals1.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TH1F*>::iterator it = h_yResiduals1.begin(); it !=h_yResiduals1.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TProfile*>::iterator it = h_xdyResiduals1.begin(); it !=h_xdyResiduals1.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TProfile*>::iterator it = h_ydxResiduals1.begin(); it !=h_ydxResiduals1.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, std::pair<TH1F*, TH1F*> >::iterator it = h_Slopes1.begin(); it != h_Slopes1.end(); ++it){
    delete it->second.first; 
    delete it->second.second; 
  }
  //********************************************************************************************************************
  //Second Step --translation
  //input "RotatedAlignment" output "TransAlignment"
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
  std::map<int, TProfile*>  h_xdyResiduals2;
  std::map<int, TProfile*>  h_ydxResiduals2;
  std::map<int, std::pair<TH1F*, TH1F*> > h_Slopes2; 

  for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
    ++ChN;
    if (h_Slopes2.count(*ich) == 0){
      const char* BUFF =  Form ("Y_Slope_Ch%02i_Second", *ich);
      h_Slopes2[*ich].second = new TH1F( BUFF, BUFF, 50, -0.1, 0.1);
      const char* BUFF2 =  Form ("X_Slope_Ch%02i_Second", *ich);
      h_Slopes2[*ich].first = new TH1F( BUFF2, BUFF2, 50, -0.1, 0.1);
    }
    for (int iroc=0; iroc<3; ++iroc){
      int id = 10 *(*ich) + iroc;
      if (h_xResiduals2.count(id) == 0){
        const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i_Second", *ich, iroc);
        h_xResiduals2[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      }
      if (h_yResiduals2.count(id) == 0){
        const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i_Second", *ich, iroc);
        h_yResiduals2[id] = new TH1F( BUFF, BUFF, 100, -0.05, 0.35);
      }
      if (h_xdyResiduals2.count(id) == 0){
        const char* BUFF =  Form ("XdY_NormalizedResidual_Ch%02i_ROC%1i_Second", *ich, iroc);
        h_xdyResiduals2[id] = new TProfile( BUFF, BUFF, 40, -0.5, 0.5, -0.5, 0.5);
      }
      if (h_ydxResiduals2.count(id) == 0){
        const char* BUFF =  Form ("YdX_NormalizedResidual_Ch%02i_ROC%1i_Second", *ich, iroc);
        h_ydxResiduals2[id] = new TProfile( BUFF, BUFF, 40, -0.5, 0.5, -0.5, 0.5);
      }
    }
  } 
  for (int ientry = 0; Event2.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 100000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    if (ientry>=3000000){break;}

    // skip miniscans (fill 4892)
    // if (Event2.Time()>61320000 && Event2.Time()<61920000) continue;
    // if (Event2.Time()>13200000+24*3600*1000 && Event2.Time()<13620000+24*3600*1000) continue;
    // skip miniscans (fill 4895)
    // if (Event2.Time()>47400000 && Event2.Time()<47820000) continue;
    // if (Event2.Time()>72840000 && Event2.Time()<73260000) continue;

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event2.NTelescopes(); ++it) {
      // THIS telescope is
      PLTTelescope* Telescope = Event2.Telescope(it);
      if (Telescope->NTracks() == 1 && Telescope->NClusters() == 3) {
        int channel = Telescope->Channel();
        // Grab the track
        ++TracksN;
        PLTTrack* Track = Telescope->Track(0);
        float xslope, yslope;
        xslope = (Track->Cluster(2)->TX()-Track->Cluster(0)->TX())/7.54;
        yslope = (Track->Cluster(2)->TY()-Track->Cluster(0)->TY())/7.54;
        h_Slopes2[channel].first->Fill(xslope);
        h_Slopes2[channel].second->Fill(yslope);
        for (int iroc = 0; iroc <= 2; ++iroc){
          PLTCluster* Cluster = Track->Cluster(iroc);
          float myLResidualX;
          float myTX;
          myTX = Cluster->TX();
          myLResidualX = myTX - Track->Cluster(0)->TX();
          float myLResidualY;
          float myTY;
          float myLY;
          myLY = 0.102 * iroc;
          myTY = Cluster->TY();
          myLResidualY = myTY - (Track->Cluster(0)->TY());
          int PlaneID = 10*channel+iroc;
          h_xResiduals2[PlaneID]->Fill(myLResidualX);
          h_yResiduals2[PlaneID]->Fill(myLResidualY);
          h_xdyResiduals2[PlaneID]->Fill(myTX,myLResidualY-myLY);
          h_ydxResiduals2[PlaneID]->Fill(myTY,myLResidualX);
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
  for (std::map<int, TProfile*>::iterator it = h_xdyResiduals2.begin(); it !=h_xdyResiduals2.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    if (it->second->GetEntries() != 0){
      const char* BUFF = Form("./plots/Alignment/XdY_Residual_Ch%02i_ROC%i_Second.gif",Channel,ROC); 
      canvas1.cd(1);
      h_xdyResiduals2[id]->Fit("pol1","","",-0.3,0.3); 
      h_xdyResiduals2[id]->Draw();
      canvas1.SaveAs(BUFF);
    }
    else continue;
  }
  for (std::map<int, TProfile*>::iterator it = h_ydxResiduals2.begin(); it !=h_ydxResiduals2.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    if (it->second->GetEntries() != 0){
      const char* BUFF = Form("./plots/Alignment/YdX_Residual_Ch%02i_ROC%i_Second.gif",Channel,ROC); 
      canvas1.cd(1);
      h_ydxResiduals2[id]->Draw();
      canvas1.SaveAs(BUFF);
    }
    else continue;
  }
  for (std::map<int, std::pair<TH1F*, TH1F*> >::iterator it = h_Slopes2.begin(); it !=h_Slopes2.end(); ++it){
    int const Channel = it->first;
    if (it->second.second->GetEntries() != 0){
      const char* BUFF = Form("./plots/Alignment/X_Y_Slopes_Ch%02i_Second.gif",Channel); 
      canvas3.cd(1);
      h_Slopes2[Channel].first->Draw();
      canvas3.cd(2);
      h_Slopes2[Channel].second->Draw();
      canvas3.SaveAs(BUFF);
    }
    else continue;
  }
  f->Write();
  //do Translational Correction
  for (std::map<int, TH1F*>::iterator it = h_xResiduals2.begin(); it !=h_xResiduals2.end(); ++it){
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
    if (x_position.count(id)==0){x_position[id] = (h_xResiduals2[id]->GetMean());}
    if (y_position.count(id)==0){y_position[id] = (h_yResiduals2[id]->GetMean());}
    PLTAlignment::CP* ConstMap = TransAlignment->GetCP(Channel,ROC);  
    //ConstMap->LX = ConstMap->LX - x_position[id];
    //ConstMap->LY = ConstMap->LY - y_position[id];
    TransAlignment->AddToLX(Channel,ROC,-x_position[id]);
    TransAlignment->AddToLY(Channel,ROC,-(y_position[id]-ConstMap->LY));
  }    
  std::string TransAlignmentFileName =  "./Trans_Alignment.dat";
  TransAlignment->WriteAlignmentFile( TransAlignmentFileName );

  for (std::map<int, TH1F*>::iterator it = h_xResiduals2.begin(); it !=h_xResiduals2.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TH1F*>::iterator it = h_yResiduals2.begin(); it !=h_yResiduals2.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TProfile*>::iterator it = h_xdyResiduals2.begin(); it !=h_xdyResiduals2.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TProfile*>::iterator it = h_ydxResiduals2.begin(); it !=h_ydxResiduals2.end(); ++it){
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
  std::map<int, TProfile*>  h_xdyResiduals3;
  std::map<int, TProfile*>  h_ydxResiduals3;
  std::map<int, std::pair<TH1F*, TH1F*> > h_Slopes3; 
  TracksN = 0;
  Channel.clear();
  Channel = FinAlignment->GetListOfChannels(); 
  ChN = 1;
  for (std::vector<int>::iterator ich = Channel.begin(); ich != Channel.end(); ++ich){
    ++ChN;
    if (h_Slopes3.count(*ich) == 0){
      const char* BUFF =  Form ("Y_Slope_Ch%02i_Third", *ich);
      h_Slopes3[*ich].second = new TH1F( BUFF, BUFF, 50, -0.1, 0.1);
      const char* BUFF2 =  Form ("X_Slope_Ch%02i_Third", *ich);
      h_Slopes3[*ich].first = new TH1F( BUFF2, BUFF2, 50, -0.1, 0.1);
    }
    for (int iroc=0; iroc<3; ++iroc){
      int id = 10 *(*ich) + iroc;
      if (h_xResiduals3.count(id) == 0){
        const char* BUFF = Form("X_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
        h_xResiduals3[id] = new TH1F( BUFF, BUFF, 100, -0.2, 0.2);
      }
      if (h_yResiduals3.count(id) == 0){
        const char* BUFF =  Form ("Y_Residual_Ch%02i_ROC%1i_Third", *ich, iroc);
        h_yResiduals3[id] = new TH1F( BUFF, BUFF, 100, -0.05, 0.35);
      }
      if (h_xdyResiduals3.count(id) == 0){
        const char* BUFF =  Form ("XdY_NormalizedResidual_Ch%02i_ROC%1i_Third", *ich, iroc);
        h_xdyResiduals3[id] = new TProfile( BUFF, BUFF, 40, -0.5, 0.5, -0.5, 0.5);
      }
      if (h_ydxResiduals3.count(id) == 0){
        const char* BUFF =  Form ("YdX_NormalizedResidual_Ch%02i_ROC%1i_Third", *ich, iroc);
        h_ydxResiduals3[id] = new TProfile( BUFF, BUFF, 40, -0.5, 0.5, -0.5, 0.5);
      }
    }
  } 
  // Loop over all events in file
  for (int ientry = 0; Event3.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 100000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    if (ientry>=3000000){break;}

    // skip miniscans (fill 4892)
    // if (Event3.Time()>61320000 && Event3.Time()<61920000) continue;
    // if (Event3.Time()>13200000+24*3600*1000 && Event3.Time()<13620000+24*3600*1000) continue;
    // skip miniscans (fill 4895)
    // if (Event3.Time()>47400000 && Event3.Time()<47820000) continue;
    // if (Event3.Time()>72840000 && Event3.Time()<73260000) continue;

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
        xslope = (Track->Cluster(2)->TX()-Track->Cluster(0)->TX())/7.54;
        yslope = (Track->Cluster(2)->TY()-Track->Cluster(0)->TY())/7.54;
        h_Slopes3[channel].first->Fill(xslope);
        h_Slopes3[channel].second->Fill(yslope);
        for (int iroc = 0; iroc <= 2; ++iroc){
          PLTCluster* Cluster = Track->Cluster(iroc);
          float myLResidualX;
          float myTX;
          myTX = Cluster->TX();
          myLResidualX = myTX - Track->Cluster(0)->TX();
          float myLResidualY;
          float myTY;
          float myLY;
          myLY = 0.102 * iroc;
          myTY = Cluster->TY();
          myLResidualY = myTY - (Track->Cluster(0)->TY());
          int PlaneID = 10*channel+iroc;
          h_xResiduals3[PlaneID]->Fill(myLResidualX);
          h_yResiduals3[PlaneID]->Fill(myLResidualY);
          h_xdyResiduals3[PlaneID]->Fill(myTX,myLResidualY-myLY);
          h_ydxResiduals3[PlaneID]->Fill(myTY,myLResidualX);
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
  for (std::map<int, TProfile*>::iterator it = h_xdyResiduals3.begin(); it !=h_xdyResiduals3.end(); ++it){
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
  for (std::map<int, TProfile*>::iterator it = h_ydxResiduals3.begin(); it !=h_ydxResiduals3.end(); ++it){
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
  f->Write();
  for (std::map<int, TH1F*>::iterator it = h_xResiduals3.begin(); it !=h_xResiduals3.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TH1F*>::iterator it = h_yResiduals3.begin(); it !=h_yResiduals3.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TProfile*>::iterator it = h_xdyResiduals3.begin(); it !=h_xdyResiduals3.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TProfile*>::iterator it = h_ydxResiduals3.begin(); it !=h_ydxResiduals3.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, std::pair<TH1F*, TH1F*> >::iterator it = h_Slopes3.begin(); it != h_Slopes3.end(); ++it){
    delete it->second.first; 
    delete it->second.second; 
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




/* extra stuff from old version goes before translational step.


  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);

  PLTAlignment* InitialAlignment = Event.GetAlignment();
  gStyle->SetOptStat(1111);
  std::map<int, TH1F*>  h_xResiduals;
  std::map<int, TH1F*>  h_yResiduals;
  std::map<int, std::pair<TH1F*, TH1F*> > h_Slopes; 
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
    if (ientry % 100000 == 0) {
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
          float myTX;
          myTX = Cluster->TX();
          myLResidualX = myTX - Track->Cluster(0)->TX();
          float myLResidualY;
          float myTY;
          myTY = Cluster->TY() - 0.204;
          myLResidualY = myTY - (Track->Cluster(0)->TY() - 0.204);
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
  for (std::map<int, TH1F*>::iterator it = h_xResiduals.begin(); it !=h_xResiduals.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, TH1F*>::iterator it = h_yResiduals.begin(); it !=h_yResiduals.end(); ++it){
    delete it->second; 
  }
  for (std::map<int, std::pair<TH1F*, TH1F*> >::iterator it = h_Slopes.begin(); it != h_Slopes.end(); ++it){
    delete it->second.first; 
    delete it->second.second; 
  }
  std::cout << "Number of Tracks accepted by PLTTracking: "<< TracksN << std::endl;

  */
