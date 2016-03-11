////////////////////////////////////////////////////////////////////
//
//  MeasureAccidentalsTele -- a framework for measuring accidental
//  rates using the pixel data
//   Joseph Heideman, March 2016
//    Paul Lujan, October 2015
//        based on MakeTracks.cc from Grant/Keith
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <iomanip>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TFile.h"

int MeasureAccidentalsTele(const std::string, const std::string, const std::string, const std::string);

int MeasureAccidentalsTele(const std::string DataFileName, const std::string GainCalFileName, const std::string AlignmentFileName, const std::string TimestampFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;
  if (TimestampFileName != "")
    std::cout << "TimestampFileName: " << TimestampFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();
  gStyle->SetOptStat(1111);

  // read the track quality vars
  std::map<int, float> MeanSlopeY;
  std::map<int, float> SigmaSlopeY;
  std::map<int, float> MeanSlopeX;
  std::map<int, float> SigmaSlopeX;
  std::map<int, float> MeanResidualY;
  std::map<int, float> SigmaResidualY;
  std::map<int, float> MeanResidualX;
  std::map<int, float> SigmaResidualX;

  bool useTrackQuality = true;
  FILE *qfile = fopen("TrackQuality-v2.txt", "r");
  if (qfile == NULL) {
    std::cout << "Track quality file not found; accidental fraction will not be measured" << std::endl;
    useTrackQuality = false;
  } else {
    int nch, ch, roc;
    float mean, sigma;
    fscanf(qfile, "%d\n", &nch);
    for (int i=0; i<nch; ++i) {
      fscanf(qfile, "SlopeY_Ch%d %f %f\n", &ch, &mean, &sigma);
      MeanSlopeY[ch] = mean;
      SigmaSlopeY[ch] = sigma;
    }
    for (int i=0; i<nch; ++i) {
      fscanf(qfile, "SlopeX_Ch%d %f %f\n", &ch, &mean, &sigma);
      MeanSlopeX[ch] = mean;
      SigmaSlopeX[ch] = sigma;
    }
    for (int i=0; i<nch*3; ++i) {
      fscanf(qfile, "ResidualY%d_ROC%d %f %f\n", &ch, &roc, &mean, &sigma);
      MeanResidualY[10*ch+roc] = mean;
      SigmaResidualY[10*ch+roc] = sigma;
    }
    for (int i=0; i<nch*3; ++i) {
      fscanf(qfile, "ResidualX%d_ROC%d %f %f\n", &ch, &roc, &mean, &sigma);
      MeanResidualX[10*ch+roc] = mean;
      SigmaResidualX[10*ch+roc] = sigma;
    }
    
//     for (std::map<int, float>::iterator it = MeanSlopeY.begin(); it != MeanSlopeY.end(); ++it) {
//     ch = it->first;
//     std::cout << ch << " sX " << MeanSlopeX[ch] << "+/-" << SigmaSlopeX[ch]
// 	      << " sY " << MeanSlopeY[ch] << "+/-" << SigmaSlopeY[ch]
// 	      << " rX0 " << MeanResidualX[10*ch] << "+/-" << SigmaResidualX[10*ch]
// 	      << " rX1 " << MeanResidualX[10*ch+1] << "+/-" << SigmaResidualX[10*ch+1]
// 	      << " rX2 " << MeanResidualX[10*ch+2] << "+/-" << SigmaResidualX[10*ch+2]
// 	      << " rY0 " << MeanResidualY[10*ch] << "+/-" << SigmaResidualY[10*ch]
// 	      << " rY1 " << MeanResidualY[10*ch+1] << "+/-" << SigmaResidualY[10*ch+1]
// 	      << " rY2 " << MeanResidualY[10*ch+2] << "+/-" << SigmaResidualY[10*ch+2]
// 	      << std::endl;
//     }
//    return(0);

    fclose(qfile);
  } // read track quality file

  // Read timestamp file, if it exists.
  bool useTimestamps = false;
  int nSteps = 1;
  std::vector<std::pair<uint32_t, uint32_t> > timestamps;
  if (TimestampFileName != "") {
    FILE *tfile;
    tfile = fopen(TimestampFileName.c_str(), "r");
    if (tfile == NULL) {
      std::cerr << "Couldn't open timestamp file " << TimestampFileName << "!" << std::endl;
      return(1);
    }
    int tBegin, tEnd;
    fscanf(tfile, "%d", &nSteps);
    for (int i=0; i<nSteps; ++i) {
      fscanf(tfile, "%d %d", &tBegin, &tEnd);
      timestamps.push_back(std::make_pair(tBegin, tEnd));
    }
    fclose(tfile);
    useTimestamps = true;
    std::cout << nSteps << " timestamps" << std::endl;
    // for (unsigned int i=0; i<timestamps.size(); ++i) {
    //   std::cout << "start: " << timestamps[i].first << " end: " << timestamps[i].second << std::endl;
    // }
  }

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_All;
  //PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, FidRegionHits);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_01to2_AllCombs);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  std::map<int, int> NTrkEvMap;

  TH2F* HistBeamSpot[4];
  HistBeamSpot[0] = new TH2F("BeamSpotX", "BeamSpot X=0;Y;Z;NTracks", 25, -25, 25, 25, -540, 540);
  HistBeamSpot[1] = new TH2F("BeamSpotY", "BeamSpot Y=0;X;Z;NTracks", 25, -25, 25, 25, -540, 540);
  HistBeamSpot[2] = new TH2F("BeamSpotZ", "BeamSpot Z=0;X;Y;NTracks", 30, -10, 10, 15, -10, 10);
  HistBeamSpot[3] = new TH2F("BeamSpotZzoom", "BeamSpotZoom Z=0;X;Y;NTracks", 30, -5, 5, 30, -5, 5);

  std::map<int, TH1F*> MapSlopeY;
  std::map<int, TH1F*> MapSlopeX;
  std::map<int, TH2F*> MapSlope2D;
  std::map<int, TH1F*> MapResidualY;
  std::map<int, TH1F*> MapResidualX;

  std::vector<int> nTotTracks(nSteps);
  std::vector<int> nGoodTracks(nSteps);
  std::vector<int> nAllTriple(nSteps);
  std::vector<int> nGoodTriple(nSteps);
  std::vector<int> nEvents(nSteps);
// Telescope vectors for triple coincidence
  std::vector<int> nAllTele1(nSteps);
  std::vector<int> nAllTele2(nSteps);
  std::vector<int> nAllTele4(nSteps);
  std::vector<int> nAllTele5(nSteps);
  std::vector<int> nAllTele7(nSteps);
  std::vector<int> nAllTele8(nSteps);
  std::vector<int> nAllTele10(nSteps);
  std::vector<int> nAllTele11(nSteps);
  std::vector<int> nAllTele13(nSteps);
  std::vector<int> nAllTele14(nSteps);
  std::vector<int> nAllTele16(nSteps);
  std::vector<int> nAllTele17(nSteps);
  std::vector<int> nAllTele19(nSteps);
  std::vector<int> nAllTele20(nSteps);
//  std::vector<int> nAllTele22(nSteps);
//  std::vector<int> nAllTele23(nSteps);

//Telescope vectors for good tracks
  std::vector<int> nGoodTele1(nSteps);
  std::vector<int> nGoodTele2(nSteps);
  std::vector<int> nGoodTele4(nSteps);
  std::vector<int> nGoodTele5(nSteps);
  std::vector<int> nGoodTele7(nSteps);
  std::vector<int> nGoodTele8(nSteps);
  std::vector<int> nGoodTele10(nSteps);
  std::vector<int> nGoodTele11(nSteps);
  std::vector<int> nGoodTele13(nSteps);
  std::vector<int> nGoodTele14(nSteps);
  std::vector<int> nGoodTele16(nSteps);
  std::vector<int> nGoodTele17(nSteps);
  std::vector<int> nGoodTele19(nSteps);
  std::vector<int> nGoodTele20(nSteps);
//  std::vector<int> nGoodTele22(nSteps);
//  std::vector<int> nGoodTele23(nSteps);  
  


  // Step-by-step slope plots
  TH1F *SlopeY_Step1 = new TH1F("SlopeY_Step1", "SlopeY_Step1", 40, -0.02, 0.06);
  SlopeY_Step1->SetXTitle("Track SlopeY, all scopes, step 0");
  TH1F *SlopeX_Step1 = new TH1F("SlopeX_Step1", "SlopeX_Step1", 40, -0.04, 0.04);
  SlopeX_Step1->SetXTitle("Track SlopeX, all scopes, step 0");
  TH1F *SlopeY_Step13 = new TH1F("SlopeY_Step13", "SlopeY_Step13", 40, -0.02, 0.06);
  SlopeY_Step13->SetXTitle("Track SlopeY, all scopes, step 12");
  TH1F *SlopeX_Step13 = new TH1F("SlopeX_Step13", "SlopeX_Step13", 40, -0.04, 0.04);
  SlopeX_Step13->SetXTitle("Track SlopeX, all scopes, step 12");

  int stepNumber = 0;
  uint32_t currentStepStart = 0;
  // const uint32_t stepLength = 60000; // 1 minute
  const uint32_t stepLength = 300000; // 5 minutes
  
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 100000 == 0) {
      int nsec = Event.Time()/1000;
      int hr = nsec/3600;
      int min = (nsec-(hr*3600))/60;
      int sec = nsec % 60;
      std::cout << "Processing event: " << ientry << " at " << Event.Time() << " ("
		<< std::setfill('0') << std::setw(2) << (hr%24) << ":" << std::setw(2) << min
		<< ":" << std::setw(2) << sec << "." << std::setw(3) << Event.Time()%1000
		<< ")" << std::endl;
    }
    //if (ientry>=20000000){break;}

    if (useTimestamps) {
      stepNumber = -1;
      if (Event.Time() > timestamps[nSteps-1].second) break;
      for (int iStep = 0; iStep < nSteps; ++iStep) {
	if (Event.Time() >= timestamps[iStep].first && Event.Time() <= timestamps[iStep].second) {
	  stepNumber = iStep;
	  break;
	}
      }
      if (stepNumber == -1) continue;
    } else {
      if (stepNumber == 0 && currentStepStart == 0) currentStepStart = Event.Time();
      if ((Event.Time() - currentStepStart) > stepLength) {
	//std::cout << "starting new step @" << Event.Time() << " from " << currentStepStart << std::endl;
	timestamps.push_back(std::make_pair(currentStepStart, Event.Time()-1));
	nTotTracks.push_back(0);
	nGoodTracks.push_back(0);
	nAllTriple.push_back(0);
	nGoodTriple.push_back(0);
	nEvents.push_back(0);
        nAllTele1.push_back(0); 
        nAllTele2.push_back(0); 
        nAllTele4.push_back(0); 
        nAllTele5.push_back(0); 
        nAllTele7.push_back(0); 
        nAllTele8.push_back(0); 
        nAllTele10.push_back(0);  
        nAllTele11.push_back(0);  
        nAllTele13.push_back(0); 
        nAllTele14.push_back(0); 
        nAllTele16.push_back(0); 
        nAllTele17.push_back(0); 
        nAllTele19.push_back(0); 
        nAllTele20.push_back(0); 
//        nAllTele22.push_back(0); 
//        nAllTele23.push_back(0);
 
        nGoodTele1.push_back(0);  
        nGoodTele2.push_back(0);
        nGoodTele4.push_back(0);
        nGoodTele5.push_back(0);
        nGoodTele7.push_back(0);
        nGoodTele8.push_back(0);
        nGoodTele10.push_back(0);
        nGoodTele11.push_back(0);
        nGoodTele13.push_back(0);
        nGoodTele14.push_back(0);
        nGoodTele16.push_back(0);
        nGoodTele17.push_back(0);
        nGoodTele19.push_back(0);
        nGoodTele20.push_back(0);
//        nGoodTele22.push_back(0);
//        nGoodTele23.push_back(0);

	currentStepStart = Event.Time();
	stepNumber++;
	nSteps++;
      }
    }
    nEvents[stepNumber]++;

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      
      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);
