////////////////////////////////////////////////////////////////////
//
//
// Working version Created by Krishna Thapa <kthapa@cern.ch>
// Sun June 19 2016
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

// All About them Tracks

int TrackParams (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName:    " << GainCalFileName << std::endl;

  PLTU::SetStyle();

  // Grab the plt event reader
  //****************************************************************************************
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "AlignmentFileName:    " << AlignmentFileName<< std::endl;


  gStyle->SetOptStat(1111);


  // save a ttree with track parameters
  TFile *f = new TFile("./TrackParams.root","RECREATE");
  TTree *ttree = new TTree("Tracks","Tracks");


  Int_t MAX = 20;
  Int_t Evn, Evt, NCl, NTk, IBestTk, ChN, nBX;
  Int_t trkID = 0;
  Float_t BsX[MAX], BsY[MAX], Sx[MAX], Sy[MAX], Rx[MAX], Ry[MAX], D[MAX];
  //  Float_t TxY[MAX],TxZ[MAX],TyX[MAX],TyZ[MAX];
  //  Float_t Roc2X[MAX],Roc2Y[MAX];


  ttree->Branch("TrackID",    &trkID,  "TrackID/I");
  ttree->Branch("EvntBX",    &nBX,    "EvntBX/I");
  ttree->Branch("EventN",    &Evn,    "EventN/I");
  ttree->Branch("EventT",    &Evt,    "EventT/I");
  ttree->Branch("Channel",   &ChN,    "Channel/I");

  ttree->Branch("NTracks",   &NTk,    "NTracks/I");
  ttree->Branch("NClusters", &NCl,    "NClusters/I");
  ttree->Branch("BestTrackI",&IBestTk,"BestTrackI/I");
  ttree->Branch("SlopeX",    Sx,      "SlopeX[NTracks]/F");
  ttree->Branch("SlopeY",    Sy,      "SlopeY[NTracks]/F");

  ttree->Branch("BeamspotX", BsX,     "BeamspotX[NTracks]/F");
  ttree->Branch("BeamspotY", BsY,     "BeamspotY[NTracks]/F");

  //  ttree->Branch("R2X",       Roc2X,   "R2X[NTracks]/F");
  //  ttree->Branch("R2Y",       Roc2Y,   "R2Y[NTracks]/F");

  // ttree->Branch("TrackxY",   TxY,     "TrackxY[NTracks]/F");
  // ttree->Branch("TrackxZ",   TxZ,     "TrackxZ[NTracks]/F");
  // ttree->Branch("TrackyX",   TyX,     "TrackyX[NTracks]/F");
  // ttree->Branch("TrackyZ",   TyZ,     "TrackyZ[NTracks]/F");

  ttree->Branch("ResidualX", Rx,      "ResidualX[NTracks]/F");
  ttree->Branch("ResidualY", Ry,      "ResidualY[NTracks]/F");
  ttree->Branch("Distance",  D,       "Distance[NTracks]/F");



  // to identify colliding and non-colliding BX
  TH1F *nTracks = new TH1F ("nTracks","nTracks",3564,-0.5,3563.5);

  unsigned int begin = 1000000; //start slink event from
  unsigned int end = 5000000;  //end slink event at

  std::set<int> snapshotCounter; // Tm, unique set
  int t0 = 0;
  int div = 60000; //1 minute snapshot, 60 * 1*10^3 milliseconds
                   //@3.5 kHz, this translates to ~ 140k tracks @ ~4 SBIL

  //start event looping
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    nBX = Event.BX();

    if (ientry == 0){
      t0 = Event.Time();
      //      std::cout << timestamp << std::endl;
    }

    if (ientry % 1000000 == 0) {
      std::cout << "Processing entry: " << ientry  << std::endl;
    }

    if (ientry >= end){break;}
    if (ientry < begin){continue;}



    int ID = Event.Time();
    int timeDiff = ID - t0;
    int tm = timeDiff / div;

    if (snapshotCounter.count(tm) == 0){
      snapshotCounter.insert(tm);
      trkID = tm;
    }

    if (ientry >= begin){
      // Loop over all planes with hits in event
      NCl = 0;
      for (size_t it = 0; it != Event.NTelescopes(); ++it) {
        // THIS telescope is
        PLTTelescope* Telescope = Event.Telescope(it);

        int phit = Telescope->HitPlaneBits();


        //      if (Telescope->NTracks() >= 1 && Telescope->Plane(0)->NClusters() >= 1 && Telescope->Plane(1)->NClusters() >= 1 && Telescope->Plane(2)->NClusters() >= 1){
        //if hit in all three planes for this telescope
        if(phit==0x7){

          Evn = Event.EventNumber();
          Evt = Event.Time();
          ChN = Telescope->Channel();

          //get tracks
          NCl = Telescope->NClusters();
          NTk = 0;

          for (uint ij = 0; ij != Telescope->NTracks(); ++ij) {
            if (Telescope->Track(ij)->NClusters()==3){
              PLTTrack* Track = Telescope->Track(ij);
              Sx[NTk] = (Track->Cluster(2)->TX()-Track->Cluster(0)->TX())/7.54;
              Sy[NTk] = (Track->Cluster(2)->TY()-Track->Cluster(0)->TY())/7.54;


              // //roc2 x,y position
              // Roc2X[NTk] = Track->Cluster(1)->TX();
              // Roc2Y[NTk] = Track->Cluster(1)->TY();


              //@Z=0
              BsX[NTk] = Track->fPlaner[2][0]; //x
              BsY[NTk] = Track->fPlaner[2][1]; //y


              // //@X=0
              // TxY[NTk] = Track->fPlaner[0][1]; //y
              // TxZ[NTk] = Track->fPlaner[0][2]; //z

              // //@Y=0
              // TyX[NTk] = Track->fPlaner[1][0]; //x
              // TyZ[NTk] = Track->fPlaner[1][2]; //z



              Float_t signx = 0; Float_t signy = 0;
              if (Track->LResidualX(0) < 0 ){signx = -1;}
              else {signx=1;}
              if (Track->LResidualY(0) < 0 ){signy = -1;}
              else {signy=1;}

              Rx[NTk] = signx*(pow(Track->LResidualX(0),2)+pow(Track->LResidualX(1),2)+pow(Track->LResidualX(2),2));
              Ry[NTk] = signy*(pow(Track->LResidualY(0),2)+pow(Track->LResidualY(1),2)+pow(Track->LResidualY(2),2));


              nTracks->Fill(nBX);

              D[NTk] = pow((Track->TX(Telescope->Plane(0)->TZ()) - Track->Cluster(0)->TX()),2)
                  + pow((Track->TY(Telescope->Plane(0)->TZ()) - Track->Cluster(0)->TY()),2)
                  + pow((Track->TX(Telescope->Plane(1)->TZ()) - Track->Cluster(1)->TX()),2)
                  + pow((Track->TY(Telescope->Plane(1)->TZ()) - Track->Cluster(1)->TY()),2)
                  + pow((Track->TX(Telescope->Plane(2)->TZ()) - Track->Cluster(2)->TX()),2)
                  + pow((Track->TY(Telescope->Plane(2)->TZ()) - Track->Cluster(2)->TY()),2);

              ++NTk;
            } // ncluster == 3 for this track

          }

          if (NTk > 0){
            Float_t DBest = 9999;
            IBestTk = -99;
            for (int ik = 0; ik != NTk; ++ik) {
              if (D[ik] < DBest) IBestTk = ik; DBest = D[ik];
            }
            ttree->Fill();

          }
        } //phit
      } // ntelescopes
    } // entry begin
  } //event

  nTracks->Write();
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

  TrackParams(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
