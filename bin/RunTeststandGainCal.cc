////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Oct  9 14:14:16 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cstdlib>
#include <algorithm>

#include "TString.h"
#include "TH1I.h"
#include "TH2I.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TMarker.h"
#include "TLine.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TFile.h"
#include "TF1.h"


int const UBThreshold = 1400;
int const NMAXUB = 6;
int EventData[205];

int LevelsROC[6];




int LevelInfo (int const Value)
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



std::pair<int, int> fill_pixel_info(int* evt , int ctr)
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




int RunTeststandGainCal (std::string const InFileName)
{
  // open input file
  std::ifstream InFile(InFileName.c_str());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open input file: " << InFileName << std::endl;
    exit(1);
  }

  TFile fOutFile("Test.root", "recreate");

  TH1I hROCUBLevels("ROCUBLevens", "ROCUBLevels", 150, 500, 2000);
  TH1I hTBMUBLevels("TBMUBLevels", "TBMUBLevels", 150, 500, 2000);
  TH1I hROCLevels("ROCLevels", "Levels", 310, 1400, 4500);
  TH1I hTBMLevels("TBMLevels", "Levels", 210, 1400, 3500);
  TH1I hPulseHeightADC("PulseHeightADC", "PulseHeightADC", 200, 1000, 4000);
  TH1I hNHits("NHits", "NHits", 20, 0, 20);
  TH1I hLastDAC("LastDAC", "LastDAC", 2000, 2000, 4000);


  std::map<int, float> GainAvg[120][120];
  std::map<int, int>   GainN[120][120];

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
          hTBMUBLevels.Fill(EventData[iadc]);
        } else if (NUB == 4) {
          hROCUBLevels.Fill(EventData[iadc]);
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

    hLastDAC.Fill(EventData[ UBPosition[3] + 2 ]); // LastDAC
    hNHits.Fill(NHits);
    for (int ihit = 0; ihit != NHits; ++ihit) {
      hROCLevels.Fill(EventData[ UBPosition[3] + 2 + 1 + ihit * 6 ]); // DCol High
      hROCLevels.Fill(EventData[ UBPosition[3] + 2 + 2 + ihit * 6 ]); // DCol Low
      hROCLevels.Fill(EventData[ UBPosition[3] + 2 + 3 + ihit * 6 ]); // Pixel High
      hROCLevels.Fill(EventData[ UBPosition[3] + 2 + 4 + ihit * 6 ]); // Pixel Mid
      hROCLevels.Fill(EventData[ UBPosition[3] + 2 + 5 + ihit * 6 ]); // Pixel Low
      //hROCLevels.Fill(EventData[ UBPosition[3] + 2 + 6 + ihit * 6 ]); // Pulse Height
      hPulseHeightADC.Fill(EventData[ UBPosition[3] + 2 + 6 + ihit * 6 ]); // Pulse Height
    }

    // Plot TBM Levels
    hTBMUBLevels.Fill(EventData[UBPosition[0]]);
    hTBMUBLevels.Fill(EventData[UBPosition[1]]);
    hTBMUBLevels.Fill(EventData[UBPosition[2]]);
    hTBMUBLevels.Fill(EventData[UBPosition[4]]);
    hTBMUBLevels.Fill(EventData[UBPosition[5]]);

    hTBMLevels.Fill(EventData[UBPosition[0] + 4]);
    hTBMLevels.Fill(EventData[UBPosition[0] + 5]);
    hTBMLevels.Fill(EventData[UBPosition[0] + 6]);
    hTBMLevels.Fill(EventData[UBPosition[0] + 7]);


  }



  TCanvas cTBMLevels;
  cTBMLevels.cd()->SetLogy(1);
  hTBMLevels.Draw("hist");
  cTBMLevels.SaveAs("LevelsTBM.eps");

  TCanvas cTBMUBLevels;
  cTBMUBLevels.cd()->SetLogy(1);
  hTBMUBLevels.Draw("hist");
  cTBMUBLevels.SaveAs("LevelsTBMUB.eps");

  TSpectrum Spectrum(20);
  Spectrum.SetAverageWindow(20);//probably does nothing
  int const NPeaks = Spectrum.Search(&hROCLevels);
  double* Peaks = Spectrum.GetPositionX();
  std::sort(Peaks, Peaks + NPeaks);//new, dangerous. 
  //print aft
  printf("Peak positions after sort\n");
  printf(" %f %f %f %f %f %f\n", Peaks[0], Peaks[1], Peaks[2], Peaks[3], Peaks[4], Peaks[5]);

  if (NPeaks != 6) {
    std::cerr << "ERROR: NPeaks != 6.  NPeaks = "<< NPeaks << std::endl;
    exit(1);
  }

  TMarker* pPoint[NPeaks];
  for (int i = 0; i < NPeaks; ++i) {
    pPoint[i] = new TMarker();
    pPoint[i]->SetX(Peaks[i]);
    pPoint[i]->SetY(hROCLevels.GetBinContent(hROCLevels.FindBin(Peaks[i])));
    pPoint[i]->SetMarkerStyle(3);
  }

  float const hHistMaxY = hROCLevels.GetMaximum();

  TLine* lLine[NPeaks];
  for (int i = 0; i < NPeaks; ++i) {
    lLine[i] = new TLine();
    float xp = Peaks[i];
    float yp = Peaks[i + 1];
    xp = xp + (yp - xp) / 2.0;
    printf(" Threshold %d value %f\n", i, xp);

    if (i <= 6) {
      LevelsROC[i] = xp;
    }

    lLine[i]->SetLineColor(2);
    lLine[i]->SetX1(xp);  lLine[i]->SetX2(xp);
    lLine[i]->SetY1(1);   lLine[i]->SetY2(hHistMaxY);
  }



  TCanvas cROCLevels;
  cROCLevels.cd()->SetLogy(1);
  hROCLevels.Draw("hist");
  for (int i = 0; i < NPeaks; ++i) {
    pPoint[i]->Draw("same");
    lLine[i]->Draw("same");
  }
  cROCLevels.SaveAs("LevelsROC.eps");

  TCanvas cROCUBLevels;
  cROCUBLevels.cd()->SetLogy(1);
  hROCUBLevels.Draw("hist");
  cROCUBLevels.SaveAs("LevelsROCUB.eps");

  TCanvas cUBPosition;
  cUBPosition.cd();
  hUBPosition[0].Draw();
  hUBPosition[1].Draw("same");
  hUBPosition[2].Draw("same");
  hUBPosition[3].Draw("same");
  hUBPosition[4].Draw("same");
  cUBPosition.SaveAs("UBPosition.eps");

  TCanvas cPulseHeightADC;
  cPulseHeightADC.cd();
  hPulseHeightADC.Draw();
  cPulseHeightADC.SaveAs("PulseHeightADC.eps");

  TCanvas cNHits;
  cNHits.cd();
  hNHits.Draw();
  cNHits.SaveAs("NHits.eps");

  TCanvas cLastDAC;
  cLastDAC.cd();
  hLastDAC.Draw();
  cLastDAC.SaveAs("LastDAC.eps");



  TH2I hOccupancy("Occupancy", "Occupancy", 26, 13, 39, 40, 40, 80);

  // Now check out the data
  InFile.clear();
  InFile.seekg(0, std::ios::beg);
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open input file: " << InFileName << std::endl;
    exit(1);
  }

  for (std::string Line; std::getline(InFile, Line); ) {
    int NUB = 0;
    int UBPosition[NMAXUB];
    std::istringstream SLine;
    SLine.str(Line);
    for (int iadc = 0; iadc != 205; ++iadc) {
      SLine >> EventData[iadc];
      if (EventData[iadc] < UBThreshold && NUB < NMAXUB) {
        UBPosition[NUB++] = iadc;

        if (NUB <= 5) {
          hUBPosition[NUB - 1].Fill(iadc);
        }
      }
    }

    //printf("%9i %9i %9i %9i %9i\n", EventData[200], EventData[201], EventData[202], EventData[203], EventData[204]);

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

    for (int ihit = 0; ihit != NHits; ++ihit) {
      std::pair<int, int> colrow = fill_pixel_info(EventData, UBPosition[3] + 2 + 0 + ihit * 6);
      //printf("Hit %4i  %2i %2i\n", ihit, colrow.first, colrow.second);
      int const VCalMult = EventData[201] == 0 ? 1 : 7;
      //printf("Time %5i   vcal %5i   adc %5i\n", EventData[200], EventData[202] * VCalMult, EventData[ UBPosition[3] + 2 + 6 + ihit * 6 ]);
      hOccupancy.Fill(colrow.first, colrow.second);

      GainAvg[colrow.first][colrow.second][EventData[202] * VCalMult] = GainAvg[colrow.first][colrow.second][EventData[202] * VCalMult] * ((double) GainN[colrow.first][colrow.second][EventData[202] * VCalMult] / ((double) GainN[colrow.first][colrow.second][EventData[202] * VCalMult] + 1.)) + EventData[ UBPosition[3] + 2 + 6 + ihit * 6 ] / ((double) GainN[colrow.first][colrow.second][EventData[202] * VCalMult] + 1.);
      ++GainN[colrow.first][colrow.second][EventData[202] * VCalMult];

    }

  }

  FILE* fGainCoefs = fopen("calibcofs.txt", "w");


  for (int i = 0; i != 120; ++i) {
    for (int j = 0; j != 120; ++j) {
      if (GainAvg[i][j].empty()) {
        continue;
      }
      printf("%3i %3i %9i\n", i, j, (int) GainAvg[i][j].size());

      TGraph g((int) GainAvg[i][j].size());
      g.SetName( TString::Format("Fit_%i_%i", i, j).Data() );
      float ADCMin = 99999;
      float ADCMax = 0;
      int p = 0;
      for (std::map<int, float>::iterator It = GainAvg[i][j].begin(); It != GainAvg[i][j].end(); ++It) {
        g.SetPoint(p++, It->second, It->first);
        if (It->second > ADCMax) {
          ADCMax = It->second;
        }
        if (It->second < ADCMin) {
          ADCMin = It->second;
        }
      }
      TString const FitName = TString::Format("f1_%i_%i", i, j);
      TF1 FitFunction(FitName, "[0]+[1]*x+[2]*x*x", 1000, 5000);
      FitFunction.SetParameter(0, 2.51747e+06);
      FitFunction.SetParameter(1, -1.80594e+03);
      FitFunction.SetParameter(2, 3.24677e-01);
      g.Fit(FitName, "Q", "", ADCMin, ADCMin + 0.7 * (ADCMax - ADCMin));
      g.Write();

      float const P0 = FitFunction.GetParameter(0);
      float const P1 = FitFunction.GetParameter(1);
      float const P2 = FitFunction.GetParameter(2);

      fprintf(fGainCoefs,"%d %d %f %f %f\n", i, j, P0, P1, P2);
    }
  }


  TCanvas cOccupancy;
  cOccupancy.cd();
  hOccupancy.Draw("colz");
  cOccupancy.SaveAs("Occupancy.eps");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  RunTeststandGainCal(argv[1]);

  return 0;
}
