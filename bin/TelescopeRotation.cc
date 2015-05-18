////////////////////////////////////////////////////////////////////
//
// Fixed by Grant Riley
//
// Created on: May 15 2015
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"

#include "PLTTrack.h"
#include "PLTAlignment.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"
#include "TFile.h"

int TelescopeRotation (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  // Set some basic style
  PLTU::SetStyle();
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  TFile *f = new TFile("histo_telescoperotation.root","RECREATE");

  // Map for all ROC hists and canvas
  std::map<int, TH1F*> hMap;
  std::map<int, TCanvas*> cMap;
  std::map<int, TH2F*> gMap;
  std::map<int, TH1F*> hClusterSize;
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    for (unsigned it = 0; it != Event.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event.Telescope(it);
      
      int const Channel = Telescope->Channel();
      
      if (Telescope->NTracks() != 1 || Telescope->NClusters() != 3) {
        continue;
      }

      if (cMap.count(Channel) == 0) {
        char BUFF[500];
        sprintf(BUFF, "ClusterSize%i_0", Channel);
        hClusterSize[10*Channel+0] = new TH1F(BUFF, BUFF, 4, 0, 4);
        sprintf(BUFF, "ClusterSize%i_1", Channel);
        hClusterSize[10*Channel+1] = new TH1F(BUFF, BUFF, 4, 0, 3);
        sprintf(BUFF, "ClusterSize%i_2", Channel);
        hClusterSize[10*Channel+2] = new TH1F(BUFF, BUFF, 4, 0, 3);
        sprintf(BUFF, "Plane1_XResidual_Ch%i", Channel);
        hMap[10*Channel + 0] = new TH1F(BUFF, BUFF, 100, -.3, .3);
        sprintf(BUFF, "Plane1_YResidual_Ch%i", Channel);
        hMap[10*Channel + 1] = new TH1F(BUFF, BUFF, 100, -.3, .3);
        sprintf(BUFF, "Plane2_XResidual_Ch%i", Channel);
        hMap[10*Channel + 2] = new TH1F(BUFF, BUFF, 100, -.3, .3);
        sprintf(BUFF, "Plane2_YResidual_Ch%i", Channel);
        hMap[10*Channel + 3] = new TH1F(BUFF, BUFF, 100, -.3, .3);
        sprintf(BUFF, "YVXDiff1_Ch%i", Channel);
        gMap[10*Channel + 0] = new TH2F(BUFF, BUFF,81, -.6, .6, 100, -.3, .3);
        sprintf(BUFF, "XVYDiff1_Ch%i", Channel);
        gMap[10*Channel + 1] = new TH2F(BUFF, BUFF, 60, -.5, .5, 100, -.3, .3);
        sprintf(BUFF, "YVXDiff2_Ch%i", Channel);
        gMap[10*Channel + 2] = new TH2F(BUFF, BUFF, 81, -.6, .6, 100, -.3, .3);
        sprintf(BUFF, "XVYDiff2_Ch%i", Channel);
        gMap[10*Channel + 3] = new TH2F(BUFF, BUFF, 60, -.5, .5, 100, -.3, .3);
        sprintf(BUFF, "plots/Residuals_Ch%i", Channel);
        cMap[Channel] = new TCanvas(BUFF, BUFF, 1600, 900);
        cMap[Channel]->Divide(2,4);
      }
      PLTTrack* Track = Telescope->Track(0);
      if(Track->Cluster(0)->NHits()>=3 ||Track->Cluster(1)->NHits()>=3 ||Track->Cluster(2)->NHits()>=3 ){
        continue;
      }
      float x0 = (Track->Cluster(0)->SeedHit()->Column()-26)*0.015;
      float x1 = (Track->Cluster(1)->SeedHit()->Column()-26)*0.015;
      float x2 = (Track->Cluster(2)->SeedHit()->Column()-26)*0.015;
//      float x0 = Track->Cluster(0)->LX();
//      float x1 = Track->Cluster(1)->LX();
//      float x2 = Track->Cluster(2)->LX();
//      float y0 = Track->Cluster(0)->LY();
//      float y1 = Track->Cluster(1)->LY();
//      float y2 = Track->Cluster(2)->LY();
      float y0 = (Track->Cluster(0)->SeedHit()->Row()-40)*0.01;
      float y1 = (Track->Cluster(1)->SeedHit()->Row()-40)*0.01;
      float y2 = (Track->Cluster(2)->SeedHit()->Row()-40)*0.01;
      float z1 = 3.77;
      float z2 = 7.54;

      float PXDiff2 = (x2) - (x0+(z2)*((x1-x0)/(z1)));
      float PXDiff1 = (x1) - (x0+(z1)*((x2-x0)/(z2)));
      float PYDiff2 = (y2) - (y0+(z2)*((y1-y0)/(z1)));
      float PYDiff1 = (y1) - (y0+(z1)*((y2-y0)/(z2)));

      hMap[10*Channel + 0]->Fill(PXDiff1);
      hMap[10*Channel + 1]->Fill(PYDiff1);
      hMap[10*Channel + 2]->Fill(PXDiff2);
      hMap[10*Channel + 3]->Fill(PYDiff2);
      gMap[10*Channel + 0]->Fill(y1,PXDiff1);
      gMap[10*Channel + 1]->Fill(x1,PYDiff1);
      gMap[10*Channel + 2]->Fill(y2,PXDiff2);
      gMap[10*Channel + 3]->Fill(x2,PYDiff2);
      for (int N=0; N<3; ++N){
        if (Track->Cluster(N)->NHits()==1){hClusterSize[10*Channel+N]->Fill(1);}
        else if (Track->Cluster(N)->NHits()==2){hClusterSize[10*Channel+N]->Fill(2);}
        else if (Track->Cluster(N)->NHits()>=3){hClusterSize[10*Channel+N]->Fill(3);}
        else {hClusterSize[10*Channel+N]->Fill(0);}
      }
    }
  }

  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    gStyle->SetOptStat(1111);
    it->second->cd(1);
    hMap[10*it->first + 0]->Draw();
    it->second->cd(2);
    hMap[10*it->first + 1]->Draw();
    it->second->cd(3);
    hMap[10*it->first + 2]->Draw();
    it->second->cd(4);
    hMap[10*it->first + 3]->Draw();
    it->second->cd(5);
    gMap[10*it->first + 0]->Draw("colz");
    it->second->cd(6);
    gMap[10*it->first + 1]->Draw("colz");
    it->second->cd(7);
    gMap[10*it->first + 2]->Draw("colz");
    it->second->cd(8);
    gMap[10*it->first + 3]->Draw("colz");
    it->second->SaveAs(TString(it->second->GetName()) + ".gif");

  }
  f->Write();
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4 || argc != 5) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName]" << " [GainCalFileName]" << " [AlignmentFileName]"  << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];

  TelescopeRotation(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
