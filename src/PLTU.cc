#include "PLTU.h"

#include "TMath.h"


namespace PLTU
{


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
    //gStyle->SetOptStat(11000010);
    gStyle->SetOptStat("e");
    gStyle->SetPadLeftMargin(0.17);
    gStyle->SetPadRightMargin(0.17);
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleX(0.1);
    gStyle->SetTitleY(1.0);
    gStyle->SetTitleH(0.09);
    gStyle->SetTitleW(0.7);
    //gStyle->SetTitleBorderSize(0);
    gStyle->SetStatY(0.88);
    gStyle->SetStatH(0.25);
    gStyle->SetCanvasBorderMode(0);


    return;
  }



  TH1F* HistFrom2D (TH2F* hIN, TString const NewName, int const NBins, bool const SkipZeroBins)
  {
    // This function returns a TH1F* and YOU are then the owner of
    // that memory.  please delete it when you are done!!!

    int const NBinsX = hIN->GetNbinsX();
    int const NBinsY = hIN->GetNbinsY();
    float const ZMin = hIN->GetMinimum();
    float const ZMax = hIN->GetMaximum() + 1;

    TString const hNAME = NewName == "" ? TString(hIN->GetName()) + "_1DZ" : NewName;

    TH1F* h;
    if (NBins > 0) {
      h = new TH1F(hNAME, hNAME, NBins, ZMin, ZMax);
    } else {
      h = new TH1F(hNAME, hNAME, (int) ZMax - ZMin + 1, ZMin, ZMax);
    }
    h->SetXTitle("Number of Hits");
    h->SetYTitle("Number of Pixels");
    h->GetXaxis()->CenterTitle();
    h->GetYaxis()->CenterTitle();
    h->SetTitleOffset(1.4, "y");
    h->SetFillColor(40);

    for (int ix = 1; ix <= NBinsX; ++ix) {
      for (int iy = 1; iy <= NBinsY; ++iy) {
        if (!SkipZeroBins || hIN->GetBinContent(ix, iy) != 0) {
          h->Fill( hIN->GetBinContent(ix, iy) );
        }
      }
    }

    return h;
  }


  float Average (std::vector<float>& V)
  {
    double Sum = 0.0;

    size_t i = 0;
    for ( ; i != V.size(); ++i) {
      Sum += V[i];
    }
    return Sum / (double) i;
  }


  float KahanSummation (std::vector<float>::iterator begin, std::vector<float>::iterator end)
  {
    // You should be careful how you sum a lot of things...

    double result = 0.0;

    double c = 0.0;
    double y, t;
    for ( ; begin != end; ++begin) {
      y = *begin - c;
      t = result + y;
      c = (t - result) - y;
      result = t;
    }
    return result;
  }



}
