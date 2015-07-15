////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu May 26 09:34:01 CEST 2011
//
////////////////////////////////////////////////////////////////////


// NOTE (kga): this needs a good look at 
// http://root.cern.ch/root/html/tutorials/fit/line3Dfit.C.html
// but maybe that exists somewhere else in this package...moving on.

#include <iostream>

#include "TGraph2D.h"
#include "TCanvas.h"
#include "TF2.h"

int GeoTest ()
{
  float X[3] = {1, 2, 2.6};
  float Y[3] = {1, 2, 3};
  float Z[3] = {1, 2, 2.6};

  TGraph2D g(3, X, Y, Z);
  TF2 Line("Line", "[0] + [1]*x + [2]*y", 1, 3, 1, 3);
  g.Fit("Line", "MLP");
  g.SetMarkerStyle(8);
  g.SetMarkerSize(1);
  TCanvas C;
  g.Draw("AP");
  Line.SetLineColor(2);
  Line.SetLineWidth(2);
  Line.Draw("same");
  C.SaveAs("plots/GeoTestPlot.eps");
  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  GeoTest();

  return 0;
}
