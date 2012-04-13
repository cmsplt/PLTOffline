////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Apr 18 15:12:56 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <cstdlib>


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
  Can.SaveAs("plots/Plot3ParamFit.eps");

  return 0;
}


int Plot5ParamFit (float const a, float const b, float const c, float const d, float const e)
{
  TF1 f("FitFunc", "[0]*x*x + [1]*x + [2] + TMath::Exp( (x-[3]) / [4]  )", 150, 400);
  f.SetParameter(0, a);
  f.SetParameter(1, b);
  f.SetParameter(2, c);
  f.SetParameter(3, d);
  f.SetParameter(4, e);
  TCanvas Can;
  Can.cd();
  f.Draw();
  Can.SaveAs("plots/Plot5ParamFit.gif");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4 && argc != 6) {
    std::cerr << "Usage: " << argv[0] << " and three/five constants " << std::endl;
    return 1;
  }

  float const a = atof(argv[1]);
  float const b = atof(argv[2]);
  float const c = atof(argv[3]);

  if (argc == 4) {
    Plot3ParamFit(a, b, c);
  } else if (argc == 6) {
    float const d = atof(argv[4]);
    float const e = atof(argv[5]);
    Plot5ParamFit(a, b, c, d, e);
  }

  return 0;
}
