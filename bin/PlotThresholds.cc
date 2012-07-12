////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Jul  9 14:28:04 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>

#include "PLTU.h"

#include "TNtuple.h"
#include "TString.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"


std::map<TString, std::map<TString, std::vector< std::pair<int, float> > > > Map;

int PlotThresholds (TString const InFileName)
{
  std::fstream InFile(InFileName.Data());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    throw;
  }


  int mf, mfc, hub, col, row, roc, vcal, hits;
  float eff;
  TString Name, ROWCOL;
  for (std::string line; std::getline(InFile, line); ) {
    std::istringstream LineStr;
    LineStr.str(line);

    LineStr >> mf >> mfc >> hub >> col >> row >> roc >> vcal >> hits >> eff;
    Name.Form("%i%i%02i%i", mf, mfc, hub, roc);
    ROWCOL.Form("%02i%02i", col, row);
    Map[Name][ROWCOL].push_back( std::make_pair<int, float>(vcal, eff) );
  }




  for (std::map<TString, std::map<TString, std::vector< std::pair<int, float> > > >::iterator It = Map.begin(); It != Map.end(); ++It) {
    sscanf(It->first, "%1i%1i%02i%1i", &mf, &mfc, &hub, &roc);

    TString const ThisName = TString::Format("Threshold_mf%i_mfc%i_hub%i_ROC%i", mf, mfc, hub, roc);
    TString const ThisNameW = TString::Format("ThresholdWidth_mf%i_mfc%i_hub%i_ROC%i", mf, mfc, hub, roc);
    TH1F hROC(ThisName, ThisName, 100, 0, 100);
    TH1F hROCWidth(ThisNameW, ThisNameW, 100, 0, 3);

    TString const ThisName2D = TString::Format("Threshold2D_mf%i_mfc%i_hub%i_ROC%i", mf, mfc, hub, roc);
    TH2F hThresholdMap(ThisName2D, ThisName2D, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);


    for (std::map<TString, std::vector< std::pair<int, float> > >::iterator It2 = It->second.begin(); It2 != It->second.end(); ++It2) {
      sscanf(It2->first, "%02i%02i", &col, &row);

      TString const GraphName = TString::Format("Graph_%i%i%02i%i_%2i%2i", mf, mfc, hub, roc, col, row);
      int ip = 0;
      TGraph g(It2->second.size());
      g.SetName(GraphName);
      for (std::vector< std::pair<int, float> >::iterator Vec = It2->second.begin(); Vec != It2->second.end(); ++Vec) {
        g.SetPoint(ip, Vec->first, Vec->second);
        ++ip;
      }
      TF1 sfit("sfit","0.5*TMath::Erf([0]*(x - [1]))+0.5",0,250);
      sfit.SetParameter(0, 0.025);
      sfit.SetParameter(1, 30);
      g.Fit(&sfit, "Q");
      hROC.Fill(sfit.GetParameter(1));
      hROCWidth.Fill(sfit.GetParameter(0));

      if (sfit.GetParameter(1) > 250) {
        hThresholdMap.SetBinContent(col - PLTU::FIRSTCOL + 1, row - PLTU::FIRSTROW + 1, 250);
      } else if (sfit.GetParameter(1) < 0) {
        hThresholdMap.SetBinContent(col - PLTU::FIRSTCOL + 1, row - PLTU::FIRSTROW + 1, 0);
      } else {
        hThresholdMap.SetBinContent(col - PLTU::FIRSTCOL + 1, row - PLTU::FIRSTROW + 1, sfit.GetParameter(1));
      }

      if (false) {
        TCanvas gCan;
        gCan.cd();
        g.Draw("A*");
        gCan.SaveAs(GraphName + ".gif");
      }

    }

    TCanvas Can;
    Can.cd();
    hROC.Draw("hist");
    Can.SaveAs( TString(hROC.GetName()) + ".gif");
    hThresholdMap.Draw("colz");
    Can.SaveAs( TString(hThresholdMap.GetName()) + ".gif");
    hROCWidth.Draw("hist");
    Can.SaveAs( TString(hROCWidth.GetName()) + ".gif");

    printf("mf %i mfc %i hub %2i ROC %i  ThreshAvg: %12.3f  Sigma: %12.3f  Width: %12.6f  WidthSigna: %12.6f\n", mf, mfc, hub, roc, hROC.GetMean(), hROC.GetRMS(), hROCWidth.GetMean(), hROCWidth.GetRMS());
  }
      //TF1 *sfit = new TF1("sfit","50.*TMath::Erf([0]*x - [1])+50.",0,250);


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFile]" << std::endl;
    return 1;
  }

  PlotThresholds(argv[1]);

  return 0;
}
