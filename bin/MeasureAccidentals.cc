////////////////////////////////////////////////////////////////////
//
//  MeasureAccidentals -- a framework for measuring accidental
//  rates using the pixel data
//    Paul Lujan, October 2015
//    based on MakeTracks.cc from Grant/Keith
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <iomanip>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TFile.h"

int MeasureAccidentals(const std::string, const std::string, const std::string, const std::string, const std::string);

int MeasureAccidentals(const std::string DataFileName, const std::string GainCalFileName, const std::string AlignmentFileName,
		       const std::string TrackDistributionFileName, const std::string FillNo, const std::string TimestampFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;
  std::cout << "TrackDistributionFileName: " << TrackDistributionFileName << std::endl;
  std::cout << "Fill Number: " << FillNo << std::endl;
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
  FILE *qfile = fopen(TrackDistributionFileName.c_str(), "r");
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
    // std::cout << nSteps << " timestamps" << std::endl;
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
  int totCleanTracks = 0;
  int totCleanFails = 0;
  int nFailSteps[14] = {0};

  // Step-by-step slope plots
  TH1F *SlopeY_Step1 = new TH1F("SlopeY_Step1", "SlopeY_Step1", 40, -0.02, 0.06);
  SlopeY_Step1->SetXTitle("Track SlopeY, all scopes, step 1");
  TH1F *SlopeX_Step1 = new TH1F("SlopeX_Step1", "SlopeX_Step1", 40, -0.04, 0.04);
  SlopeX_Step1->SetXTitle("Track SlopeX, all scopes, step 1");
  TH1F *SlopeY_Step13 = new TH1F("SlopeY_Step13", "SlopeY_Step13", 40, -0.02, 0.06);
  SlopeY_Step13->SetXTitle("Track SlopeY, all scopes, step 13");
  TH1F *SlopeX_Step13 = new TH1F("SlopeX_Step13", "SlopeX_Step13", 40, -0.04, 0.04);
  SlopeX_Step13->SetXTitle("Track SlopeX, all scopes, step 13");

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
//   if (hr >= 25 && min >= 10){
//	std::cout << "Reached time of dropout" << std::endl;
// 	break;  
//     }
      std::cout << "Processing event: " << ientry << " at " << Event.Time() << " ("
		<< std::setfill('0') << std::setw(2) << (hr%24) << ":" << std::setw(2) << min
		<< ":" << std::setw(2) << sec << "." << std::setw(3) << Event.Time()%1000
		<< ")" << std::endl;
      //if (ientry > 0 && fabs(MapResidualX[202]->GetRMS() - 0.002881) < 0.000001 && fabs(MapResidualX[201]->GetRMS() - 0.004684) < 0.000001) break;
      FILE *f = fopen("abort.txt", "r");
      if (f != NULL) {
	fclose(f);
	break;
      }
    }
    //if (ientry>=200000){break;}
    
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
	if (foundOneGoodTrack) nGoodTriple[stepNumber]++;
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
	totCleanTracks++;
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
	bool trackIsGood = true;
	bool slopeXGood = true;
	bool slopeYGood = true;
	bool residXGood = true;
	bool residYGood = true;
	if (fabs((slopeY-MeanSlopeY[ch])/SigmaSlopeY[ch]) > 5.0) { trackIsGood = false; slopeYGood = false; nFailSteps[0]++; }
	if (fabs((slopeX-MeanSlopeX[ch])/SigmaSlopeX[ch]) > 5.0) { trackIsGood = false; slopeXGood = false; nFailSteps[1]++; }
	if (fabs((Track->LResidualY(0)-MeanResidualY[10*ch])/SigmaResidualY[10*ch]) > 5.0) { trackIsGood = false; residYGood = false; nFailSteps[2]++; }
	if (fabs((Track->LResidualY(1)-MeanResidualY[10*ch+1])/SigmaResidualY[10*ch+1]) > 5.0) { trackIsGood = false; residYGood = false; nFailSteps[3]++; }
	if (fabs((Track->LResidualY(2)-MeanResidualY[10*ch+2])/SigmaResidualY[10*ch+2]) > 5.0) { trackIsGood = false; residYGood = false; nFailSteps[4]++; }
	if (fabs((Track->LResidualX(0)-MeanResidualX[10*ch])/SigmaResidualX[10*ch]) > 5.0) { trackIsGood = false; residXGood = false; nFailSteps[5]++; }
	if (fabs((Track->LResidualX(1)-MeanResidualX[10*ch+1])/SigmaResidualX[10*ch+1]) > 5.0) { trackIsGood = false; residXGood = false; nFailSteps[6]++; }
	if (fabs((Track->LResidualX(2)-MeanResidualX[10*ch+2])/SigmaResidualX[10*ch+2]) > 5.0) { trackIsGood = false; residXGood = false; nFailSteps[7]++; }
	bool residsGood = (residXGood && residYGood);
	if (residsGood && slopeXGood && !slopeYGood) nFailSteps[8]++;
	if (residsGood && !slopeXGood && slopeYGood) nFailSteps[9]++;
	if (residsGood && !slopeXGood && !slopeYGood) nFailSteps[10]++;
	if (slopeXGood && slopeYGood && !residsGood) nFailSteps[11]++;
	if (slopeXGood && slopeYGood && !residXGood && residYGood) nFailSteps[12]++;
	if (!(slopeXGood && slopeYGood) && !residsGood) nFailSteps[13]++;
	if (trackIsGood)
	  nGoodTracks[stepNumber]++;
	else
	  totCleanFails++;
      }
    }
  }
  // properly catch the last step
  timestamps.push_back(std::make_pair(currentStepStart, Event.Time()));

