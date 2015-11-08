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
#include "TMath.h"
//prints a text list of timestamped numbers. the numbers will be percentages of events which are measured but shouldn't have been because they're probably accidentals..!!.1.1.1.1!>!>!

int AccidentalMeasurement (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName:    " << GainCalFileName << std::endl;

  PLTU::SetStyle();
  TFile *f = new TFile("ttree_accidentalprobability.root","RECREATE");
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
  Int_t MAX = 100;
  Int_t Evn, Evt, NCl, NTk, IBestTk, ChN;
  Float_t BsX[MAX], BsY[MAX], Sx[MAX], Sy[MAX], Rx[MAX], Ry[MAX], Prob[MAX];
  Float_t ProbSx[MAX], ProbSy[MAX], ProbRx[MAX], ProbRy[MAX];
  
  ttree->Branch("EventN",    &Evn,    "EventN/I");
  ttree->Branch("EventT",    &Evt,    "EventT/I");
  ttree->Branch("Channel",   &ChN,    "Channel/I");
  ttree->Branch("NTracks",   &NTk,    "NTracks/I");
  ttree->Branch("NClusters", &NCl,    "NClusters/I");
  ttree->Branch("BestTrackI",&IBestTk,"BestTrackI/I");
  ttree->Branch("SlopeX",    Sx,      "SlopeX[NTracks]/F");
  ttree->Branch("SlopeY",    Sy,      "SlopeY[NTracks]/F");
  ttree->Branch("ResidualX", Rx,      "ResidualX[NTracks]/F");
  ttree->Branch("ResidualY", Ry,      "ResidualY[NTracks]/F");
  ttree->Branch("Prob",      Prob,    "Prob[NTracks]/F");
  ttree->Branch("ProbSlopeX", ProbSx, "ProbSlopeX[NTracks]/F");
  ttree->Branch("ProbSlopeY", ProbSy, "ProbSlopeY[NTracks]/F");
  ttree->Branch("ProbResidX", ProbRx, "ProbResidX[NTracks]/F");
  ttree->Branch("ProbResidY", ProbRy, "ProbResidX[NTracks]/F");
//parameters of fits!!!
 Float_t slope_frac_x     =  7.63007e-01;
 Float_t slope_mean_x     =  1.89624e-04;
 Float_t slope_width_x    =  3.68345e-03;
 Float_t slope_frac_y     =  8.87517e-01;
 Float_t slope_mean_y     =  2.70418e-02;
 Float_t slope_width_y    =  1.42262e-03;
 Float_t rsquared_frac_x  =  2.78366e-01;
 Float_t rsquared_mean_x  = -1.17443e-07;
 Float_t rsquared_width_x =  1.69606e-06;
 Float_t rsquared_frac_y  =  3.07276e-01;
 Float_t rsquared_mean_y  = -5.24714e-07;
 Float_t rsquared_width_y =  1.40776e-06;

  //start event looping
  for (int ientry1 = 0; Event1.GetNextEvent() >= 0; ++ientry1) {
    if (ientry1 % 10000 == 0) {
      std::cout << "Processing entry: " << ientry1 << std::endl;
    }
//    if (ientry1>=500000){break;}
    // Loop over all planes with hits in event
    NCl = 0;
    for (size_t it = 0; it != Event1.NTelescopes(); ++it) {
      // THIS telescope is
      PLTTelescope* Telescope = Event1.Telescope(it);
      if (Telescope->NTracks() >= 1 && Telescope->Plane(0)->NClusters() >= 1 && Telescope->Plane(1)->NClusters() >= 1 && Telescope->Plane(2)->NClusters() >= 1){
        Evn = Event1.EventNumber();
        Evt = Event1.Time();
        ChN = Telescope->Channel();
        //get tracks
        NCl = Telescope->NClusters();
        NTk = 0;
        for (uint ij = 0; ij != Telescope->NTracks(); ++ij) {
          if (Telescope->Track(ij)->NClusters()==3){
            PLTTrack* Track = Telescope->Track(ij);
            Sx[NTk] = (Track->Cluster(2)->TX()-Track->Cluster(0)->TX())/7.54;
            Sy[NTk] = (Track->Cluster(2)->TY()-Track->Cluster(0)->TY())/7.54;
            Float_t signx = 0; Float_t signy = 0;
            if (Track->LResidualX(0) < 0 ){signx = -1;}
            else {signx=1;}
            if (Track->LResidualY(0) < 0 ){signy = -1;}
            else {signy=1;}
            Rx[NTk] = signx*(pow(Track->LResidualX(0),2)+pow(Track->LResidualX(1),2)+pow(Track->LResidualX(2),2));
            Ry[NTk] = signy*(pow(Track->LResidualY(0),2)+pow(Track->LResidualY(1),2)+pow(Track->LResidualY(2),2));
            ProbSx[NTk] = TMath::Prob((pow((Sx[NTk]-slope_mean_x),2)/(pow(slope_width_x,2))),1);
            ProbSy[NTk] = TMath::Prob((pow((Sy[NTk]-slope_mean_y),2)/(pow(slope_width_y,2))),1);
            ProbRx[NTk] = TMath::Prob((pow((Rx[NTk]-rsquared_mean_x),2)/(pow(rsquared_width_x,2))),1);
            ProbRy[NTk] = TMath::Prob((pow((Ry[NTk]-rsquared_mean_y),2)/(pow(rsquared_width_y,2))),1);
            Prob[NTk] = ProbSx[NTk] * ProbSy[NTk] * ProbRx[NTk];
            ++NTk;
          }
        }
        if (NTk > 0){
          Float_t PBest = -1.0;
          IBestTk = -99;
          for (int ik = 0; ik != NTk; ++ik) {
            if (Prob[ik] > PBest) IBestTk = ik; PBest = Prob[ik];
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

  AccidentalMeasurement(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}

