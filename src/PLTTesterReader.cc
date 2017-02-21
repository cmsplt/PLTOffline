#include "PLTTesterReader.h"

PLTTesterReader::PLTTesterReader ()
{
  fOutDir = "./";
  fPlaneFiducialRegion = PLTPlane::kFiducialRegion_All;
}


PLTTesterReader::~PLTTesterReader ()
{
}



void PLTTesterReader::SetPlaneFiducialRegion (PLTPlane::FiducialRegion in)
{
  fPlaneFiducialRegion = in;
  return;
}


void PLTTesterReader::SetOutDir (TString const in)
{
  fOutDir = in;
  return;
}


void PLTTesterReader::SetOutRootFile (TFile* RootFile)
{
  fOutRoot = RootFile;
  SetupHistograms();
  return;
}



void PLTTesterReader::SetupHistograms ()
{
  // Some histograms to keep track of everything we read
  hROCUBLevels = new TH1I("ROCUBLevels", "ROC UB Levels", 150, 500, 2000);
  hTBMUBLevels = new TH1I("TBMUBLevels", "TBM UB Levels", 150, 500, 2000);
  hROCLevels   = new TH1I("ROCLevels", "ROC Levels", 210, 1800, 3500);
  hTBMLevels   = new TH1I("TBMLevels", "TBM Levels", 210, 1800, 3500);

  hROCUBLevels->SetDirectory(fOutRoot);
  hTBMUBLevels->SetDirectory(fOutRoot);
  hROCLevels->SetDirectory(fOutRoot);
  hTBMLevels->SetDirectory(fOutRoot);

  return;
}



void PLTTesterReader::MakePlots ()
{
  fOutRoot->cd();
  float const hHistMaxY = hROCLevels->GetMaximum();

  TLine lLine[6];
  for (int i = 0; i < 6; ++i) {
    lLine[i].SetLineColor(2);
    lLine[i].SetX1(LevelsROC[i]);  lLine[i].SetX2(LevelsROC[i]);
    lLine[i].SetY1(1);   lLine[i].SetY2(hHistMaxY);
  }



  TCanvas cROCLevels("LevelsROC", "LevelsROC");
  cROCLevels.cd()->SetLogy(1);
  hROCLevels->Draw("hist");
  hROCLevels->Write();
  for (int i = 0; i < 6; ++i) {
    lLine[i].Draw("same");
  }
  cROCLevels.SaveAs(fOutDir + "/LevelsROC.gif");
  cROCLevels.Write();

  TCanvas cROCUBLevels("LevelsROCUB", "LevelsROCUB");;
  cROCUBLevels.cd()->SetLogy(1);
  hROCUBLevels->Draw("hist");
  hROCUBLevels->Write();
  cROCUBLevels.SaveAs(fOutDir + "/LevelsROCUB.gif");
  cROCUBLevels.Write();

  return;
}



bool PLTTesterReader::OpenFile (std::string const InFileName)
{
  fFileName = InFileName;

  fFile.open(fFileName.c_str());
  if (fFile.is_open()) {
    return true;
  }

  return false;
}



