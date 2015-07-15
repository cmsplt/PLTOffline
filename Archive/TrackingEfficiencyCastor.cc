////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Jul  4 19:20:41 CEST 2011
//
////////////////////////////////////////////////////////////////////
//This is a modified version of TrackingEfficiency for the analysis
//of efficiencies at Castor and PS/PSI testbeams. The output hit_data.root
//is used by EfficiencyVsTime.

#include <iostream>
#include <string>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TFile.h"
#include "TTree.h"

TFile *fTracking;

int startEvent, endEvent;

bool doSlopeSelection=kFALSE;

class HitCounter
{
  public:
    HitCounter () {
      NFiducial[0] = 0;
      NFiducial[1] = 0;
      NFiducial[2] = 0;
      NFiducialAndHit[0] = 0;
      NFiducialAndHit[1] = 0;
      NFiducialAndHit[2] = 0;
    }
    ~HitCounter () {}

    int NFiducial[3];
    int NFiducialAndHit[3];
};



// FUNCTION DEFINITIONS HERE
int TrackingEfficiency (std::string const, std::string const, std::string const);







// CODE BELOW
TH1F* FidHistFrom2D (TH2F* hIN, TString const NewName, int const NBins, PLTPlane::FiducialRegion FidRegion)
{
  // This function returns a TH1F* and YOU are then the owner of
  // that memory.  please delete it when you are done!!!

  int const NBinsX = hIN->GetNbinsX();
  int const NBinsY = hIN->GetNbinsY();
  float const ZMin = 0;//hIN->GetMinimum();
  float const ZMax = hIN->GetMaximum() * (1.0 + 1.0 / (float) NBins);
  std::cout << hIN->GetMaximum() << "  " << ZMax << std::endl;
  int const MyNBins = NBins + 1;

  TString const hNAME = NewName == "" ? TString(hIN->GetName()) + "_1DZFid" : NewName;

  TH1F* h;
  h = new TH1F(hNAME, hNAME, MyNBins, ZMin, ZMax);
  h->SetXTitle("Number of Hits");
  h->SetYTitle("Number of Pixels");
  h->GetXaxis()->CenterTitle();
  h->GetYaxis()->CenterTitle();
  h->SetTitleOffset(1.4, "y");
  h->SetFillColor(40);

  for (int ix = 1; ix <= NBinsX; ++ix) {
    for (int iy = 1; iy <= NBinsY; ++iy) {
      int const px = hIN->GetXaxis()->GetBinLowEdge(ix);
      int const py = hIN->GetYaxis()->GetBinLowEdge(iy);
      if (PLTPlane::IsFiducial(FidRegion, px, py)) {
        if (hIN->GetBinContent(ix, iy) > ZMax) {
          h->Fill(ZMax - hIN->GetMaximum() / (float) NBins);
        } else {
          h->Fill( hIN->GetBinContent(ix, iy) );
        }
      }
    }
  }

  return h;
}

int SlopeSelection (int Channel, PLTTrack Track)
{
  //The selection window is set by looking at Slope{X,Y}_CH#.gif from MakeTracks.
  if(!doSlopeSelection) return 1;
  float slopeX = Track.fTVX/Track.fTVZ, slopeY = Track.fTVY/Track.fTVZ;
  switch (Channel)
    {
    case 13:
      if(slopeX>-0.014 && slopeX<-0.008 && slopeY>0.010 && slopeY<0.016) return 1;
      break;
    case 24:
      if(slopeX>0.014 && slopeX<0.022 && slopeY>-0.004 && slopeY<0.004) return 1;
      break;
    case 14:
      if(slopeX>-0.012 && slopeX<-0.004 && slopeY>-0.008 && slopeY<-0.004) return 1;
      break;
    case 16:
      if(slopeX>0.002 && slopeX<0.004 && slopeY>-0.022 && slopeY<-0.014) return 1;
      break;
    default:
      return 1;
    }
  return 0;
}

