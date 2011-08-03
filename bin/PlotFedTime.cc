////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon May 30 16:44:37 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <fstream>

#include "TGraph.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TLine.h"
#include "TFrame.h"


int PlotFedTime (std::string const InFileName)
{
  std::ifstream In(InFileName.c_str());
  if (!In.is_open()) {
    std::cerr << "ERROR: cannot open input file: " << InFileName << std::endl;
    throw;
  }

  int const N = 960;
  float X[N], Y[N];

  std::vector<TGraph*> Graphs;

  while (!In.eof()) {

    for (int i = 0; i != N; ++i) {
      In >> X[i] >> Y[i];
    }

    Graphs.push_back( new TGraph(N, X, Y) );
  }

  std::cout << "NGraphs: " << Graphs.size() << std::endl;

  TCanvas Can;
  Can.cd();
  for (size_t i = 0; i != Graphs.size(); ++i) {
    Graphs[i]->SetMarkerColor(i+1);
    Graphs[i]->GetXaxis()->SetRangeUser(400, 900);
    Graphs[i]->SetMarkerStyle(2);
    if (i == 0) {
      Graphs[i]->Draw("AP");
    } else {
      Graphs[i]->Draw("P");
    }
  }
  Graphs[0]->SetTitle("FED Timing");

  TLine* Lines[5];
  Lines[0] = new TLine(400, Graphs[0]->GetYaxis()->GetXmin(), 400, Graphs[0]->GetYaxis()->GetXmax());
  Lines[1] = new TLine(600, Graphs[0]->GetYaxis()->GetXmin(), 600, Graphs[0]->GetYaxis()->GetXmax());
  Lines[2] = new TLine(700, Graphs[0]->GetYaxis()->GetXmin(), 700, Graphs[0]->GetYaxis()->GetXmax());
  Lines[3] = new TLine(800, Graphs[0]->GetYaxis()->GetXmin(), 800, Graphs[0]->GetYaxis()->GetXmax());
  Lines[4] = new TLine(900, Graphs[0]->GetYaxis()->GetXmin(), 900, Graphs[0]->GetYaxis()->GetXmax());
  for (int i = 0; i != 5; ++i) {
    Lines[i]->SetLineColor(2);
    Lines[i]->Draw("same");
  }
  Can.SaveAs("plots/Fedtime.gif");


  for (size_t i = 0; i != Graphs.size(); ++i) {
    delete Graphs[i];
  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  std::string const InFileName = argv[1];

  PlotFedTime(InFileName);

  return 0;
}
