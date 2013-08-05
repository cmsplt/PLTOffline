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
    gStyle->SetLegendFillColor(0);
    gStyle->SetLegendBorderSize(0);


    return;
  }



  float GetMeanBinContentSkipEmptyBins (TH2F& h)
  {
    int MyNotEmptyBinCount = 0;
    double Sum = 0;
    for (int i = 1; i <= h.GetNbinsX(); ++i) {
      for (int j = 1; j <= h.GetNbinsY(); ++j) {
        if (h.GetBinContent(i, j) != 0) {
          ++MyNotEmptyBinCount;
          Sum += h.GetBinContent(i, j);
        }
      }
    }

    if (MyNotEmptyBinCount == 0) {
      return 0;
    }

    return ((float) Sum) / (float) MyNotEmptyBinCount;
  }



  TH2F* Get3x3EfficiencyHist (TH2F& HistIn, int const FirstCol, int const LastCol, int const FirstRow, int const LastRow)
  {
    // This will creat a new TH2 which YOU are the owner of.

    // This function calculates the average of neighboring bins, excluding bins with no
    // entries and bins outside a specified range.  The bin in question is then divided
    // by that average
    //
    // If all neighbor bins are empty and so is this bin we set the 3x3Efficiency to 0
    // If all neighbor bins are empty but this bin has content we set 3x3Efficiency to 1


    // Get the same format histogram and clear it.  This is where we have created memory that
    // YOU are responsible for!!!
    TH2F* HistEff = (TH2F*) HistIn.Clone(HistIn.GetName() + TString("_3x3Efficiency"));
    HistEff->Reset();

    // Loop over all columns
    for (int icol = 1; icol <= HistIn.GetNbinsX(); ++icol) {

      // What pixel column is this?  If it's outside the range skip it
      int const PixCol = (int) HistEff->GetXaxis()->GetBinLowEdge(icol);
      if (PixCol < FirstCol || PixCol > LastCol) {
        continue;
      }

      // Loop over all rows
      for (int irow = 1; irow <= HistIn.GetNbinsY(); ++irow) {

        // What row is this?  If it's outside the range skip it
        int const PixRow = (int) HistEff->GetYaxis()->GetBinLowEdge(irow);
        if (PixRow < FirstRow || PixRow > LastRow) {
          continue;
        }


        // For *this* pixel get the 3x3 surrounding values..  If a neighbor is outside of bounds we skip it
        std::vector<float> HitsNearThisPixel;
        for (int i = -1; i <= 1; ++i) {
          for (int j = -1; j <= 1; ++j) {
            if (PixCol + i >= FirstCol && PixCol + i <= LastCol && PixRow + j >= FirstRow && PixRow + j <= LastRow) {
              if ( (i != 0 || j != 0) && HistIn.GetBinContent(icol + i, irow + j) > 0) {
                HitsNearThisPixel.push_back( HistIn.GetBinContent(icol + i, irow + j));

              }
            }
          }
        }


        // If we have neighbor hits let's dvide, if not ask if this bin is empty.  If empty set to 0, if not empty set to 1
        if (HitsNearThisPixel.size() > 0) {
          // Calcluate average neighbor occupancy
          float const NeighborsMean = Average(HitsNearThisPixel);
          HistEff->SetBinContent(icol, irow, HistIn.GetBinContent(icol, irow) / NeighborsMean);

        } else if (HistIn.GetBinContent(icol, irow) == 0) {
          HistEff->SetBinContent(icol, irow, 0);
        } else {
          HistEff->SetBinContent(icol, irow, 1);
        }

      }
    }

    return HistEff;
  }




  TH1F* HistFrom2D (TH2F* hIN, float const ZMin, float const ZMax, TString const NewName, int const NBins, bool const SkipZeroBins)
  {
    // This function returns a TH1F* and YOU are then the owner of
    // that memory.  please delete it when you are done!!!

    int const NBinsX = hIN->GetNbinsX();
    int const NBinsY = hIN->GetNbinsY();

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


  TH1F* HistFrom2D (TH2F* hIN, TString const NewName, int const NBins, bool const SkipZeroBins)
  {
    // This function returns a TH1F* and YOU are then the owner of
    // that memory.  please delete it when you are done!!!

    float const ZMin = hIN->GetMinimum();
    float const ZMax = hIN->GetMaximum() + 1;



    TH1F* h = HistFrom2D(hIN, ZMin, ZMax, NewName, NBins, SkipZeroBins);
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


  float KahanAverage (std::vector<float>& Vec)
  {
    if (Vec.size() == 0) {
      return 0;
    }

    float const Sum = KahanSummation(Vec.begin(), Vec.end());
    return Sum / (float) Vec.size();
  }


  void AddToRunningAverage (double& Average, int& N, double const NewValue)
  {
    Average = Average * ((double) N / ((double) N + 1.)) + NewValue / ((double) N + 1.);
    ++N;

  }


}
