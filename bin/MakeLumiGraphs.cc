////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri May 11 11:42:15 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TString.h"


int MakeLumiGraphs (TString const InFileName, TString const OutFileName)
{
  TFile InFile(InFileName, "read");
  if (!InFile.IsOpen()) {
    std::cerr << "ERROR: cannot open input file: " << InFileName << std::endl;
    exit(1);
  }

  TTree* Tree = (TTree*) InFile.Get("t1");
  if (!Tree) {
    std::cerr << "ERROR: cannot find tree!" << std::endl;
    exit(1);
  }


  TFile OutFile(OutFileName, "create");
  if (!OutFile.IsOpen()) {
    std::cerr << "ERROR: cannot create out file: " << OutFileName << std::endl;
    exit(1);
  }
  OutFile.cd();

  TGraph* gr;

  TCanvas Can;
  Can.cd();

  //TString const TimeOffset = "-58344973";
  //TString const TimeOffset = "-43501008-63000";
  TString const TimeOffset = "-43501008";

  Tree->Draw("htot:(time_orbit"+TimeOffset+")", "ch == 0");
  gr = new TGraph(Tree->GetSelectedRows(), Tree->GetV2(), Tree->GetV1());
  gr->SetName("LumiRages_Ch0");
  gr->Write();

  Tree->Draw("htot:(time_orbit"+TimeOffset+")", "ch == 1");
  gr = new TGraph(Tree->GetSelectedRows(), Tree->GetV2(), Tree->GetV1());
  gr->SetName("LumiRages_Ch1");
  gr->Write();

  Tree->Draw("htot:(time_orbit"+TimeOffset+")", "ch == 3");
  gr = new TGraph(Tree->GetSelectedRows(), Tree->GetV2(), Tree->GetV1());
  gr->SetName("LumiRages_Ch3");
  gr->Write();

  Tree->Draw("htot:(time_orbit"+TimeOffset+")", "ch == 4");
  gr = new TGraph(Tree->GetSelectedRows(), Tree->GetV2(), Tree->GetV1());
  gr->SetName("LumiRages_Ch4");
  gr->Write();

  Tree->Draw("htot:(time_orbit"+TimeOffset+")", "ch == 5");
  gr = new TGraph(Tree->GetSelectedRows(), Tree->GetV2(), Tree->GetV1());
  gr->SetName("LumiRages_Ch5");
  gr->Write();



  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFile] [OutFile]" << std::endl;
    return 1;
  }

  MakeLumiGraphs(argv[1], argv[2]);

  return 0;
}