int TrackingEfficiency (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_m2_m2;//kFiducialRegion_Diamond;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_m2_m2;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Map for tracking efficiency
  std::map<int, HitCounter> HC;
  std::map<int, TH2F*> hEffMapN;
  std::map<int, TH2F*> hEffMapD;
  std::map<int, TH1F*> hEffMapSlopeXN;
  std::map<int, TH1F*> hEffMapSlopeXD;
  std::map<int, TH1F*> hEffMapSlopeYN;
  std::map<int, TH1F*> hEffMapSlopeYD;
  std::map<int, TH1F*> hEffMapPulseHeightN;
  std::map<int, TH1F*> hEffMapPulseHeightD;
  std::map<int, TH1F*> hMapPulseHeights;

  std::map<int, TH1F*> hMapLocalTranslationX;
  std::map<int, TH1F*> hMapLocalTranslationY;
  std::map<int, TH1F*> hMapGlobalTranslationX;
  std::map<int, TH1F*> hMapGlobalTranslationY;
  std::map<int, TH2F*> hMapLocalRotation;

  float const PixelDist = 3;

  //Create tree and branches
  int eventNo, eventTime, BXNo, roc1_trial_ch13, roc1_success_ch13, roc1_trial_ch14, roc1_success_ch14, roc1_trial_ch16, roc1_success_ch16, roc1_trial_ch24, roc1_success_ch24;
  int roc1_trial_ch2, roc1_success_ch2, roc1_trial_ch3, roc1_success_ch3, roc1_trial_ch7, roc1_success_ch7, roc1_trial_ch8, roc1_success_ch8;
  float slopeX_ch13, slopeX_ch14, slopeX_ch16, slopeX_ch24, slopeY_ch13, slopeY_ch14, slopeY_ch16, slopeY_ch24; 
  float slopeX_ch2, slopeX_ch3, slopeX_ch7, slopeX_ch8, slopeY_ch2, slopeY_ch3, slopeY_ch7, slopeY_ch8; 
  TTree *data = new TTree("data","data");
  data->Branch("EventNo",&eventNo,"eventNo/I");
  data->Branch("EventTime",&eventTime,"eventTime/I");
  data->Branch("BXNo",&BXNo,"BXNo/I");
  data->Branch("roc1_trial_ch13",&roc1_trial_ch13,"roc1_trial_ch13/I");
  data->Branch("roc1_trial_ch14",&roc1_trial_ch14,"roc1_trial_ch14/I");
  data->Branch("roc1_trial_ch16",&roc1_trial_ch16,"roc1_trial_ch16/I");
  data->Branch("roc1_trial_ch24",&roc1_trial_ch24,"roc1_trial_ch24/I");
  data->Branch("roc1_success_ch13",&roc1_success_ch13,"roc1_success_ch13/I");
  data->Branch("roc1_success_ch14",&roc1_success_ch14,"roc1_success_ch14/I");
  data->Branch("roc1_success_ch16",&roc1_success_ch16,"roc1_success_ch16/I");
  data->Branch("roc1_success_ch24",&roc1_success_ch24,"roc1_success_ch24/I");
  data->Branch("slopeX_ch13",&slopeX_ch13,"slopeX_ch13/F");
  data->Branch("slopeX_ch14",&slopeX_ch14,"slopeX_ch14/F");
  data->Branch("slopeX_ch16",&slopeX_ch16,"slopeX_ch16/F");
  data->Branch("slopeX_ch24",&slopeX_ch24,"slopeX_ch24/F");
  data->Branch("slopeY_ch13",&slopeY_ch13,"slopeY_ch13/F");
  data->Branch("slopeY_ch14",&slopeY_ch14,"slopeY_ch14/F");
  data->Branch("slopeY_ch16",&slopeY_ch16,"slopeY_ch16/F");
  data->Branch("slopeY_ch24",&slopeY_ch24,"slopeY_ch24/F");
  data->Branch("roc1_trial_ch2",&roc1_trial_ch2,"roc1_trial_ch2/I");
  data->Branch("roc1_trial_ch3",&roc1_trial_ch3,"roc1_trial_ch3/I");
  data->Branch("roc1_trial_ch7",&roc1_trial_ch7,"roc1_trial_ch7/I");
  data->Branch("roc1_trial_ch8",&roc1_trial_ch8,"roc1_trial_ch8/I");
  data->Branch("roc1_success_ch2",&roc1_success_ch2,"roc1_success_ch2/I");
  data->Branch("roc1_success_ch3",&roc1_success_ch3,"roc1_success_ch3/I");
  data->Branch("roc1_success_ch7",&roc1_success_ch7,"roc1_success_ch7/I");
  data->Branch("roc1_success_ch8",&roc1_success_ch8,"roc1_success_ch8/I");
  data->Branch("slopeX_ch2",&slopeX_ch2,"slopeX_ch2/F");
  data->Branch("slopeX_ch3",&slopeX_ch3,"slopeX_ch3/F");
  data->Branch("slopeX_ch7",&slopeX_ch7,"slopeX_ch7/F");
  data->Branch("slopeX_ch8",&slopeX_ch8,"slopeX_ch8/F");
  data->Branch("slopeY_ch2",&slopeY_ch2,"slopeY_ch2/F");
  data->Branch("slopeY_ch3",&slopeY_ch3,"slopeY_ch3/F");
  data->Branch("slopeY_ch7",&slopeY_ch7,"slopeY_ch7/F");
  data->Branch("slopeY_ch8",&slopeY_ch8,"slopeY_ch8/F");

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }
    if (startEvent >= 0 && endEvent > 0) {
      if (ientry < startEvent) continue;
      if (ientry > endEvent) break;
    }

    eventNo=eventTime=BXNo=roc1_trial_ch13=roc1_success_ch13=roc1_trial_ch14=roc1_success_ch14=roc1_trial_ch16=roc1_success_ch16=roc1_trial_ch24=roc1_success_ch24=0;
    slopeX_ch13=slopeX_ch14=slopeX_ch16=slopeX_ch24=slopeY_ch13=slopeY_ch14=slopeY_ch16=slopeY_ch24=-99;
    roc1_trial_ch2=roc1_success_ch2=roc1_trial_ch3=roc1_success_ch3=roc1_trial_ch7=roc1_success_ch7=roc1_trial_ch8=roc1_success_ch8=0;
    slopeX_ch2=slopeX_ch3=slopeX_ch7=slopeX_ch8=slopeY_ch2=slopeY_ch3=slopeY_ch7=slopeY_ch8=-99;

    eventNo=Event.EventNumber();
    eventTime=Event.Time();
    BXNo=Event.BX();
   
    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event.Telescope(it);

      int    const Channel = Telescope->Channel();
      size_t const NPlanes = Telescope->NPlanes();

      // make them clean events
      if (Telescope->NHitPlanes() < 2 || Telescope->NHitPlanes() != Telescope->NClusters()) {
        continue;
      }


      // Make some hists for this telescope
      if (!hEffMapN.count(Channel * 10 + 0)) {

        // Make a numerator and demonitor hist for every roc for this channel
        for (int iroc = 0; iroc != 3; ++iroc) {
          TString Name = TString::Format("EffNumerator_Ch%i_ROC%i", Channel, iroc);
          hEffMapN[Channel * 10 + iroc] = new TH2F(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
          Name = TString::Format("EffNumeratorSlopeX_Ch%i_ROC%i", Channel, iroc);
	  hEffMapSlopeXN[Channel * 10 + iroc] = new TH1F(Name, Name, 100, -0.1, 0.1);
          Name = TString::Format("EffNumeratorSlopeY_Ch%i_ROC%i", Channel, iroc);
          hEffMapSlopeYN[Channel * 10 + iroc] = new TH1F(Name, Name, 100, -0.1, 0.1);
          Name = TString::Format("EffNumeratorPulseHeight_Ch%i_ROC%i", Channel, iroc);
          hEffMapPulseHeightN[Channel * 10 + iroc] = new TH1F(Name, Name, 60, -1000, 50000);
          Name = TString::Format("EffDenominator%i_ROC%i", Channel, iroc);
          hEffMapD[Channel * 10 + iroc] = new TH2F(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
          Name = TString::Format("EffDenominatorSlopeX_Ch%i_ROC%i", Channel, iroc);
          hEffMapSlopeXD[Channel * 10 + iroc] = new TH1F(Name, Name, 100, -0.1, 0.1);
          Name = TString::Format("EffDenominatorSlopeY_Ch%i_ROC%i", Channel, iroc);
          hEffMapSlopeYD[Channel * 10 + iroc] = new TH1F(Name, Name, 100, -0.1, 0.1);
          Name = TString::Format("EffDenominatorPulseHeight_Ch%i_ROC%i", Channel, iroc);
          hEffMapPulseHeightD[Channel * 10 + iroc] = new TH1F(Name, Name, 60, -1000, 50000);
          Name = TString::Format("ExtrapolatedTrackPulseHeights_Ch%i_ROC%i", Channel, iroc);
          hMapPulseHeights[Channel * 10 + iroc] = new TH1F(Name, Name, 60, -1000, 50000);
	  if(iroc==1){
	    Name = TString::Format("LocalTranslationAlignment_Ch%i", Channel);
	    hMapLocalTranslationX[Channel * 10 + iroc] = new TH1F(Name, Name, 50, -0.3, 0.3);
	    Name = TString::Format("LocalTranslationAlignment_Ch%i", Channel);
	    hMapLocalTranslationY[Channel * 10 + iroc] = new TH1F(Name, Name, 50, -0.3, 0.3);
	    Name = TString::Format("GlobalTranslationAlignment_Ch%i", Channel);
	    hMapGlobalTranslationX[Channel * 10 + iroc] = new TH1F(Name, Name, 50, -0.3, 0.3);
	    Name = TString::Format("GlobalTranslationAlignment_Ch%i", Channel);
	    hMapGlobalTranslationY[Channel * 10 + iroc] = new TH1F(Name, Name, 50, -0.3, 0.3);
	    Name = TString::Format("LocalRotationAlignment_Ch%i", Channel);
	    hMapLocalRotation[Channel * 10 + iroc] = new TH2F(Name, Name, 50, -0.3, 0.3, 50, -0.3, 0.3);
	  }
        }
      }

      PLTPlane* Plane[3] = {0x0, 0x0, 0x0};
      for (size_t ip = 0; ip != NPlanes; ++ip) {
        Plane[ Telescope->Plane(ip)->ROC() ] = Telescope->Plane(ip);
      }

      // To construct 4 tracks.. one testing each plane.. and one using all planes if it be.
      PLTTrack Tracks[4];

      // If it has all 3 planes that'll be number 0
      if (Plane[0]->NClusters() && Plane[1]->NClusters() && Plane[2]->NClusters()) {
        Tracks[0].AddCluster(Plane[0]->Cluster(0));
        Tracks[0].AddCluster(Plane[1]->Cluster(0));
        Tracks[0].AddCluster(Plane[2]->Cluster(0));
        Tracks[0].MakeTrack(Alignment);
      }

      // 2-plane tracks
      if (Plane[0]->NClusters() && Plane[1]->NClusters()) {
        Tracks[1].AddCluster(Plane[0]->Cluster(0));
        Tracks[1].AddCluster(Plane[1]->Cluster(0));
        Tracks[1].MakeTrack(Alignment);
      }
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        Tracks[2].AddCluster(Plane[0]->Cluster(0));
        Tracks[2].AddCluster(Plane[2]->Cluster(0));
        Tracks[2].MakeTrack(Alignment);
      }
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        Tracks[3].AddCluster(Plane[1]->Cluster(0));
        Tracks[3].AddCluster(Plane[2]->Cluster(0));
        Tracks[3].MakeTrack(Alignment);
      }

      // Test of plane 2
      if (Plane[0]->NClusters() && Plane[1]->NClusters()) {
        if (Tracks[1].IsFiducial(Channel, 2, Alignment, FidRegionTrack) && Tracks[1].NHits() == 2) {
          if(SlopeSelection(Channel,Tracks[1])) ++HC[Channel].NFiducial[2];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 2);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[1].TX( CP->LZ ), Tracks[1].TY( CP->LZ ), Channel, 2);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          if(SlopeSelection(Channel,Tracks[1])) hEffMapD[Channel * 10 + 2]->Fill(PXY.first, PXY.second);
          hEffMapSlopeXD[Channel * 10 + 2]->Fill(Tracks[1].fTVX/Tracks[1].fTVZ);
          hEffMapSlopeYD[Channel * 10 + 2]->Fill(Tracks[1].fTVY/Tracks[1].fTVZ);
	  float cluster_charge = 0;
	  if(Plane[2]->NClusters()) cluster_charge=Plane[2]->Cluster(0)->Charge();
          hEffMapPulseHeightD[Channel * 10 + 2]->Fill(cluster_charge);
          if (Plane[2]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[1].LResiduals( *(Plane[2]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            //printf("ResXY: %15.9f   RPXY: %15.9f\n", ResXY.first, RPXY.first);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
              //hEffMapN[Channel * 10 + 2]->Fill(Plane[2]->Cluster(0)->SeedHit()->Column(), Plane[2]->Cluster(0)->SeedHit()->Row());
              if(SlopeSelection(Channel,Tracks[1])) hEffMapN[Channel * 10 + 2]->Fill(PXY.first, PXY.second);
	      hEffMapSlopeXN[Channel * 10 + 2]->Fill(Tracks[1].fTVX/Tracks[1].fTVZ);
	      hEffMapSlopeYN[Channel * 10 + 2]->Fill(Tracks[1].fTVY/Tracks[1].fTVZ);
	      hEffMapPulseHeightN[Channel * 10 + 2]->Fill(cluster_charge);
	      if(SlopeSelection(Channel,Tracks[1])) //!!!
		hMapPulseHeights[Channel * 10 + 2]->Fill(cluster_charge);
              if(SlopeSelection(Channel,Tracks[1])) ++HC[Channel].NFiducialAndHit[2];
            } else {
              static int ievent = 0;
              if (ievent < 20) {
                Telescope->DrawTracksAndHits( TString::Format("plots/Jeebus_%04i.gif", ievent++).Data());
              }
	      //!!!hMapPulseHeights[Channel * 10 + 2]->Fill(0);
            }
          }
        }
      }

      // Test of plane 1
      if (Plane[0]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[2].IsFiducial(Channel, 1, Alignment, FidRegionTrack) && Tracks[2].NHits() == 2) {
          if(SlopeSelection(Channel,Tracks[2])){
	    ++HC[Channel].NFiducial[1];
	    switch(Channel){
	    case 2: ++roc1_trial_ch2;
	      slopeX_ch2 = Tracks[2].fTVX/Tracks[2].fTVZ;
	      slopeY_ch2 = Tracks[2].fTVY/Tracks[2].fTVZ;
	      break;
	    case 3: ++roc1_trial_ch3;
	      slopeX_ch3 = Tracks[2].fTVX/Tracks[2].fTVZ;
	      slopeY_ch3 = Tracks[2].fTVY/Tracks[2].fTVZ;
	      break;
	    case 7: ++roc1_trial_ch7;
	      slopeX_ch7 = Tracks[2].fTVX/Tracks[2].fTVZ;
	      slopeY_ch7 = Tracks[2].fTVY/Tracks[2].fTVZ;
	      break;
	    case 8: ++roc1_trial_ch8;
	      slopeX_ch8 = Tracks[2].fTVX/Tracks[2].fTVZ;
	      slopeY_ch8 = Tracks[2].fTVY/Tracks[2].fTVZ;
	      break;
	    case 13: ++roc1_trial_ch13;
	      slopeX_ch13 = Tracks[2].fTVX/Tracks[2].fTVZ;
	      slopeY_ch13 = Tracks[2].fTVY/Tracks[2].fTVZ;
	      break;
	    case 14: ++roc1_trial_ch14;
	      slopeX_ch14 = Tracks[2].fTVX/Tracks[2].fTVZ;
	      slopeY_ch14 = Tracks[2].fTVY/Tracks[2].fTVZ;
	      break;
	    case 16: ++roc1_trial_ch16;
	      slopeX_ch16 = Tracks[2].fTVX/Tracks[2].fTVZ;
	      slopeY_ch16 = Tracks[2].fTVY/Tracks[2].fTVZ;
	      break;
	    case 24: ++roc1_trial_ch24;
	      slopeX_ch24 = Tracks[2].fTVX/Tracks[2].fTVZ;
	      slopeY_ch24 = Tracks[2].fTVY/Tracks[2].fTVZ;
	      break;
	    }
	  }
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 1);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[2].TX( CP->LZ ), Tracks[2].TY( CP->LZ ), Channel, 1);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          if(SlopeSelection(Channel,Tracks[2])) hEffMapD[Channel * 10 + 1]->Fill(PXY.first, PXY.second);
          hEffMapSlopeXD[Channel * 10 + 1]->Fill(Tracks[2].fTVX/Tracks[2].fTVZ);
          hEffMapSlopeYD[Channel * 10 + 1]->Fill(Tracks[2].fTVY/Tracks[2].fTVZ);
	  float cluster_charge = 0;
	  if(Plane[1]->NClusters()) cluster_charge=Plane[1]->Cluster(0)->Charge();
          hEffMapPulseHeightD[Channel * 10 + 1]->Fill(cluster_charge);
          if (Plane[1]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[2].LResiduals( *(Plane[1]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
              if(SlopeSelection(Channel,Tracks[2])) hEffMapN[Channel * 10 + 1]->Fill(PXY.first, PXY.second);
	      hEffMapSlopeXN[Channel * 10 + 1]->Fill(Tracks[2].fTVX/Tracks[2].fTVZ);
	      hEffMapSlopeYN[Channel * 10 + 1]->Fill(Tracks[2].fTVY/Tracks[2].fTVZ);
	      hEffMapPulseHeightN[Channel * 10 + 1]->Fill(cluster_charge);
	      if(SlopeSelection(Channel,Tracks[2])) //!!!
		hMapPulseHeights[Channel * 10 + 1]->Fill(cluster_charge);
              if(SlopeSelection(Channel,Tracks[2])){
		++HC[Channel].NFiducialAndHit[1];
		switch(Channel){
		case 2: ++roc1_success_ch2; break;
		case 3: ++roc1_success_ch3; break;
		case 7: ++roc1_success_ch7; break;
		case 8: ++roc1_success_ch8; break;
		case 13: ++roc1_success_ch13; break;
		case 14: ++roc1_success_ch14; break;
		case 16: ++roc1_success_ch16; break;
		case 24: ++roc1_success_ch24; break;
		}
	      }
            } //!!!else hMapPulseHeights[Channel * 10 + 1]->Fill(0);
          }
        }
      }

      // Test of plane 0
      if (Plane[1]->NClusters() && Plane[2]->NClusters()) {
        if (Tracks[3].IsFiducial(Channel, 0, Alignment, FidRegionTrack) && Tracks[3].NHits() == 2) {
          if(SlopeSelection(Channel,Tracks[3])) ++HC[Channel].NFiducial[0];
          PLTAlignment::CP* CP = Alignment.GetCP(Channel, 0);
          std::pair<float, float> LXY = Alignment.TtoLXY(Tracks[3].TX( CP->LZ ), Tracks[3].TY( CP->LZ ), Channel, 0);
          std::pair<int, int> PXY = Alignment.PXYfromLXY(LXY);
          if(SlopeSelection(Channel,Tracks[3])) hEffMapD[Channel * 10 + 0]->Fill(PXY.first, PXY.second);
          hEffMapSlopeXD[Channel * 10 + 0]->Fill(Tracks[3].fTVX/Tracks[3].fTVZ);
          hEffMapSlopeYD[Channel * 10 + 0]->Fill(Tracks[3].fTVY/Tracks[3].fTVZ);
	  float cluster_charge = 0;
	  if(Plane[0]->NClusters()) cluster_charge=Plane[0]->Cluster(0)->Charge();
          hEffMapPulseHeightD[Channel * 10 + 0]->Fill(cluster_charge);
          if (Plane[0]->NClusters() > 0) {
            std::pair<float, float> ResXY = Tracks[3].LResiduals( *(Plane[0]->Cluster(0)), Alignment );
            std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(ResXY);
            if (abs(RPXY.first) <= PixelDist && abs(RPXY.second) <= PixelDist) {
              if(SlopeSelection(Channel,Tracks[3])) hEffMapN[Channel * 10 + 0]->Fill(PXY.first, PXY.second);
	      hEffMapSlopeXN[Channel * 10 + 0]->Fill(Tracks[3].fTVX/Tracks[3].fTVZ);
	      hEffMapSlopeYN[Channel * 10 + 0]->Fill(Tracks[3].fTVY/Tracks[3].fTVZ);
	      hEffMapPulseHeightN[Channel * 10 + 0]->Fill(cluster_charge);
	      if(SlopeSelection(Channel,Tracks[3])) //!!!
		hMapPulseHeights[Channel * 10 + 0]->Fill(cluster_charge);
              if(SlopeSelection(Channel,Tracks[3])) ++HC[Channel].NFiducialAndHit[0];
            } //!!!else hMapPulseHeights[Channel * 10 + 0]->Fill(0);
          }
        }
      }

      //printf("%9i %9i %9i\n", HC[Channel].NFiducialAndHit[0], HC[Channel].NFiducialAndHit[1], HC[Channel].NFiducialAndHit[2]);


      //Now do alignment check.
      //First select good events: exactly 1 cluster per plane having exactly 1 pixel
      if (Telescope->NHitPlanes()==3 && Plane[0]->NClusters()==1 && Plane[1]->NClusters()==1 && Plane[2]->NClusters()==1 && Plane[0]->Cluster(0)->NHits()==1 && Plane[1]->Cluster(0)->NHits()==1 && Plane[2]->Cluster(0)->NHits()==1) {
	float plane0X  = Plane[0]->Cluster(0)->TX();
	float plane0Y  = Plane[0]->Cluster(0)->TY();
	float plane1X  = Plane[1]->Cluster(0)->TX();
	float plane1Y  = Plane[1]->Cluster(0)->TY();
	float plane2X  = Plane[2]->Cluster(0)->TX();
	float plane2Y  = Plane[2]->Cluster(0)->TY();
	float averageX = (plane0X+plane2X)/2.0;
	float averageY = (plane0Y+plane2Y)/2.0;
	hMapLocalTranslationX[Channel * 10 + 1]->Fill(plane1X-averageX);
	hMapLocalTranslationY[Channel * 10 + 1]->Fill(plane1Y-averageY);
	hMapGlobalTranslationX[Channel * 10 + 1]->Fill(plane0X-plane2X);
	hMapGlobalTranslationY[Channel * 10 + 1]->Fill(plane0Y-plane2Y);
	hMapLocalRotation[Channel * 10 + 1]->Fill(averageY,plane1X-averageX);
      }

    }
    data->Fill();
  }
  data->Write();

  // Save some efficiency maps
  for (std::map<int, TH2F*>::iterator it = hEffMapD.begin(); it != hEffMapD.end(); ++it) {
    int const id = it->first;
    int const Channel = id / 10;
    int const ROC     = id % 10;

    TString Name = TString::Format("TrackingEfficiencyMap_Ch%i_ROC%i", Channel, ROC);
    TCanvas Can1(Name, Name, 400, 1200);
    Can1.Divide(1, 3);

    Can1.cd(1);
    hEffMapN[id]->SetTitle(Name);
    hEffMapN[id]->Divide(it->second);
    hEffMapN[id]->SetMinimum(0.0);//0.3
    hEffMapN[id]->SetMaximum(1.0);//1.7
    hEffMapN[id]->SetStats(0);
    hEffMapN[id]->Draw("colz");

    Can1.cd(2)->SetLogy();
    TH1F* Hist1D = FidHistFrom2D(hEffMapN[id], "", 30, FidRegionTrack);
    Hist1D->SetTitle( TString::Format("Tracking Efficiency Ch%i ROC%i", Channel, ROC));
    Hist1D->SetXTitle("Efficiency");
    Hist1D->SetYTitle("# of Pixels");
    Hist1D->SetMinimum(0.5);
    Hist1D->Clone()->Draw();

    Can1.cd(3);
    Hist1D->SetMinimum(0);
    Hist1D->Draw();

    Name = TString::Format("plots/TrackingEfficiencyMap_Ch%i_ROC%i", Channel, ROC);
    Can1.SaveAs(Name + ".gif");

    Name = TString::Format("TrackingEfficiencySlopes_Ch%i_ROC%i", Channel, ROC);
    TCanvas Can2(Name, Name, 600, 1200);
    Can2.Divide(1, 3);

    Can2.cd(1);
    Name = TString::Format("Tracking Efficiency vs. Slope X for Ch%i ROC%i", Channel, ROC);
    hEffMapSlopeXN[id]->SetTitle(Name);
    hEffMapSlopeXN[id]->Divide(hEffMapSlopeXD[id]);
    hEffMapSlopeXN[id]->GetYaxis()->SetTitle("Tracking Efficiency");
    hEffMapSlopeXN[id]->GetXaxis()->SetTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
    hEffMapSlopeXN[id]->SetStats(0);
    hEffMapSlopeXN[id]->Draw();
    hEffMapSlopeXN[id]->Write();

    Can2.cd(2);
    Name = TString::Format("Tracking Efficiency vs. Slope Y for Ch%i ROC%i", Channel, ROC);
    hEffMapSlopeYN[id]->SetTitle(Name);
    hEffMapSlopeYN[id]->Divide(hEffMapSlopeYD[id]);
    hEffMapSlopeYN[id]->GetYaxis()->SetTitle("Tracking Efficiency");
    hEffMapSlopeYN[id]->GetXaxis()->SetTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
    hEffMapSlopeYN[id]->SetStats(0);
    hEffMapSlopeYN[id]->Draw();
    hEffMapSlopeYN[id]->Write();

    Can2.cd(3);
    Name = TString::Format("Tracking Efficiency vs. Pulse Height for Ch%i ROC%i", Channel, ROC);
    hEffMapPulseHeightN[id]->SetTitle(Name);
    hEffMapPulseHeightN[id]->Divide(hEffMapPulseHeightD[id]);
    hEffMapPulseHeightN[id]->GetYaxis()->SetTitle("Tracking Efficiency");
    hEffMapPulseHeightN[id]->GetXaxis()->SetTitle("Electrons");
    hEffMapPulseHeightN[id]->SetStats(0);
    hEffMapPulseHeightN[id]->Draw();

    Name = TString::Format("plots/TrackingEfficiencySlopes_Ch%i_ROC%i", Channel, ROC);
    Can2.SaveAs(Name + ".gif");

    Name = TString::Format("ExtrapolatedTrackPulseHeights_Ch%i_ROC%i", Channel, ROC);
    TCanvas Can3(Name, Name, 400, 400);

    Can3.cd(1);
    Name = TString::Format("Extrapolated Track Pulse Heights for Ch%i ROC%i", Channel, ROC);
    hMapPulseHeights[id]->SetTitle(Name);
    //hEffMapSlopeXN[id]->GetYaxis()->SetTitle("Events per");
    hMapPulseHeights[id]->GetXaxis()->SetTitle("Electrons");
    hMapPulseHeights[id]->SetFillColor(40);
    gStyle->SetOptStat(10);
    hMapPulseHeights[id]->Draw();
    
    Name = TString::Format("plots/ExtrapolatedTrackPulseHeights_Ch%i_ROC%i", Channel, ROC);
    Can3.SaveAs(Name + ".gif");

  }


  for (std::map<int, HitCounter>::iterator it = HC.begin(); it != HC.end(); ++it) {
    printf("Efficiencies for Channel %2i:\n", it->first);
    for (int i = 0; i != 3; ++i) {
      printf("ROC %1i  NFiducial: %10i  NFiducialAndHit: %10i  Efficiency: %12.9f\n",
             i, it->second.NFiducial[i],
             it->second.NFiducialAndHit[i],
             float(it->second.NFiducialAndHit[i]) / float(it->second.NFiducial[i]) );
    }
  }

  TCanvas *Can4 = new TCanvas("Local Telescope Translation X","Local Telescope Translation X",800,800);
  Can4->Divide(2,2);
  TCanvas *Can5 = new TCanvas("Local Telescope Translation Y","Local Telescope Translation Y",800,800);
  Can5->Divide(2,2);
  TCanvas *Can6 = new TCanvas("Global Telescope Translation X","Global Telescope Translation X",800,800);
  Can6->Divide(2,2);
  TCanvas *Can7 = new TCanvas("Global Telescope Translation Y","Global Telescope Translation Y",800,800);
  Can7->Divide(2,2);
  TCanvas *Can8 = new TCanvas("Local Telescope Rotation","Local Telescope Rotation",800,800);
  Can8->Divide(2,2);
  gStyle->SetOptStat(1110);
  int counter=1;
  for (std::map<int, TH1F*>::iterator it = hMapLocalTranslationX.begin(); it != hMapLocalTranslationX.end(); ++it) {
    int const id = it->first;
    int const Channel = id / 10;
    Can4->cd(counter);
    TString Name = TString::Format("Local Telescope Alignment in X for Ch%i", Channel);
    hMapLocalTranslationX[id]->SetTitle(Name);
    hMapLocalTranslationX[id]->GetXaxis()->SetTitle("x_{plane1}-x_{average} (cm)");
    hMapLocalTranslationX[id]->Draw();
    Can5->cd(counter);
    Name = TString::Format("Local Telescope Alignment in Y for Ch%i", Channel);
    hMapLocalTranslationY[id]->SetTitle(Name);
    hMapLocalTranslationY[id]->GetXaxis()->SetTitle("y_{plane1}-y_{average} (cm)");
    hMapLocalTranslationY[id]->Draw();
    Can6->cd(counter);
    Name = TString::Format("Global Telescope Alignment in X for Ch%i", Channel);
    hMapGlobalTranslationX[id]->SetTitle(Name);
    hMapGlobalTranslationX[id]->GetXaxis()->SetTitle("x_{plane0}-x_{plane2} (cm)");
    hMapGlobalTranslationX[id]->Draw();
    Can7->cd(counter);
    Name = TString::Format("Global Telescope Alignment in Y for Ch%i", Channel);
    hMapGlobalTranslationY[id]->SetTitle(Name);
    hMapGlobalTranslationY[id]->GetXaxis()->SetTitle("y_{plane0}-y_{plane2} (cm)");
    hMapGlobalTranslationY[id]->Draw();
    Can8->cd(counter);
    Name = TString::Format("Local Telescope Rotation for Ch%i", Channel);
    hMapLocalRotation[id]->SetTitle(Name);
    hMapLocalRotation[id]->GetXaxis()->SetTitle("y_{average} (cm)");
    hMapLocalRotation[id]->GetXaxis()->SetTitleOffset(1.3);
    hMapLocalRotation[id]->GetYaxis()->SetTitle("x_{plane1}-x_{average} (cm)");
    hMapLocalRotation[id]->Draw("colz");
    std::cout<<"The correlation for Channel "<<Channel<<" is "<<hMapLocalRotation[id]->GetCorrelationFactor()<<std::endl;
    counter++;
  }
  Can4->SaveAs("plots/LocalTranslationX.gif");
  Can5->SaveAs("plots/LocalTranslationY.gif");
  Can6->SaveAs("plots/GlobalTranslationX.gif");
  Can7->SaveAs("plots/GlobalTranslationY.gif");
  Can8->SaveAs("plots/LocalRotation.gif");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4 && argc != 6) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];

  if (argc == 6){
    startEvent = atoi(argv[4]);
    endEvent = atoi(argv[5]);
  }
  else {
    startEvent = -1;
    endEvent = -1;
  }

  fTracking = new TFile("hit_data.root","RECREATE");
  TrackingEfficiency(DataFileName, GainCalFileName, AlignmentFileName);
  fTracking->Close();

  return 0;
}
