#include "PLTU.h"

#include "TMath.h"


namespace PLTU
{
  const int FIRSTCOL = 13;
  const int LASTCOL = 38;
  const int FIRSTROW = 40;
  const int LASTROW = 79;
  const int NCOL=LASTCOL-FIRSTCOL+1;
  const int NROW=LASTROW-FIRSTROW+1;


  // From http://root.cern.ch/root/roottalk/roottalk03/0199.html
  Double_t PoissonFit(Double_t* x, Double_t* par) {
    Double_t xx = x[0];
    if(xx <= 0) return 0;
    // Poisson distribution
    // par[1] - distribution parameter 
    return par[0] * TMath::Power(par[1], xx) / TMath::Gamma(xx + 1) / TMath::Exp(par[1]);
  }



  void SetStyle ()
  {
    // Set some basic style for output plots
    gROOT->SetStyle("Plain");                  
    gStyle->SetPalette(1);
//    gStyle->SetOptStat(11000010);
    gStyle->SetOptStat("e");
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
    h->SetXTitle("Number of Hits");
    h->SetYTitle("Number of Pixels");
    h->GetXaxis()->CenterTitle();
    h->GetYaxis()->CenterTitle();
    h->SetTitleOffset(1.4, "y");
    h->SetFillColor(40);

    for (int ix = 1; ix <= NBinsX; ++ix) {
      for (int iy = 1; iy <= NBinsY; ++iy) {
        h->Fill( hIN->GetBinContent(ix, iy) );
      }
    }

    return h;
  }


}