int PLTTesterReader::ReadEventHits (std::vector<PLTHit*>& Hits, int& Time, int& VCal, int& VCalMult)
{

  std::string Line;
  while (std::getline(fFile, Line) && Line == "") {
    std::cout << "PLTTesterReader: Skipping blank line in data file" << std::endl;
  }

  if (fFile.eof()) {
    return -1;
  }

  int NUB = 0;
  int UBPosition[NMAXUB];
  std::istringstream SLine;
  SLine.str("");
  SLine.str(Line);
  for (int iadc = 0; iadc != 205; ++iadc) {
    SLine >> EventData[iadc];
    //std::cout << iadc << "  " <<  EventData[iadc] << "  ";
    if (EventData[iadc] < UBThreshold) {
      if (NUB < NMAXUB) {
        UBPosition[NUB++] = iadc;
      }

      if (NUB <= 3 || NUB >= 5) {
        hTBMUBLevels->Fill(EventData[iadc]);
      } else if (NUB == 4) {
        hROCUBLevels->Fill(EventData[iadc]);
      }
      //if (NUB <= 5) {
      //  //hUBPosition[NUB - 1]->Fill(iadc);
      //}
    }
  }


  if (NUB != NMAXUB) {
    //std::cerr << "WARNING: Incorrect number of UB in this event.  Skipping.  NUB = " << NUB << std::endl;
    return 0;
  }

  int const NHits = (UBPosition[4] - UBPosition[3] - 3) / 6;
  if (UBPosition[4] - UBPosition[3] <= 3) {
    //std::cout << "No Hits" << std::endl;
  }

  //std::cout << UBPosition[4] - UBPosition[3] << "  : " << NHits << std::endl;
  if ((UBPosition[4] - UBPosition[3] - 3) % 6 != 0) {
    //std::cerr << "ERROR: Incorrect position for an UB is messing this event up.  Skipping" << std::endl;
  }
  Time = EventData[200];
  VCal = EventData[202];
  VCalMult = EventData[201] == 0 ? 1 : 7;


  //hLastDAC->Fill(EventData[ UBPosition[3] + 2 ]); // LastDAC
  //hNHits->Fill(NHits);
  for (int ihit = 0; ihit != NHits; ++ihit) {
    std::pair<int, int> colrow = fill_pixel_info(EventData, UBPosition[3] + 2 + 0 + ihit * 6);
    //printf("Hit %4i  %2i %2i\n", ihit, colrow.first, colrow.second);
    PLTHit* Hit = new PLTHit(1, 0, colrow.first, colrow.second, EventData[ UBPosition[3] + 2 + 6 + ihit * 6 ]);
    if (PLTPlane::IsFiducial(fPlaneFiducialRegion, Hit)) {
      Hits.push_back(Hit);
    } else {
      delete Hit;
    }

    hROCLevels->Fill(EventData[ UBPosition[3] + 2 + 1 + ihit * 6 ]); // DCol High
    hROCLevels->Fill(EventData[ UBPosition[3] + 2 + 2 + ihit * 6 ]); // DCol Low
    hROCLevels->Fill(EventData[ UBPosition[3] + 2 + 3 + ihit * 6 ]); // Pixel High
    hROCLevels->Fill(EventData[ UBPosition[3] + 2 + 4 + ihit * 6 ]); // Pixel Mid
    hROCLevels->Fill(EventData[ UBPosition[3] + 2 + 5 + ihit * 6 ]); // Pixel Low
    //hPulseHeightADC->Fill(EventData[ UBPosition[3] + 2 + 6 + ihit * 6 ]); // Pulse Height
  }

  // Plot TBM Levels
  hTBMUBLevels->Fill(EventData[UBPosition[0]]);
  hTBMUBLevels->Fill(EventData[UBPosition[1]]);
  hTBMUBLevels->Fill(EventData[UBPosition[2]]);
  hTBMUBLevels->Fill(EventData[UBPosition[4]]);
  hTBMUBLevels->Fill(EventData[UBPosition[5]]);

  hTBMLevels->Fill(EventData[UBPosition[0] + 4]);
  hTBMLevels->Fill(EventData[UBPosition[0] + 5]);
  hTBMLevels->Fill(EventData[UBPosition[0] + 6]);
  hTBMLevels->Fill(EventData[UBPosition[0] + 7]);

  return Hits.size();

}



