//  const char *distbuff[50];
//  printf(distbuff, "TrackDistributions_%s.txt", FillNo);
  TFile *f = new TFile("histo_slopes_" + TString(FillNo)+ ".root","RECREATE");
  FILE *textf = fopen("TrackDistributions_" + TString(FillNo) + ".txt", "w");

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
  Can.SaveAs("plots/" + TString(FillNo) + "/BeamSpot.gif");
  
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
    Can.SaveAs("plots/" + FillNo + "/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH1F*>::iterator it = MapSlopeX.begin(); it != MapSlopeX.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + FillNo + "/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }

  for (std::map<int, TH2F*>::iterator it = MapSlope2D.begin(); it != MapSlope2D.end(); ++it) {
    it->second->Write();
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + FillNo + "/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = MapResidualY.begin(); it != MapResidualY.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + FillNo + "/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = MapResidualX.begin(); it != MapResidualX.end(); ++it) {
    it->second->Write();
    fprintf(textf, "%s %f %f\n", it->second->GetName(), it->second->GetMean(), it->second->GetRMS());
    TCanvas Can;
    Can.cd();
    it->second->Draw("hist");
    Can.SaveAs("plots/" + FillNo + "/" + TString(it->second->GetName()) + ".gif");
    delete it->second;
  }
  SlopeX_Step1->Write();
  SlopeX_Step13->Write();
  SlopeY_Step1->Write();
  SlopeY_Step13->Write();
  
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
  FILE *outf = fopen("AccidentalRates_" + TString(FillNo) + ".txt", "w");
  fprintf(outf, "%d\n", nSteps);
  for (int i=0; i<nSteps; ++i) {
    std::cout << "==step " << i << "==" << std::endl;
    std::cout << "Total in clean events: " << nTotTracks[i] << " and " << nGoodTracks[i] << " were good " << std::endl;
    std::cout << "Total: " << nAllTriple[i] << " events with potential tracks and " << nGoodTriple[i]
	      << " were good (" << (float)nGoodTriple[i]*100.0/nAllTriple[i] << ")" << std::endl;
    std::cout << "Total of " << nEvents[i] << " triggers (" << (float)nEvents[i]*1000.0/(timestamps[i].second-timestamps[i].first) << " Hz)" << std::endl;
    fprintf(outf, "%d %d %d %d %d\n", timestamps[i].first, timestamps[i].second, nEvents[i], nAllTriple[i], nGoodTriple[i]);
  }
  fclose(outf);
  std::cout << "Causes of failure for tracks that failed (note: categories are not mutually exclusive): " << std::endl;
  for (int i=0; i<14; ++i) {
    std::cout << nFailSteps[i] << " tracks (";
    if (totCleanTracks > 0)
      std::cout << (float)nFailSteps[i]*100.0/totCleanTracks;
    else
      std::cout << "n/a";
    std::cout << "% of total, ";
    if (totCleanFails > 0)
      std::cout << (float)nFailSteps[i]*100.0/totCleanFails;
    else
      std::cout << "n/a";
    std::cout << "% of failures) fail ";
    switch (i) {
    case 0: 
      std::cout << "slopeY cut"; break;
    case 1:
      std::cout << "slopeX cut"; break;
    case 2:
      std::cout << "residY0 cut"; break;
    case 3:
      std::cout << "residY1 cut"; break;
    case 4:
      std::cout << "residY2 cut"; break;
    case 5:
      std::cout << "residX0 cut"; break;
    case 6:
      std::cout << "residX1 cut"; break;
    case 7:
      std::cout << "residX2 cut"; break;
    case 8:
      std::cout << "slopeY cut but pass slopeX and residuals"; break;
    case 9:
      std::cout << "slopeX cut but pass slopeY and residuals"; break;
    case 10:
      std::cout << "both slope cuts but pass residuals"; break;
    case 11:
      std::cout << "residual cuts but pass slope cuts"; break;
    case 12:
      std::cout << "residual X specifically, passing slope and residual Y"; break;
    case 13:
      std::cout << "at least one slope and at least one residual cut"; break;
    }
    std::cout << std::endl;
  }
  
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc < 6 || argc > 7) {
    std::cerr << "Usage: " << argv[0] << " DataFile.dat GainCal.dat AlignmentFile.dat TrackDistributions.txt FillNumber [TimestampFile.dat]" << std::endl;
    return 1;
  }

  const std::string DataFileName = argv[1];
  const std::string GainCalFileName = argv[2];
  const std::string AlignmentFileName = argv[3];
  const std::string TrackDistributionFileName = argv[4];
  const std::string fNo = argv[5];
  const std::string TimestampFileName = (argc == 7) ? argv[6] : "";

  MeasureAccidentals(DataFileName, GainCalFileName, AlignmentFileName, TrackDistributionFileName, fNo, TimestampFileName);

  return 0;
}
