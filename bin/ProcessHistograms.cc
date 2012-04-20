////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Apr 10 19:48:45 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdint.h>
#include <algorithm>

#include "TH1F.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TApplication.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TROOT.h"
//#include "TGMainFrame.h"



int const NBUCKETS = 3564;
int const NORBITS  = 4096;
int const NMAXCHANNELS = 6;

int ChColor[6]  = { 20, 30, 0, 40, 46, 9 };

int ProcessHistograms (std::string const InFileName, int const FirstBucket, int const LastBucket)
{

  std::ifstream InFile(InFileName.c_str(), std::ios::in | std::ios::binary);
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    exit(1);
  }

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);

  int const NOrbitsToAvg = 9*5;

  uint32_t OrbitTime;
  uint32_t Orbit;
  uint32_t Channel;
  uint32_t BigBuff[NBUCKETS];

  uint32_t FirstOrbitTime;
  uint32_t LastOrbitTime;

  bool GotIt;
  bool SeenFirstEOF = false;

  uint32_t OTime[NMAXCHANNELS][NOrbitsToAvg];
  uint32_t O[NMAXCHANNELS][NOrbitsToAvg];
  uint32_t Counts[NMAXCHANNELS][NOrbitsToAvg][NBUCKETS];
  std::vector<int> Ch;

  bool IsChInOrbit[NMAXCHANNELS][NOrbitsToAvg];
  for (int i = 0; i != NMAXCHANNELS; ++i) {
    for (int j = 0; j != NOrbitsToAvg; ++j) {
      IsChInOrbit[i][j] = false;
    }
  }

  int const MyNBuckets  = LastBucket - FirstBucket;

  TH1F*    HistByChannel[NMAXCHANNELS];
  TCanvas* CanByChannel[NMAXCHANNELS];
  for (int i = 0; i != NMAXCHANNELS; ++i) {
    TString const Name = TString::Format("HistByChannel_Ch%i", i);
    HistByChannel[i] = new TH1F(Name, Name, MyNBuckets, FirstBucket, LastBucket);
    HistByChannel[i]->SetLineColor( ChColor[i] );
  }
  for (int i = 0; i != 1; ++i) {
    TString const NameC = TString::Format("HistByChannel_Ch%i", i);
    CanByChannel[i] = new TCanvas(NameC, NameC, 1800, 1100);
    CanByChannel[i]->Divide(1, NMAXCHANNELS);
  }



  TGraph GraphTotal;
  GraphTotal.SetMarkerStyle(6);
  TGraph GraphCh[NMAXCHANNELS];
  for (int ich = 0; ich < NMAXCHANNELS; ++ich) {
    GraphCh[ich].SetMarkerStyle(1);
    GraphCh[ich].SetMarkerColor( ChColor[ich] );
  }
  int    NGraphPoints = 0;

  while (true) {

    uint64_t TotalSum = 0;
    for (int iOrbit = 0; iOrbit < NOrbitsToAvg; ++iOrbit) {


      GotIt = false;
      while (!GotIt) {
        InFile.read( (char*) &OrbitTime, sizeof(uint32_t));
        if (InFile.eof()) {
          usleep(200000);
          InFile.clear();
          SeenFirstEOF = true;
        } else {
          GotIt = true;
        }
      }

      if (iOrbit == 0) {
        FirstOrbitTime = OrbitTime;
      } else if (iOrbit == NOrbitsToAvg - 1) {
        LastOrbitTime = OrbitTime;
      }

      GotIt = false;
      while (!GotIt) {
        InFile.read( (char*) &Orbit, sizeof(uint32_t));
        if (InFile.eof()) {
          usleep(200000);
          InFile.clear();
          SeenFirstEOF = true;
        } else {
          GotIt = true;
        }
      }

      GotIt = false;
      while (!GotIt) {
        InFile.read( (char*) &Channel, sizeof(uint32_t));
        if (InFile.eof()) {
          usleep(200000);
          InFile.clear();
          SeenFirstEOF = true;
        } else {
          GotIt = true;
        }
      }


      int iBucket = 0;
      while (iBucket < NBUCKETS) {
        GotIt = false;
        while (!GotIt) {
          InFile.read( (char*) &BigBuff[iBucket], sizeof(uint32_t));
          if (InFile.eof()) {
            GotIt = false;
            usleep(200000);
            InFile.clear();
            SeenFirstEOF = true;
          } else {
            GotIt = true;
            ++iBucket;
          }
        }
      }

      if (Orbit < 25000) {
        --iOrbit;
        continue;
      }

      if (!SeenFirstEOF) {
        //continue;
      }

      IsChInOrbit[Channel][iOrbit] = true;
      bool IsInVec = false;
      for (size_t imy = 0; imy != Ch.size(); ++imy) {
        if (Channel == Ch[imy]) {
          IsInVec = true;
        }
      }
      if (!IsInVec) {
        Ch.push_back(Channel);
      }

      for (int ib = 0; ib < NBUCKETS; ++ib) {
        Counts[Channel][iOrbit][ib] = (BigBuff[ib] & 0xfff);
        TotalSum += (BigBuff[ib] & 0xfff) / 5.0;
      }

      OTime[Channel][iOrbit] = OrbitTime;
      O[Channel][iOrbit] = Orbit;

    }



    int ThisPoint = NGraphPoints++;
    GraphTotal.Set(ThisPoint+1);
    GraphTotal.SetPoint(ThisPoint, FirstOrbitTime, TotalSum);
    CanByChannel[0]->cd(3);
    GraphTotal.Draw("Ap");
    GraphTotal.GetXaxis()->SetLimits(FirstOrbitTime - 14400000, FirstOrbitTime);
    //GraphTotal.GetYaxis()->SetRangeUser(0, GraphTotal.GetMaximum() * (1.2));

    for (std::vector<int>::iterator ich = Ch.begin(); ich != Ch.end(); ++ich) {
      HistByChannel[*ich]->Clear();
      HistByChannel[*ich]->Reset();
      uint64_t TotalInChannel = 0;
      for (int iOrbit = 0; iOrbit != NOrbitsToAvg; ++iOrbit) {
        if (IsChInOrbit[*ich][iOrbit]) {
          for (int ib = 0; ib < NBUCKETS; ++ib) {
            TotalInChannel += Counts[*ich][iOrbit][ib];
            if (ib >= FirstBucket && ib <= LastBucket) {
              HistByChannel[*ich]->Fill(ib, Counts[*ich][iOrbit][ib]);
            }
          }
        }
      }
    GraphCh[*ich].Set(ThisPoint+1);
    GraphCh[*ich].SetPoint(ThisPoint, FirstOrbitTime, TotalInChannel);
    CanByChannel[0]->cd(3);
    GraphCh[*ich].Draw("samep");
    GraphCh[*ich].GetXaxis()->SetLimits(FirstOrbitTime - 14400000, FirstOrbitTime);


      if (SeenFirstEOF) {
        CanByChannel[0]->cd(*ich + 1);
        HistByChannel[*ich]->Draw();
      }
      //CanByChannel[*ich]->SaveAs( TString(CanByChannel[*ich]->GetName()) + ".gif", "s");

      printf("Channel: %2i  OrbitTime: %15lu  Orbit: %15lu   Total: %15lu\n", (int) *ich, (unsigned long) 0, (unsigned long) 0, (unsigned long) TotalInChannel);
    }
    //if (!SeenFirstEOF) {
    //    sleep(1);
    //}
    CanByChannel[0]->Update();
    std::cout << std::endl;
  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [InFileName] [FirstBin] [LastBin]" << std::endl;
    return 1;
  }

  int myargc;
  char* myargv[0];
  //TApplication theApp("PLT", &myargc, myargv);
  //TestMainFrame mainWindow(gClient->GetRoot(), 400, 220);


  std::string const InFileName = argv[1];
  int const FirstBin = atoi(argv[2]);
  int const LastBin  = atoi(argv[3]);


  ProcessHistograms(InFileName, FirstBin, LastBin);
  //theApp.Run();

  return 0;
}
