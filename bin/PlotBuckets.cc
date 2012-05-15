////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue May 15 05:35:22 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <vector>
#include <map>

#include "TCanvas.h"
#include "TH1F.h"

#include "PLTHistReader.h"
#include "PLTU.h"



int PlotBuckets (std::string const InFileName, int const ENTRY)
{
  PLTHistReader HistReader(InFileName);

  for (int ientry = 0; HistReader.GetNextBuffer() >= 0; ++ientry) {
    if (ientry != ENTRY) {
      continue;
    }

    std::cout << "Found Entry: " << ENTRY << std::endl;


    // container for hists
    TH1F* Hist[6];

    // Loop over all channels (skip 2
    for (int ich = 0; ich != 6; ++ich) {
      if (ich == 2) continue;

      TString const Name = TString::Format("Buckets_Ch%i", ich);
      Hist[ich] = new TH1F(Name, Name, PLTHistReader::NBUCKETS, 0, PLTHistReader::NBUCKETS-1);

      for (int ib = 0; ib != PLTHistReader::NBUCKETS; ++ib) {
        Hist[ich]->SetBinContent(ib, HistReader.GetChBucket(ich, ib));
      }

    }


    // Make Canvas
    TCanvas Can("Buckets", "Buckets", 1200, 800);
    Can.Divide(1, 5);

    Can.cd(1);
    Hist[0]->Draw("hist");
    Can.cd(2);
    Hist[1]->Draw("hist");
    Can.cd(3);
    Hist[3]->Draw("hist");
    Can.cd(4);
    Hist[4]->Draw("hist");
    Can.cd(5);
    Hist[5]->Draw("hist");

    Can.SaveAs("Buckets.gif");

    break;



  }

  std::cout << "done." << std::endl;

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFile] [ientry]" << std::endl;
    return 1;
  }

  std::string const InFileName = argv[1];
  int const ENTRY = atoi(argv[2]);

  PlotBuckets(InFileName, ENTRY);

  return 0;
}
