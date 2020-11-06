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
#include "TGraph.h"
#include "TFile.h"

#include "PLTHistReader.h"
#include "PLTU.h"



int PlotBuckets (std::string const InFileName, int const ENTRY)
{
  PLTHistReader HistReader(InFileName);

  //PLTU::SetStyle();
  gStyle->SetOptStat(0);

  TGraph gLumi;
  TGraph* Lumi[6];
  Lumi[0] = new TGraph();
  Lumi[1] = new TGraph();
  Lumi[3] = new TGraph();
  Lumi[4] = new TGraph();
  Lumi[5] = new TGraph();
  // container for hists
  TH1F* Hist[6];
  // Loop over all channels (skip 2
  for (int ich = 0; ich != 6; ++ich) {
    if (ich == 2) continue;

    TString const Name = TString::Format("Buckets_Ch%i", ich);
    Hist[ich] = new TH1F(Name, Name, PLTHistReader::NBUCKETS, 0, PLTHistReader::NBUCKETS-1);


  }

  for (int ientry = 0; HistReader.GetNextBuffer() >= 0; ++ientry) {
    gLumi.Set(ientry + 1);
    gLumi.SetPoint(ientry, ientry, HistReader.GetTotal());
    if (ientry % 1000 == 0) {
      printf("ientry: %15i   %15i\n", ientry, HistReader.GetOrbitTime());
    }
    for (int ich = 0; ich != 6; ++ich) {
      if (ich == 2) continue;
      Lumi[ich]->Set(ientry + 1);
      if (HistReader.GetTotalInChannel(ich) < PLTHistReader::NBUCKETS*4000) {
        Lumi[ich]->SetPoint(ientry, ientry, HistReader.GetTotalInChannel(ich));
      } else {
        Lumi[ich]->SetPoint(ientry, ientry, 0);
      }
    }


    if (ientry != ENTRY) {
      continue;
    }

    std::cout << "Found Entry: " << ENTRY << std::endl;
    std::cout << "At Time: " << HistReader.GetOrbitTime() << std::endl;



    // Loop over all channels (skip 2
    for (int ich = 0; ich != 6; ++ich) {
      if (ich == 2) continue;
      for (int ib = 0; ib != PLTHistReader::NBUCKETS; ++ib) {
        Hist[ich]->SetBinContent(ib, HistReader.GetChBucket(ich, ib));
      }

    }


    break;



  }

  // Make a TFile
  TFile OutFile(TString::Format("Buckets_%i.root", ENTRY), "recreate");
  OutFile.cd();
  // Make Canvas
  TCanvas Can("Buckets", "Buckets", 1200, 800);
  Can.Divide(2, 6);

  Can.cd(1);
  Can.cd(2);
  gLumi.Draw("Ap");
  Can.cd(3);
  Lumi[0]->Draw("Ap");
  Can.cd(4);
  Hist[0]->Draw("hist");
  Can.cd(5);
  Lumi[1]->Draw("Ap");
  Can.cd(6);
  Hist[1]->Draw("hist");
  Can.cd(7);
  Lumi[3]->Draw("Ap");
  Can.cd(8);
  Hist[3]->Draw("hist");
  Can.cd(9);
  Lumi[4]->Draw("Ap");
  Can.cd(10);
  Hist[4]->Draw("hist");
  Can.cd(11);
  Lumi[5]->Draw("Ap");
  Can.cd(12);
  Hist[5]->Draw("hist");

  Can.SaveAs(TString::Format("Buckets_%i.gif", ENTRY));
  Can.Write();

  for (int i = 0; i != 6; ++i) {
    if (i == 2) {
      continue;
    }

    Hist[i]->Write();
    Lumi[i]->Write();
  }

  gLumi.Write();

  OutFile.Close();

  std::cout << gLumi.GetMaximum() << std::endl;

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
