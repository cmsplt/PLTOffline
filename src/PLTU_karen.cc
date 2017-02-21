#include "PLTU_karen.h"

#include "TMath.h"


namespace PLTU_karen
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
    
    gROOT->SetStyle("Plain");
    gStyle->SetPalette(1);
    gStyle->SetOptStat(0);
    gStyle->SetLabelFont(42,"XYZ");
    gStyle->SetLabelSize(0.03,"XYZ");
    gStyle->SetTextFont(42);
    gStyle->SetTextSize(0.07);
    //gStyle->SetTitleFont(42,"t");
    gStyle->SetTitleFontSize(0.06);
    gStyle->SetTitleX(0.15f);
    gStyle->SetTitleY(0.97f);
    //gStyle->SetTitleFont(42,"XYZ");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetCanvasDefW(500);
    gStyle->SetCanvasDefW(500);
    gStyle->SetPadGridX(1);
    gStyle->SetPadGridY(1);
    
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
      h = new TH1F(hNAME, hNAME, (int)(ZMax - ZMin + 1), ZMin, ZMax);
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
