////////////////////////////////////////////////////////////////////
//
//
// Working version Created by Grant Riley <Grant.Riley@cern.ch>
// Thrus Nov 06 2015
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
#include "TH1F.h"
#include "TF1.h"
#include "TFile.h"
#include "TTree.h"
//makes plots and a root file which contains events with 3-plane tracks. gives all the information we need to do an accidential analysis.

int AccidentalAnalysis (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName:    " << GainCalFileName << std::endl;

  PLTU::SetStyle();
  TFile *f = new TFile("ttree_accidentalanalysis.root","RECREATE");
  TTree *ttree = new TTree("Events","Events");
  // Grab the plt event reader
  //****************************************************************************************
  PLTEvent Event1(DataFileName, GainCalFileName, AlignmentFileName);
  Event1.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event1.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "AlignmentFileName:    " << AlignmentFileName<< std::endl;

  //PLTAlignment* BaseAlignment = Event1.GetAlignment();
  gStyle->SetOptStat(1111);
  Int_t MAX = 20;
  Int_t Evn, Evt, NCl, NTk, IBestTk, ChN;
  Float_t BsX[MAX], BsY[MAX], Sx[MAX], Sy[MAX], Rx[MAX], Ry[MAX], D[MAX];

  ttree->Branch("EventN",    &Evn,    "EventN/I");
  ttree->Branch("EventT",    &Evt,    "EventT/I");
  ttree->Branch("Channel",   &ChN,    "Channel/I");
  ttree->Branch("NTracks",   &NTk,    "NTracks/I");
  ttree->Branch("NClusters", &NCl,    "NClusters/I");
  ttree->Branch("BestTrackI",&IBestTk,"BestTrackI/I");
  ttree->Branch("SlopeX",    Sx,      "SlopeX[NTracks]/F");
  ttree->Branch("SlopeY",    Sx,      "SlopeY[NTracks]/F");
  ttree->Branch("BeamspotX", BsX,     "BeamspotX[NTracks]/F"); 
  ttree->Branch("BeamspotY", BsY,     "BeamspotY[NTracks]/F");
  ttree->Branch("ResidualX", Rx,      "ResidualX[NTracks]/F");
  ttree->Branch("ResidualY", Ry,      "ResidualY[NTracks]/F");
  ttree->Branch("Distance",  D,       "Distance[NTracks]/F");

//start event looping
  for (int ientry1 = 0; Event1.GetNextEvent() >= 0; ++ientry1) {
    if (ientry1 % 10000 == 0) {
      std::cout << "Processing entry: " << ientry1 << std::endl;
    }
//    if (ientry1>=3000){break;}
    // Loop over all planes with hits in event
    NCl = 0;
    for (size_t it = 0; it != Event1.NTelescopes(); ++it) {
      // THIS telescope is
      PLTTelescope* Telescope = Event1.Telescope(it);
      if (Telescope->NTracks() >= 1 && Telescope->Plane(0)->NClusters() >= 1 && Telescope->Plane(1)->NClusters() >= 1 && Telescope->Plane(2)->NClusters() >= 1){
        ChN = Telescope->Channel();
        //get tracks
        NCl = Telescope->NClusters();
        NTk = 0;
        for (uint ij = 0; ij != Telescope->NTracks(); ++ij) {
          if (Telescope->Track(ij)->NClusters()==3){
            PLTTrack* Track = Telescope->Track(ij);
            Sx[NTk] = (Track->Cluster(2)->TX()-Track->Cluster(0)->TX())/7.54;
            Sy[NTk] = (Track->Cluster(2)->TY()-Track->Cluster(0)->TY())/7.54;
            BsX[NTk] = Track->fPlaner[2][0]; 
            BsY[NTk] = Track->fPlaner[2][1];
            Rx[NTk] = (Track->TX(Telescope->Plane(0)->TZ()) - Track->Cluster(0)->TX())   
                    + (Track->TX(Telescope->Plane(1)->TZ()) - Track->Cluster(1)->TX())  
                    + (Track->TX(Telescope->Plane(2)->TZ()) - Track->Cluster(2)->TX()) ; 
            Ry[NTk] = (Track->TY(Telescope->Plane(0)->TZ()) - Track->Cluster(0)->TY())   
                    + (Track->TY(Telescope->Plane(1)->TZ()) - Track->Cluster(1)->TY())  
                    + (Track->TY(Telescope->Plane(2)->TZ()) - Track->Cluster(2)->TY()) ; 
            D[NTk] = sqrt(pow((Track->TX(Telescope->Plane(0)->TZ()) - Track->Cluster(0)->TX()),2) + 
                          pow((Track->TY(Telescope->Plane(0)->TZ()) - Track->Cluster(0)->TY()),2))
                   + sqrt(pow((Track->TX(Telescope->Plane(1)->TZ()) - Track->Cluster(1)->TX()),2) + 
                          pow((Track->TY(Telescope->Plane(1)->TZ()) - Track->Cluster(1)->TY()),2))
                   + sqrt(pow((Track->TX(Telescope->Plane(2)->TZ()) - Track->Cluster(2)->TX()),2) + 
                          pow((Track->TY(Telescope->Plane(2)->TZ()) - Track->Cluster(2)->TY()),2));

            ++NTk;
          }
        }
        if (NTk > 0){
          Float_t DBest = 9999;
          IBestTk = -99;
          for (int ik = 0; ik != NTk; ++ik) {
            if (D[ik] < DBest) IBestTk = ik; DBest = D[ik];
          }
          ttree->Fill();

        }
      }
    }
  }
  f->Write();
  f->Close();
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

  AccidentalAnalysis(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}