int PLTTesterReader::CalculateLevels (std::string const& InFileName, int const NLines = 10000)
{
  // scan through tezt file NLines and try to calculate the levels

  std::cout << "PLTTesterReader: Calculating Levels from file: " << InFileName << "  Using at most this many events: " << NLines << std::endl;

  // open input file
  std::ifstream InFile(InFileName.c_str());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open input file: " << InFileName << std::endl;
    exit(1);
  }

  fOutRoot->cd();

  TH1I hCLROCUBLevels("CL_ROCUBLevels", "CL ROC UB Levels", 150, 500, 2000);
  TH1I hCLTBMUBLevels("CL_TBMUBLevels", "CL TBM UB Levels", 150, 500, 2000);
  TH1I hCLROCLevels("CL_ROCLevels", "CL ROC Levels", 310, 1400, 3500);
  TH1I hCLTBMLevels("CL_TBMLevels", "CL TBM Levels", 210, 1400, 3500);

  TH1I hUBPosition[5];
  for (int i = 0; i != 5; ++i) {
    TString const Name = TString::Format("UBPositionl%i", i);
    hUBPosition[i].SetName(Name);
    hUBPosition[i].SetTitle(Name);
    hUBPosition[i].SetBins(200, 0, 200);
    hUBPosition[i].SetLineColor(i + 1);
  }

  // Run through each line and look at the levels
  int iLine = 0;
  for (std::string Line; std::getline(InFile, Line) && iLine < 100000; ++iLine) {
    int NUB = 0;
    int UBPosition[NMAXUB];
    std::istringstream SLine;
    SLine.str(Line);
    for (int iadc = 0; iadc != 205; ++iadc) {
      SLine >> EventData[iadc];
      if (EventData[iadc] < UBThreshold) {
        UBPosition[NUB++] = iadc;

        if (NUB <= 3 || NUB >= 5) {
          hCLTBMUBLevels.Fill(EventData[iadc]);
        } else if (NUB == 4) {
          hCLROCUBLevels.Fill(EventData[iadc]);
        }
        if (NUB <= 5) {
          hUBPosition[NUB - 1].Fill(iadc);
        }
      }
      if (NUB == NMAXUB) {
        break;
      }
    }

    if (NUB != NMAXUB) {
      //std::cerr << "WARNING: Incorrect number of UB in this event.  Skipping.  NUB = " << NUB << std::endl;
      continue;
    }

    int const NHits = (UBPosition[4] - UBPosition[3] - 3) / 6;
    if (UBPosition[4] - UBPosition[3] <= 3) {
      //std::cout << "No Hits" << std::endl;
      continue;
    }

    //std::cout << UBPosition[4] - UBPosition[3] << "  : " << NHits << std::endl;
    if ((UBPosition[4] - UBPosition[3] - 3) % 6 != 0) {
      //std::cerr << "ERROR: Incorrect position for an UB is messing this event up.  Skipping" << std::endl;
      continue;
    }

    //hLastDAC.Fill(EventData[ UBPosition[3] + 2 ]); // LastDAC
    for (int ihit = 0; ihit != NHits; ++ihit) {
      hCLROCLevels.Fill(EventData[ UBPosition[3] + 2 + 1 + ihit * 6 ]); // DCol High
      hCLROCLevels.Fill(EventData[ UBPosition[3] + 2 + 2 + ihit * 6 ]); // DCol Low
      hCLROCLevels.Fill(EventData[ UBPosition[3] + 2 + 3 + ihit * 6 ]); // Pixel High
      hCLROCLevels.Fill(EventData[ UBPosition[3] + 2 + 4 + ihit * 6 ]); // Pixel Mid
      hCLROCLevels.Fill(EventData[ UBPosition[3] + 2 + 5 + ihit * 6 ]); // Pixel Low
      //hCLROCLevels.Fill(EventData[ UBPosition[3] + 2 + 6 + ihit * 6 ]); // Pulse Height
      //hCLPulseHeightADC.Fill(EventData[ UBPosition[3] + 2 + 6 + ihit * 6 ]); // Pulse Height
    }

    // Plot TBM Levels
    hCLTBMUBLevels.Fill(EventData[UBPosition[0]]);
    hCLTBMUBLevels.Fill(EventData[UBPosition[1]]);
    hCLTBMUBLevels.Fill(EventData[UBPosition[2]]);
    hCLTBMUBLevels.Fill(EventData[UBPosition[4]]);
    hCLTBMUBLevels.Fill(EventData[UBPosition[5]]);

    hCLTBMLevels.Fill(EventData[UBPosition[0] + 4]);
    hCLTBMLevels.Fill(EventData[UBPosition[0] + 5]);
    hCLTBMLevels.Fill(EventData[UBPosition[0] + 6]);
    hCLTBMLevels.Fill(EventData[UBPosition[0] + 7]);


  }



  TCanvas cCLTBMLevels;
  cCLTBMLevels.cd()->SetLogy(1);
  hCLTBMLevels.Draw("hist");
  cCLTBMLevels.SaveAs(fOutDir + "/CL_LevelsTBM.gif");
  hCLTBMLevels.Write();

  TCanvas cCLTBMUBLevels;
  cCLTBMUBLevels.cd()->SetLogy(1);
  hCLTBMUBLevels.Draw("hist");
  cCLTBMUBLevels.SaveAs(fOutDir + "/CL_LevelsTBMUB.gif");
  hCLTBMUBLevels.Write();

  TSpectrum Spectrum(20);
  Spectrum.SetAverageWindow(20);//probably does nothing
  int const NPeaks = Spectrum.Search(&hCLROCLevels);
  Float_t* Peaks = (Float_t *) Spectrum.GetPositionX();
  std::sort(Peaks, Peaks + NPeaks);

  //printf("Peak positions after sort\n");
  //printf(" %f %f %f %f %f %f\n", Peaks[0], Peaks[1], Peaks[2], Peaks[3], Peaks[4], Peaks[5]);

  TMarker* pPoint[NPeaks];
  for (int i = 0; i < NPeaks; ++i) {
    pPoint[i] = new TMarker();
    pPoint[i]->SetX(Peaks[i]);
    pPoint[i]->SetY(hCLROCLevels.GetBinContent(hCLROCLevels.FindBin(Peaks[i])));
    pPoint[i]->SetMarkerStyle(3);
  }

  float const hHistMaxY = hCLROCLevels.GetMaximum();

  TLine* lLine[NPeaks];
  for (int i = 0; i < NPeaks - 1; ++i) {
    lLine[i] = new TLine();
    float xp = Peaks[i];
    float yp = Peaks[i + 1];
    xp = xp + (yp - xp) / 2.0;

    printf(" Threshold %d value %f\n", i, xp);

    if (i <= 6) {
      LevelsROC[i] = (int)xp;
    }

    lLine[i]->SetLineColor(2);
    lLine[i]->SetX1(xp);  lLine[i]->SetX2(xp);
    lLine[i]->SetY1(1);   lLine[i]->SetY2(hHistMaxY);
  }




  TCanvas cCLROCLevels("CL_LevelsROC", "CL_LevelsROC");
  cCLROCLevels.cd()->SetLogy(1);
  hCLROCLevels.Draw("hist");
  hCLROCLevels.Write();
  for (int i = 0; i < NPeaks; ++i) {
    pPoint[i]->Draw("same");
    lLine[i]->Draw("same");
  }
  cCLROCLevels.SaveAs(fOutDir + "/CL_LevelsROC.gif");
  cCLROCLevels.Write();

  TCanvas cCLROCUBLevels("CL_LevelsROCUB", "CL_LevelsROCUB");;
  cCLROCUBLevels.cd()->SetLogy(1);
  hCLROCUBLevels.Draw("hist");
  hCLROCUBLevels.Write();
  cCLROCUBLevels.SaveAs(fOutDir + "/CL_LevelsROCUB.gif");
  cCLROCUBLevels.Write();

  if (NPeaks != 6) {
    std::cerr << "ERROR: NPeaks != 6.  NPeaks = "<< NPeaks << std::endl;
    exit(1);
  }

  return 1;
}





