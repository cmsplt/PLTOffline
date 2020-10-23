////////////////////////////////////////////////////////////////////
//
// PlotTrackLumiVdM -- plots the data for a single VdM scan for the
// track-counting luminosity. Based (more or less) on
// PlotTrackLumiFill, but without the comparison to the HFOC/PLTZ
// luminosity. Also does not try to do the automatic exclusion of
// problematic channels since if that's happening in the VdM scan
// you're probably not going to have good data anyway.
//
// Paul Lujan, October 20, 20
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <time.h>
#include "TROOT.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLegend.h"

const int nPixelChannels = 13;

// The main timestamp file, which contains the separations for each point.
const char *separationFileName = "VdMSteps_4954_AllScans.txt";

// To call directly, you need three arguments: the name of the scan file, the name of the output file, and the
// title for the plot. Some defaults are also provided in the utility functions at the end.

void PlotTrackLumiVdM(const char *scanFileName, const char *outFileName, const char *plotTitle) {
  gROOT->SetStyle("Plain");
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadLeftMargin(0.12);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleX(0.1);
  gStyle->SetTitleY(1.0);
  gStyle->SetTitleH(0.09);
  gStyle->SetTitleW(0.7);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetLegendBorderSize(0);
  //gStyle->SetOptFit(1111);

  // Read separation file.
  std::vector<std::tuple<int, int, float> > separationByTimestamp;
  std::ifstream separationFile(separationFileName);
  if (!separationFile.is_open()) {
    std::cerr << "Couldn't open separation timestamps file!" << std::endl;
    return;
  }
  // Go through the lines of the file.
  std::string line;
  while (1) {
    std::getline(separationFile, line);
    if (separationFile.eof()) break;
    if (line.empty()) continue; // skip blank lines
    if (line.at(0) == '#' || line.at(0) == '=') continue; // skip comment lines and header lines for new scans

    // Break into fields
    std::stringstream ss(line);
    int timestampStart, timestampEnd;
    float separation;
    ss >> timestampStart >> timestampEnd >> separation;
    separationByTimestamp.push_back(std::make_tuple(timestampStart, timestampEnd, separation));
  }
  separationFile.close();

  // Now actually read in the luminosity information.

  std::vector<double> sepVal;
  std::vector<double> sepErr;
  std::vector<double> trackLumiVal;
  std::vector<double> trackLumiErr;

  std::ifstream scanFile(scanFileName);
  if (!scanFile.is_open()) {
    std::cerr << "Couldn't open track luminosity file!" << std::endl;
    return;
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig, nFilledTrig;
  float tracksAll, tracksGood, nEmpty, nFull;

  // Get header line.
  std::getline(scanFile, line);
  std::stringstream ss(line);
  ss >> nsteps >> nBunches;
  // Go through the rest of the lines.
  int n = 0;
  while (1) {
    std::getline(scanFile, line);
    if (scanFile.eof()) break;
    if (line.empty()) continue; // skip blank lines
    if (line.at(0) == '#') continue; // skip comment lines

    // Break into fields. Note that we ignore the per-channel data at the end (if present).
    std::stringstream ss(line);
    ss >> tBegin >> tEnd >> nTrig >> tracksAll >> tracksGood >> nFilledTrig >> nEmpty >> nFull;

    // Find the separation corresponding to this timestamp.
    float separation = -99;
    for (int i=0; i<separationByTimestamp.size(); ++i) {
      if (tBegin == std::get<0>(separationByTimestamp[i]) && tEnd == std::get<1>(separationByTimestamp[i])) {
	separation = std::get<2>(separationByTimestamp[i]);
	break;
      }
    }
    if (separation == -99) {
      std::cerr << "Failed to find separation for time interval " << tBegin << " to " << tEnd << std::endl;
      return;
    }
    n++;
    // Skip the first and last lines since these are the head-on steps at the beginning and end of the scan
    if ((n == 1 || n == nsteps) && separation == 0) {
      continue;
    }
     
    // Compute the luminosity and add it in.
    sepVal.push_back(separation);
    sepErr.push_back(0);

    double zeroFrac = 1.0-(double)nFull/nFilledTrig;
    double zeroFracErr = sqrt(zeroFrac*(1-zeroFrac)/nFilledTrig);
    
    double goodTrackZC = -log(zeroFrac);
    double goodTrackZCPlus = -log(zeroFrac + zeroFracErr);
    double goodTrackZCMinus = -log(zeroFrac - zeroFracErr);
    // use symmetrized error
    double goodTrackErr = (std::abs(goodTrackZCPlus-goodTrackZC) + std::abs(goodTrackZCMinus-goodTrackZC))/2;

    //std::cout << "separation " << separation << " lumi " << goodTrackZC << " +/- " << goodTrackErr << std::endl;

    // scale by 1000 just so we don't have a lot of decimal places lying around
    trackLumiVal.push_back(goodTrackZC*1000);
    trackLumiErr.push_back(goodTrackErr*1000);
  }
  scanFile.close();

  // Plot and fit the results.

  TGraph *g1 = new TGraphErrors(sepVal.size(), sepVal.data(), trackLumiVal.data(), sepErr.data(), trackLumiErr.data());

  TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
  g1->Draw("AP");
  g1->SetTitle(plotTitle);
  g1->GetXaxis()->SetTitle("Separation (mm)");
  g1->GetYaxis()->SetTitle("Track luminosity (a.u.)");
  g1->GetYaxis()->SetTitleOffset(1.2);
  g1->SetMarkerStyle(kFullCircle);
  g1->SetMarkerColor(kBlue);
  g1->SetMarkerSize(1);

  TF1 *f1 = new TF1("f1", "gaus");
  f1->SetLineColor(kRed);
  f1->SetLineWidth(2);
  g1->Fit(f1);

  TLegend *l1 = new TLegend(0.65, 0.78, 0.9, 0.88);
  l1->AddEntry(g1, "Data", "P");
  l1->AddEntry(f1, "Gaussian fit", "L");
  l1->SetBorderSize(0);
  l1->SetFillColor(0);
  l1->Draw();

  std::stringstream fitWidthText;
  fitWidthText << "Fitted width: " << std::fixed << std::setprecision(1) << f1->GetParameter(2)*1000 << " #kern[-0.2]{#mu}#kern[-0.1]{m}";
  TLatex *t1 = new TLatex(0, 0, fitWidthText.str().c_str());
  t1->SetNDC();
  t1->SetX(0.65);
  t1->SetY(0.70);
  t1->SetTextSize(0.03);
  t1->Draw();

  c1->Print(outFileName);
}

// If called with no arguments, then do the X and Y scans for all bunches
void PlotTrackLumiVdM() {
  PlotTrackLumiVdM("TrackLumiZC_4954_X1.txt", "TrackLumi_VdM_4954_X1_All.png", "Fill 4954, Scan X1, all bunches");
  PlotTrackLumiVdM("TrackLumiZC_4954_Y1.txt", "TrackLumi_VdM_4954_Y1_All.png", "Fill 4954, Scan Y1, all bunches");
}

// If called with one argument, then the argument is whether to do the bunches individually or not
void PlotTrackLumiVdM(bool doBX) {
  if (doBX) {
    const std::vector<int> bunches = {1, 41, 81, 110, 121, 161, 201, 241, 281, 591, 872, 912, 952, 992,
				      1032, 1072, 1112, 1151, 1152, 1682, 1783, 1823, 1863, 1903, 1943, 1983,
				      2023, 2063, 2654, 2655, 2694, 2734, 2774, 2814, 2854, 2894, 2934};
    for (unsigned int i=0; i<bunches.size(); ++i) {
      std::stringstream fileString;
      fileString << "TrackLumi_4954_X1_BX/TrackLumiZC" << std::setfill('0') << std::setw(4) << bunches[i] << ".txt";
      std::stringstream plotString;
      plotString << "TrackLumi_VdM_4954_X1_" << std::setfill('0') << std::setw(4) << bunches[i] << ".png";
      std::stringstream titleString;
      titleString <<  "Fill 4954, Scan X1, bunch " << bunches[i];
      PlotTrackLumiVdM(fileString.str().c_str(), plotString.str().c_str(), titleString.str().c_str());

      fileString.str("");
      fileString << "TrackLumi_4954_Y1_BX/TrackLumiZC" << std::setfill('0') << std::setw(4) << bunches[i] << ".txt";
      plotString.str("");
      plotString << "TrackLumi_VdM_4954_Y1_" << std::setfill('0') << std::setw(4) << bunches[i] << ".png";
      titleString.str("");
      titleString <<  "Fill 4954, Scan Y1, bunch " << bunches[i];
      PlotTrackLumiVdM(fileString.str().c_str(), plotString.str().c_str(), titleString.str().c_str());
    }
  } else {
    PlotTrackLumiVdM();
  }
}