//
//
//possibly where to identify what telescope the track is in
//
//
      if (!MapSlopeY[Telescope->Channel()]) {
        TString Name = TString::Format("SlopeY_Ch%i", Telescope->Channel());
        MapSlopeY[Telescope->Channel()] = new TH1F(Name, Name, 40, -0.02, 0.06);
        MapSlopeY[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
        Name = TString::Format("SlopeX_Ch%i", Telescope->Channel());
        MapSlopeX[Telescope->Channel()] = new TH1F(Name, Name, 40, -0.04, 0.04);
        MapSlopeX[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
        Name = TString::Format("Slope2D_Ch%i", Telescope->Channel());
        MapSlope2D[Telescope->Channel()] = new TH2F(Name, Name, 100, -0.1, 0.1, 100, -0.1, 0.1);
        MapSlope2D[Telescope->Channel()]->SetXTitle("Local Telescope Track-SlopeX #DeltaX/#DeltaZ");
        MapSlope2D[Telescope->Channel()]->SetYTitle("Local Telescope Track-SlopeY #DeltaY/#DeltaZ");
        Name = TString::Format("ResidualY%i_ROC0", Telescope->Channel());
        MapResidualY[Telescope->Channel()*10+0] = new TH1F(Name, Name, 40, -0.02, 0.06);
        MapResidualY[Telescope->Channel()*10+0]->SetXTitle("Local Telescope Residual-Y (cm)");
        Name = TString::Format("ResidualX%i_ROC0", Telescope->Channel());
        MapResidualX[Telescope->Channel()*10+0] = new TH1F(Name, Name, 40, -0.04, 0.04);
        MapResidualX[Telescope->Channel()*10+0]->SetXTitle("Local Telescope Residual-X (cm)");
        Name = TString::Format("ResidualY%i_ROC1", Telescope->Channel());
        MapResidualY[Telescope->Channel()*10+1] = new TH1F(Name, Name, 40, -0.02, 0.06);
        MapResidualY[Telescope->Channel()*10+1]->SetXTitle("Local Telescope Residual-Y (cm)");
        Name = TString::Format("ResidualX%i_ROC1", Telescope->Channel());
        MapResidualX[Telescope->Channel()*10+1] = new TH1F(Name, Name, 40, -0.04, 0.04);
        MapResidualX[Telescope->Channel()*10+1]->SetXTitle("Local Telescope Residual-X (cm)");
        Name = TString::Format("ResidualY%i_ROC2", Telescope->Channel());
        MapResidualY[Telescope->Channel()*10+2] = new TH1F(Name, Name, 40, -0.02, 0.06);
        MapResidualY[Telescope->Channel()*10+2]->SetXTitle("Local Telescope Residual-Y (cm)");
        Name = TString::Format("ResidualX%i_ROC2", Telescope->Channel());
        MapResidualX[Telescope->Channel()*10+2] = new TH1F(Name, Name, 40, -0.04, 0.04);
        MapResidualX[Telescope->Channel()*10+2]->SetXTitle("Local Telescope Residual-X (cm)");
      }

      if (Telescope->NTracks() > 0) {
	nAllTriple[stepNumber]++;
//put # of overall tracks in telescope here 
    int chnum = Telescope->Channel();
        if (chnum == 1) nAllTele1[stepNumber]++;
        if (chnum == 2) nAllTele2[stepNumber]++;
        if (chnum == 4) nAllTele4[stepNumber]++;
        if (chnum == 5) nAllTele5[stepNumber]++;
        if (chnum == 7) nAllTele7[stepNumber]++;
        if (chnum == 8) nAllTele8[stepNumber]++;
        if (chnum == 10) nAllTele10[stepNumber]++;
        if (chnum == 11) nAllTele11[stepNumber]++;
        if (chnum == 13) nAllTele13[stepNumber]++;
        if (chnum == 14) nAllTele14[stepNumber]++;
        if (chnum == 16) nAllTele16[stepNumber]++;
        if (chnum == 17) nAllTele17[stepNumber]++;
        if (chnum == 19) nAllTele19[stepNumber]++;
        if (chnum == 20) nAllTele20[stepNumber]++;
//        if (chnum == 22) nAllTele22[stepNumber]++;
//        if (chnum == 23) nAllTele23[stepNumber]++;

	bool foundOneGoodTrack = false;
	for (size_t itrack = 0; itrack < Telescope->NTracks(); ++itrack) {
	  PLTTrack *tr = Telescope->Track(itrack);
	  // apply track quality cuts
	  int ch = Telescope->Channel();
	  float slopeY = tr->fTVY/tr->fTVZ;
	  float slopeX = tr->fTVX/tr->fTVZ;
	  if (fabs((slopeY-MeanSlopeY[ch])/SigmaSlopeY[ch]) > 5.0) continue;
	  if (fabs((slopeX-MeanSlopeX[ch])/SigmaSlopeX[ch]) > 5.0) continue;
	  if (fabs((tr->LResidualY(0)-MeanResidualY[10*ch])/SigmaResidualY[10*ch]) > 5.0) continue;
	  if (fabs((tr->LResidualY(1)-MeanResidualY[10*ch+1])/SigmaResidualY[10*ch+1]) > 5.0) continue;
	  if (fabs((tr->LResidualY(2)-MeanResidualY[10*ch+2])/SigmaResidualY[10*ch+2]) > 5.0) continue;
	  if (fabs((tr->LResidualX(0)-MeanResidualX[10*ch])/SigmaResidualX[10*ch]) > 5.0) continue;
	  if (fabs((tr->LResidualX(1)-MeanResidualX[10*ch+1])/SigmaResidualX[10*ch+1]) > 5.0) continue;
	  if (fabs((tr->LResidualX(2)-MeanResidualX[10*ch+2])/SigmaResidualX[10*ch+2]) > 5.0) continue;
	  foundOneGoodTrack = true;
	  break;
	}
	if (foundOneGoodTrack){
		 nGoodTriple[stepNumber]++;  
          int ch =  Telescope->Channel();
	if (ch == 1) nGoodTele1[stepNumber]++;
        	if (ch == 2) nGoodTele2[stepNumber]++;
	        if (ch == 4) nGoodTele4[stepNumber]++;
	        if (ch == 5) nGoodTele5[stepNumber]++;
	        if (ch == 7) nGoodTele7[stepNumber]++;
	        if (ch == 8) nGoodTele8[stepNumber]++;
	        if (ch == 10) nGoodTele10[stepNumber]++;
	        if (ch == 11) nGoodTele11[stepNumber]++;
	        if (ch == 13) nGoodTele13[stepNumber]++;
	        if (ch == 14) nGoodTele14[stepNumber]++;
        	if (ch == 16) nGoodTele16[stepNumber]++;
	        if (ch == 17) nGoodTele17[stepNumber]++;
	        if (ch == 19) nGoodTele19[stepNumber]++;
        	if (ch == 20) nGoodTele20[stepNumber]++;
//	        if (ch == 22) nGoodTele22[stepNumber]++;
//        	if (ch == 23) nGoodTele23[stepNumber]++;     
      }
      
     }

      if (Telescope->NClusters() > 3) continue;

      if (Telescope->NTracks() > 0 && NTrkEvMap[Telescope->Channel()]++ < 10) {
        //Telescope->DrawTracksAndHits( TString::Format("plots/Tracks_Ch%i_Ev%i.gif", Telescope->Channel(), NTrkEvMap[Telescope->Channel()]).Data() );
      }

      for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack) {
        PLTTrack* Track = Telescope->Track(itrack);

        HistBeamSpot[0]->Fill( Track->fPlaner[0][1], Track->fPlaner[0][2]);
        HistBeamSpot[1]->Fill( Track->fPlaner[1][0], Track->fPlaner[1][2]);
        HistBeamSpot[2]->Fill( Track->fPlaner[2][0], Track->fPlaner[2][1]);
        HistBeamSpot[3]->Fill( Track->fPlaner[2][0], Track->fPlaner[2][1]);
        //std::cout<<Track->fPlaner[2][1]<<std::endl;
        MapSlopeY[Telescope->Channel()]->Fill(Track->fTVY/Track->fTVZ);
        MapSlopeX[Telescope->Channel()]->Fill(Track->fTVX/Track->fTVZ);
        MapSlope2D[Telescope->Channel()]->Fill(Track->fTVX/Track->fTVZ, Track->fTVY/Track->fTVZ);
        MapResidualY[Telescope->Channel()*10+0]->Fill(Track->LResidualY(0));
        MapResidualX[Telescope->Channel()*10+0]->Fill(Track->LResidualX(0));
        MapResidualY[Telescope->Channel()*10+1]->Fill(Track->LResidualY(1));
        MapResidualX[Telescope->Channel()*10+1]->Fill(Track->LResidualX(1));
        MapResidualY[Telescope->Channel()*10+2]->Fill(Track->LResidualY(2));
        MapResidualX[Telescope->Channel()*10+2]->Fill(Track->LResidualX(2));

	nTotTracks[stepNumber]++;
	float slopeY = Track->fTVY/Track->fTVZ;
	float slopeX = Track->fTVX/Track->fTVZ;
	if (stepNumber == 1) {
	  SlopeY_Step1->Fill(slopeY);
	  SlopeX_Step1->Fill(slopeX);
	}
	if (stepNumber == 13) {
	  SlopeY_Step13->Fill(slopeY);
	  SlopeX_Step13->Fill(slopeX);
	}

	if (!useTrackQuality) continue;
	int ch = Telescope->Channel();
	if (fabs((slopeY-MeanSlopeY[ch])/SigmaSlopeY[ch]) > 5.0) continue;
	if (fabs((slopeX-MeanSlopeX[ch])/SigmaSlopeX[ch]) > 5.0) continue;
	if (fabs((Track->LResidualY(0)-MeanResidualY[10*ch])/SigmaResidualY[10*ch]) > 5.0) continue;
	if (fabs((Track->LResidualY(1)-MeanResidualY[10*ch+1])/SigmaResidualY[10*ch+1]) > 5.0) continue;
	if (fabs((Track->LResidualY(2)-MeanResidualY[10*ch+2])/SigmaResidualY[10*ch+2]) > 5.0) continue;
	if (fabs((Track->LResidualX(0)-MeanResidualX[10*ch])/SigmaResidualX[10*ch]) > 5.0) continue;
	if (fabs((Track->LResidualX(1)-MeanResidualX[10*ch+1])/SigmaResidualX[10*ch+1]) > 5.0) continue;
	if (fabs((Track->LResidualX(2)-MeanResidualX[10*ch+2])/SigmaResidualX[10*ch+2]) > 5.0) continue;
	nGoodTracks[stepNumber]++;
//put # of good tracks per telescopehere

      }
    }
  }
  // properly catch the last step
  timestamps.push_back(std::make_pair(currentStepStart, Event.Time()));


  TFile *f = new TFile("histo_slopes.root","RECREATE");
  FILE *textf = fopen("TrackQuality.txt", "w");

  TCanvas Can("BeamSpot", "BeamSpot", 900, 900);
  Can.Divide(3, 3);
  Can.cd(1);
  HistBeamSpot[0]->SetXTitle("X(cm)");
  HistBeamSpot[0]->SetYTitle("Y(cm)");
  HistBeamSpot[0]->Draw("colz");
  Can.cd(2);
  HistBeamSpot[1]->SetXTitle("X(cm)");
  HistBeamSpot[1]->SetYTitle("Y(cm)");
  HistBeamSpot[1]->Draw("colz");
  Can.cd(3);
  HistBeamSpot[2]->SetXTitle("X(cm)");
  HistBeamSpot[2]->SetYTitle("Y(cm)");
  HistBeamSpot[2]->Draw("colz");

  HistBeamSpot[3]->SetXTitle("X (cm)");
  HistBeamSpot[3]->SetYTitle("Y (cm)");
  HistBeamSpot[3]->Draw("colz");

  Can.cd(1+3);
  HistBeamSpot[0]->ProjectionX()->Draw("ep");
  Can.cd(2+3);
  HistBeamSpot[1]->ProjectionX()->Draw("ep");
  Can.cd(3+3);
  HistBeamSpot[2]->ProjectionX()->Draw("ep");

  Can.cd(1+6);
  HistBeamSpot[0]->ProjectionY()->Draw("ep");
  Can.cd(2+6);
  HistBeamSpot[1]->ProjectionY()->Draw("ep");
  Can.cd(3+6);
  HistBeamSpot[2]->ProjectionY()->Draw("ep");
  Can.SaveAs("plots/BeamSpot.gif");
  
  HistBeamSpot[0]->Write();
  HistBeamSpot[1]->Write();
  HistBeamSpot[2]->Write();
  HistBeamSpot[3]->Write();

  fprintf(textf, "%d\n", (int)MapSlopeY.size());

  for (std::map<int, TH1F*>::iterator it = MapSlopeY.begin(); it != MapSlopeY.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH1F*>::iterator it = MapSlopeX.begin(); it != MapSlopeX.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH2F*>::iterator it = MapSlope2D.begin(); it != MapSlope2D.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = MapResidualY.begin(); it != MapResidualY.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = MapResidualX.begin(); it != MapResidualX.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  if (0) {
    TCanvas Can;
    Can.cd();
    SlopeY_Step1->Draw("hist");
    Can.SaveAs("SlopeY_Step1.png");
    SlopeX_Step1->Draw("hist");
    Can.SaveAs("SlopeX_Step1.png");
    SlopeY_Step13->Draw("hist");
    Can.SaveAs("SlopeY_Step13.png");
    SlopeX_Step13->Draw("hist");
    Can.SaveAs("SlopeX_Step13.png");
  }

  f->Close();
  fclose(textf);
  FILE *outf = fopen("AccidentalRatesTele.txt", "w");
  fprintf(outf, "%d\n", nSteps);
  for (int i=0; i<nSteps; ++i) {
    std::cout << "==step " << i << "==" << std::endl;
    std::cout << "Total: " << nTotTracks[i] << " and " << nGoodTracks[i] << " were good " << std::endl;
    std::cout << "Total: " << nAllTriple[i] << " events with potential tracks and " << nGoodTriple[i]
	      << " were good (" << (float)nGoodTriple[i]*100.0/nAllTriple[i] << ")" << std::endl;
    std::cout << "Total of " << nEvents[i] << " triggers (" << (float)nEvents[i]*1000.0/(timestamps[i].second-timestamps[i].first) << " Hz)" << std::endl;
    fprintf(outf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", timestamps[i].first, timestamps[i].second, nEvents[i], nAllTriple[i], nGoodTriple[i], nAllTele1[i], nGoodTele1[i], nAllTele2[i], nGoodTele2[i], nAllTele4[i], nGoodTele4[i], nAllTele5[i], nGoodTele5[i], nAllTele7[i], nGoodTele7[i], nAllTele8[i], nGoodTele8[i], nAllTele10[i], nGoodTele10[i], nAllTele11[i], nGoodTele11[i], nAllTele13[i], nGoodTele13[i], nAllTele14[i], nGoodTele14[i], nAllTele16[i], nGoodTele16[i], nAllTele17[i], nGoodTele17[i], nAllTele19[i], nGoodTele19[i], nAllTele20[i], nGoodTele20[i]);
// nAllTele22[i], nGoodTele22[i], nAllTele23[i], nGoodTele23[i]);
//printing of step info -> add columns telescope info here 
 }
  fclose(outf);
  
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc < 4 || argc > 5) {
    std::cerr << "Usage: " << argv[0] << " DataFile.dat GainCal.dat AlignmentFile.dat [TimestampFile.dat]" << std::endl;
    return 1;
  }

  const std::string DataFileName = argv[1];
  const std::string GainCalFileName = argv[2];
  const std::string AlignmentFileName = argv[3];
  const std::string TimestampFileName = (argc == 5) ? argv[4] : "";

  MeasureAccidentalsTele(DataFileName, GainCalFileName, AlignmentFileName, TimestampFileName);

  return 0;
}
