////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Apr 18 15:12:56 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "TCanvas.h"
#include "TF1.h"

int Plot3ParamFit (float const a, float const b, float const c)
{
  TF1 f("f", "65. * ([0]*x*x + [1]*x + [2])", 650, 1000);
  f.SetParameter(0, a);
  f.SetParameter(1, b);
  f.SetParameter(2, c);
  TCanvas Can;
  Can.cd();
  f.Draw();
  Can.SaveAs("Plot3ParamFit.eps");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  float const a = atof(argv[1]);
  float const b = atof(argv[2]);
  float const c = atof(argv[3]);

  Plot3ParamFit(a, b, c);

  return 0;
}
