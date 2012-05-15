////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri Apr 20 12:18:26 UTC 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <vector>
#include <map>

#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TLegend.h"
#include "TFile.h"
#include "TTree.h"

#include "PLTHistReader.h"

int const NBUCKETS = 3564;
int const NORBITS  = 4096;
int const NMAXCHANNELS = 9;

int HistNTP (std::string const InFileName, std::string const OutFileName)
{
  PLTHistReader HistReader(InFileName);


  uint32_t FirstTime;

  TFile OutFile(OutFileName.c_str(), "recreate");
  if (!OutFile.IsOpen()) {
    std::cerr << "ERROR: cannot open output file: " << OutFileName << std::endl;
    exit(1);
  }

  TTree Tree("PLTHist", "PLTHist");
  int Time;
  int Total = 0;
  int Hist[NMAXCHANNELS][NBUCKETS];
  Tree.Branch("Time", &Time, "Time/I");
  Tree.Branch("Total", &Total, "Total/I");
  Tree.Branch("Hist", Hist, "Hist[9][3564]/I");

  uint32_t LastTime;
  for (int ientry = 0; HistReader.GetNextBuffer() >= 0; ++ientry) {
    if (ientry % 1000 == 0) {
      std::cout << "Processing readout: " << ientry << std::endl;
    }

    if (ientry == 0) {
      FirstTime = HistReader.GetOrbitTime();
      LastTime = HistReader.GetOrbitTime();
    }
    Time = (int) (HistReader.GetOrbitTime() - FirstTime);



      Total = HistReader.GetTotal();
      std::vector<uint32_t>* Ch = HistReader.Channels();
      for (int ich = 0; ich != Ch->size(); ++ich) {
        int const Chan = (*Ch)[ich];
        for (int ibin = 0; ibin != NBUCKETS; ++ibin) {
          Hist[Chan][ibin] = HistReader.GetChBucket(Chan, ibin);
          //printf("Time %20i  Channel %2i  Bin %5i   Hits %15i\n", Time, Chan, ibin, (int) HistReader.GetChBucket(Chan, ibin));
        }
      }
      Tree.Fill();

      for (int i = 0; i != NMAXCHANNELS; ++i) {
        for (int j = 0; j != NBUCKETS; ++j) {
          Hist[i][j] = 0;
        }
      }
    }

    //printf("%15i  %2i %20u\n", (int) (HistReader.GetOrbitTime() - FirstTime), (int) HistReader.Channels()->size(), HistReader.GetTotal());

  Tree.Write();
  Tree.GetCurrentFile()->Write();
  Tree.GetCurrentFile()->Close();



  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [HistFileName] [OutFileName]" << std::endl;
    return 1;
  }

  HistNTP(argv[1], argv[2]);

  return 0;
}
