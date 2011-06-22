#include "PLTU.h"



namespace PLTU
{

  void SetStyle ()
  {
    // Set some basic style for output plots
    gROOT->SetStyle("Plain");                  
    gStyle->SetPalette(1);
    gStyle->SetOptStat(11000010);
    gStyle->SetPadLeftMargin (0.17);
    gStyle->SetPadRightMargin (0.17);

    return;
  }

  TH1F* HistFrom2D(TH2F* hIN, TString const NewName)
  {
    // This function returns a TH1F* and YOU are then the owner of
    // that memory.  please delete it when you are done!!!

    int const NBinsX = hIN->GetNbinsX();
    int const NBinsY = hIN->GetNbinsY();
    int const ZMin = hIN->GetMinimum();
    int const ZMax = hIN->GetMaximum();

    TString const hNAME = NewName == "" ? TString(hIN->GetName()) + "_1DZ" : NewName;

    TH1F* h = new TH1F(hNAME, hNAME, (int) ZMax - ZMin + 1, ZMin, ZMax);

    for (int ix = 1; ix <= NBinsX; ++ix) {
      for (int iy = 1; iy <= NBinsY; ++iy) {
        h->Fill( hIN->GetBinContent(ix, iy) );
      }
    }

    return h;
  }


}