int PLTTesterReader::LevelInfo (int const Value)
{
  if (Value <= 0) {
    std::cout << "Something is wrong" << std::endl;
    return -1;
  }
  else if (Value > 0            && Value <= LevelsROC[0]) return 0;
  else if (Value > LevelsROC[0] && Value <= LevelsROC[1]) return 1;
  else if (Value > LevelsROC[1] && Value <= LevelsROC[2]) return 2;
  else if (Value > LevelsROC[2] && Value <= LevelsROC[3]) return 3;
  else if (Value > LevelsROC[3] && Value <= LevelsROC[4]) return 4;
  else if (Value > LevelsROC[4]) return 5;

  return -1;
}



std::pair<int, int> PLTTesterReader::fill_pixel_info(int* evt , int ctr)
{
  int finalcol = -1;
  int finalrow = -1;
  int c1 = evt[ctr + 1];
  int c0 = evt[ctr + 2];
  int r2 = evt[ctr + 3];
  int r1 = evt[ctr + 4];
  int r0 = evt[ctr + 5];

  int trancol=(LevelInfo(c1))*6 + (LevelInfo(c0));
  int tranrow=(LevelInfo(r2))*36 + (LevelInfo(r1))*6 + (LevelInfo(r0));
  if(tranrow%2 == 0)
  {
    finalrow=79 - (tranrow - 2)/2;
    finalcol=trancol * 2;
  }
  else
  {
    finalrow=79 - (tranrow - 3)/2;
    finalcol=trancol * 2 + 1;
  }



  return std::make_pair(finalcol, finalrow);
}



