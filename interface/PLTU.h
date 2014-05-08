#ifndef GUARD_PLTU_h
#define GUARD_PLTU_h

// This is a namespace with some random utility function for PLT analysis.
// The kind of things I might put here are loosely related to the PLT
// but useful for many analysis..  a common space, for very general functions
// VERY GENERAL => translate: Don't put specific crap here, that crap goes elsewhere
//
// This is for function declarations


#include "TH1F.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TROOT.h"


namespace PLTU
{
  int const FIRSTCOL =  0;
  int const LASTCOL  = 51;
  int const FIRSTROW =  0;
  int const LASTROW  = 79;
  int const NCOL     = LASTCOL - FIRSTCOL + 1;
  int const NROW     = LASTROW - FIRSTROW + 1;

  // Width and height in centimeters
  float const PIXELWIDTH  = 0.0150;
  float const PIXELHEIGHT = 0.0100;

  Double_t PoissonFit(Double_t* x, Double_t* par);
  void SetStyle ();

  float GetMeanBinContentSkipEmptyBins (TH2F&);
  TH2F* Get3x3EfficiencyHist (TH2F&, int const, int const, int const, int const);
  TH1F* HistFrom2D(TH2F*, TString const NewName = "", int const NBins = -1, bool const SkipZeroBins = true);
  TH1F* HistFrom2D(TH2F*, float const ZMin, float const ZMax, TString const NewName = "", int const NBins = -1, bool const SkipZeroBins = true);
  float KahanSummation (std::vector<float>::iterator, std::vector<float>::iterator);
  float Average (std::vector<float>&);
  float KahanAverage (std::vector<float>&);
  void  AddToRunningAverage(double&, int&, double const);

}






















#endif
