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
//#include "TApplication.h"
//#include "TGMainFrame.h"



int const NBUCKETS = 3564;
int const NORBITS  = 4096;
int const NMAXCHANNELS = 6;

int ProcessHistograms (std::string const InFileName, int const FirstBucket, int const LastBucket)
{

  std::ifstream InFile(InFileName.c_str(), std::ios::in | std::ios::binary);
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    exit(1);
  }

  int const NOrbitsToAvg = 9;

  uint32_t OrbitTime;
  uint32_t Orbit;
  uint32_t Channel;
  uint32_t BigBuff[NBUCKETS];

  bool GotIt;

  uint32_t OTime[NMAXCHANNELS][NOrbitsToAvg];
  uint32_t O[NMAXCHANNELS][NOrbitsToAvg];
  uint32_t Counts[NMAXCHANNELS][NOrbitsToAvg][NBUCKETS];
  std::vector<int> Ch;

  bool IsChInOrbit[NMAXCHANNELS][NOrbitsToAvg];
  for (int i = 0; i != NOrbitsToAvg; ++i) {
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
  }
  for (int i = 0; i != 1; ++i) {
    TString const NameC = TString::Format("HistByChannel_Ch%i", i);
    CanByChannel[i] = new TCanvas(NameC, NameC, 900, 700);
    CanByChannel[i]->Divide(1, NMAXCHANNELS);
  }


  while (true) {

    for (int iOrbit = 0; iOrbit < NOrbitsToAvg; ++iOrbit) {

      GotIt = false;
      while (!GotIt) {
        InFile.read( (char*) &OrbitTime, sizeof(uint32_t));
        if (InFile.eof()) {
          usleep(200000);
          InFile.clear();
        } else {
          GotIt = true;
        }
      }

      GotIt = false;
      while (!GotIt) {
        InFile.read( (char*) &Orbit, sizeof(uint32_t));
        if (InFile.eof()) {
          usleep(200000);
          InFile.clear();
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
          } else {
            GotIt = true;
            ++iBucket;
          }
        }
      }

      IsChInOrbit[Channel][iOrbit] = true;
      bool IsInVec = false;
      for (size_t imy = 0; imy != Ch.size(); ++imy) {
        if (Channel == Ch[imy]) {
          IsInVec = true;
        }
      }
      //if (std::count(Ch.begin(), Ch.end(), Channel) == 0) {
      if (!IsInVec) {
        Ch.push_back(Channel);
      }

      for (int ib = 0; ib < NBUCKETS; ++ib) {
        Counts[Channel][iOrbit][ib] = (BigBuff[ib] & 0xfff);
      }

      OTime[Channel][iOrbit] = OrbitTime;
      O[Channel][iOrbit] = Orbit;


    }



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
      CanByChannel[0]->cd(*ich + 1);
      HistByChannel[*ich]->Draw();
      //CanByChannel[*ich]->SaveAs( TString(CanByChannel[*ich]->GetName()) + ".gif", "s");

      printf("Channel: %2i  OrbitTime: %15lu  Orbit: %15lu   Total: %15lu\n", (int) *ich, (unsigned long) 0, (unsigned long) 0, (unsigned long) TotalInChannel);
    }
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
  //TApplication theApp("PLT", &argc, argv);

  std::string const InFileName = argv[1];
  int const FirstBin = atoi(argv[2]);
  int const LastBin  = atoi(argv[3]);


  ProcessHistograms(InFileName, FirstBin, LastBin);
  //theApp.Run();

  return 0;
}
