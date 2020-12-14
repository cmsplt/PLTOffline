////////////////////////////////////////////////////////////////////
//
// PlotTrackLumiVdM -- plots the data for a single VdM scan for the
// track-counting luminosity. Based (more or less) on
// PlotTrackLumiFill, but without the comparison to the HFOC/PLTZ
// luminosity. Also does not try to do the automatic exclusion of
// problematic channels since if that's happening in the VdM scan
// you're probably not going to have good data anyway.
//
// Paul Lujan, October 20, 2020
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <time.h>
#include <math.h>
#include "TROOT.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TAxis.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TLine.h"

const std::string fillNumber = "6016"; // actually a string
const std::string scanPair = "4"; // select X1/Y1, X2/Y2, etc. here
const int nPixelChannels = 14; // 13 for 2016, 14 for 2017-18

// The main timestamp file, which contains the separations for each point.
const std::string separationFileName = "VdMSteps_"+fillNumber+"_AllScans.txt";

// A little helper function to convert the channel number (in the order found in the original file) to a
// readout channel number.
int convertToReadoutChannel(std::string fill, int i) {
  if (fill == "4945" || fill == "4954") {
    // For 2016 the active channels were readout channels 1-13, so we just need to add 1
    return i+1;
  } else if (fill == "6016") {
    // For 2017-18 the active channels were 1-3 and 5-16, so it's a little more complicated
    // (but not much more)
    if (i <= 2)
      return i+1;
    return i+2;
  } else {
    std::cerr << "Error: don't know active channels for fill " << fillNumber << "; please add them to the script" << std::endl;
    return i;
  }
}

// To call directly, you need three arguments: the name of the scan file, the name of the output file, and the
// title for the plot. You can also add a channel number as a 4th argument, or -1 (default) to plot the
// all-channel average. Note that the "channel number" refers simply to the order that they appear in the
// file, which may not be the real channel number. (Converting that channel number to the actual readout
// channel number is done by the function above.) Some defaults are also provided in the utility functions at
// the end. The function returns four floats in a std::tuple: the fitted peak and its error, and the fitted
// CapSigma and its error (which is just the width of the Gaussian).

std::tuple<float, float, float, float> PlotTrackLumiVdM(const char *scanFileName, const char *outFileName, const char *plotTitle, int channelNum = -1) {
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
    return std::make_tuple(-1, -1, -1, -1);
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
    return std::make_tuple(-1, -1, -1, -1);
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

    // Break into fields.
    std::stringstream ss(line);
    ss >> tBegin >> tEnd >> nTrig >> tracksAll >> tracksGood >> nFilledTrig >> nEmpty >> nFull;

    // If we're using a specific channel, then also get the per-channel data and use the nFull value for that
    // channel.
    if (channelNum >= 0) {
      int channelData[nPixelChannels];
      for (int i=0; i<nPixelChannels; ++i)
	ss >> channelData[i];
      nFull = channelData[channelNum];
    }

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
      return std::make_tuple(-1, -1, -1, -1);
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
    int nSamples = nFilledTrig;
    // If we're averaging over all channels, then nFull is the average over all channels, which means that we
    // really have nFilledTrig*nChannels data points, so use that as our denominator in computing the binomial
    // error.
    if (channelNum == -1)
      nSamples *= nPixelChannels;

    double zeroFracErr = sqrt(zeroFrac*(1-zeroFrac)/nSamples);
    
    double goodTrackZC = -log(zeroFrac);
    double goodTrackZCPlus = -log(zeroFrac + zeroFracErr);
    double goodTrackZCMinus = -log(zeroFrac - zeroFracErr);
    // use symmetrized error
    double goodTrackErr = (std::abs(goodTrackZCPlus-goodTrackZC) + std::abs(goodTrackZCMinus-goodTrackZC))/2;

    //std::cout << "separation " << separation << " lumi " << goodTrackZC << " +/- " << goodTrackErr << std::endl;

    // scale by 1000 so the Y-axis values look reasonable
    trackLumiVal.push_back(goodTrackZC*1000);
    trackLumiErr.push_back(goodTrackErr*1000);
  }
  scanFile.close();

  // Plot and fit the results.

  TGraph *g1 = new TGraphErrors(sepVal.size(), sepVal.data(), trackLumiVal.data(), sepErr.data(), trackLumiErr.data());

  TCanvas *c1 = new TCanvas("c1", "c1", 600, 700);

  // Set up the two pads for the main plot and the residual plot below.
  TPad *p1 = new TPad("p1", "p1", 0, 0.2, 1, 1);
  TPad *p2 = new TPad("p2", "p2", 0, 0, 1, 0.2);
  p1->SetBottomMargin(0.1);
  p1->SetBorderMode(0);
  p2->SetTopMargin(1e-5);
  p2->SetBottomMargin(0.15);
  p2->SetBorderMode(0);
  p1->Draw();
  p2->Draw();
  p1->cd();

  g1->Draw("AP");
  g1->SetTitle(plotTitle);
  g1->GetXaxis()->SetTitle("Separation (mm)");
  g1->GetYaxis()->SetTitle("#mu from track zero-counting (#times 10^{-3})");
  g1->GetYaxis()->SetTitleOffset(1.4);
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
  fitWidthText << "#splitline{Fitted width:}{" << std::fixed << std::setprecision(1) << f1->GetParameter(2)*1000 << " #pm " << f1->GetParError(2)*1000 << " #kern[-0.2]{#mu}#kern[-0.1]{m}}";
  TLatex *t1 = new TLatex(0, 0, fitWidthText.str().c_str());
  t1->SetNDC();
  t1->SetX(0.65);
  t1->SetY(0.70);
  t1->SetTextSize(0.03);
  t1->Draw();

  // Make residual plot.
  std::vector<double> residualVal;
  for (int i=0; i<sepVal.size(); ++i) {
    float x = sepVal[i];
    // Ideally we should account for the uncertainty in the fit function also but this should be good enough for our purposes.
    float resid = 0.0;
    if (trackLumiErr[i] != 0)
      resid = (trackLumiVal[i] - f1->Eval(x))/trackLumiErr[i];
    residualVal.push_back(resid);
  }
  p2->cd();
  TGraph *g2 = new TGraph(sepVal.size(), sepVal.data(), residualVal.data());
  g2->Draw("AP");
  g2->SetTitle("");
  g2->GetYaxis()->SetTitle("Residual (#sigma)");
  g2->GetYaxis()->SetTitleSize(0.12);
  g2->GetYaxis()->SetTitleOffset(0.3);
  g2->GetYaxis()->SetLabelSize(0.12);
  g2->GetXaxis()->SetLabelSize(0.12);
  g2->SetMarkerStyle(kFullCircle);
  g2->SetMarkerColor(kBlue);
  g2->SetMarkerSize(1);
  TLine *l0 = new TLine(g2->GetXaxis()->GetBinLowEdge(1), 0, g2->GetXaxis()->GetBinUpEdge(g2->GetXaxis()->GetNbins()), 0);
  l0->Draw();

  c1->Print(outFileName);

  // undo the scaling by 1000 that we put in above
  return std::make_tuple(f1->GetParameter(0)/1000, f1->GetParError(0)/1000, f1->GetParameter(2), f1->GetParError(2));
}

// If called with no arguments, then do the X and Y scans for all bunches
void PlotTrackLumiVdM() {
  PlotTrackLumiVdM(("TrackLumiZC_"+fillNumber+"_X"+scanPair+".txt").c_str(), ("TrackLumi_VdM_"+fillNumber+"_X"+scanPair+"_All.png").c_str(),
		   ("Fill "+fillNumber+", Scan X"+scanPair+", all bunches").c_str());
  PlotTrackLumiVdM(("TrackLumiZC_"+fillNumber+"_Y"+scanPair+".txt").c_str(), ("TrackLumi_VdM_"+fillNumber+"_Y"+scanPair+"_All.png").c_str(),
		   ("Fill "+fillNumber+", Scan Y"+scanPair+", all bunches").c_str());
}

// If called with one argument, then the argument works as follows:
// 0 -- just plot the all-bunches plot (as above)
// 1 -- do the per-BX plots for each BXes
// 2 -- do the per-channel plots for all BXes
// 3 -- do the per-channel plots for a single specific BX
void PlotTrackLumiVdM(int whichPlot) {
  std::vector<int> bunches;
  if (fillNumber == "4954" || fillNumber == "4945") {
    bunches = {1, 41, 81, 110, 121, 161, 201, 241, 281, 591, 872,
	       912, 952, 992, 1032, 1072, 1112, 1151, 1152, 1682, 1783,
	       1823, 1863, 1903, 1943, 1983, 2023, 2063, 2654, 2655, 2694,
	       2734, 2774, 2814, 2854, 2894, 2934};
  } else if (fillNumber == "6016") {
    bunches = {1, 41, 81, 110, 121, 161, 201, 241, 281, 872,
	       912, 952, 992, 1031, 1032, 1072, 1112, 1152, 1153, 1783,
	       1823, 1863, 1903, 1943, 1983, 2023, 2063, 2064, 2654, 2694,
	       2695, 2734, 2774, 2814, 2854, 2894, 2934};
  }

  // Beam intensities as obtained from brilcalc beam via getBeamIntensities.py. In principle we should use the time-dependent intensity but since
  // a) we're only using the first scan and b) the intensities are pretty constant anyway, this shouldn't make a big difference.
  std::vector<float> beam1Intensity, beam2Intensity;
  if (fillNumber == "4954") {
    beam1Intensity = {8.4821e+10, 8.3422e+10, 8.4234e+10, 0.0, 8.4166e+10, 8.4114e+10, 8.7254e+10, 9.2722e+10, 8.0035e+10, 7.8627e+10, 8.1388e+10, 8.0154e+10, 9.4013e+10, 8.0961e+10, 8.2971e+10, 8.7840e+10, 8.5181e+10, 0.0, 8.1261e+10, 0.0000e+00, 8.0797e+10, 8.2530e+10, 9.1883e+10, 8.1674e+10, 8.1070e+10, 8.6306e+10, 8.1381e+10, 8.5866e+10, 8.8483e+10, 0.0, 8.4703e+10, 8.4744e+10, 9.3394e+10, 8.5722e+10, 8.0588e+10, 9.1071e+10, 8.9264e+10};
    beam2Intensity = {8.3129e+10, 8.2751e+10, 8.9938e+10, 0.0, 8.4354e+10, 8.1070e+10, 8.8199e+10, 8.9259e+10, 8.7807e+10, 0.0000e+00, 8.0598e+10, 9.3153e+10, 9.2925e+10, 8.4464e+10, 8.1155e+10, 8.3763e+10, 8.8342e+10, 0.0, 7.9895e+10, 8.2696e+10, 8.2977e+10, 8.8401e+10, 9.1344e+10, 9.1197e+10, 8.3538e+10, 8.1467e+10, 9.5099e+10, 9.2746e+10, 9.1364e+10, 0.0, 8.8320e+10, 8.7610e+10, 8.5767e+10, 8.2952e+10, 8.6650e+10, 9.4673e+10, 9.2896e+10};
  } else if (fillNumber == "4945") {
    beam1Intensity = {8.6636e+10, 9.3628e+10, 8.4700e+10, 0.0, 8.7151e+10, 8.1655e+10, 9.2911e+10, 8.6121e+10, 8.2967e+10, 8.0893e+10, 8.0351e+10, 8.0912e+10, 8.1689e+10, 8.5672e+10, 8.5710e+10, 9.3638e+10, 8.6560e+10, 0.0, 8.7584e+10, 0.0000e+00, 8.6174e+10, 9.2788e+10, 8.7202e+10, 8.5001e+10, 8.3384e+10, 9.4195e+10, 8.4360e+10, 8.2543e+10, 8.5007e+10, 0.0, 9.2695e+10, 8.3441e+10, 8.6949e+10, 7.9753e+10, 8.0453e+10, 8.2468e+10, 8.1625e+10};
    beam2Intensity = {8.3183e+10, 8.7209e+10, 8.5625e+10, 0.0, 8.9035e+10, 8.4098e+10, 9.7680e+10, 8.2924e+10, 8.2968e+10, 0.0000e+00, 8.0406e+10, 9.5034e+10, 8.5700e+10, 8.0820e+10, 8.1693e+10, 8.9988e+10, 8.7191e+10, 0.0, 8.3523e+10, 8.5938e+10, 8.4521e+10, 9.2949e+10, 8.6067e+10, 8.2300e+10, 8.1893e+10, 8.9461e+10, 8.8270e+10, 8.4715e+10, 8.4349e+10, 0.0, 8.7887e+10, 8.5696e+10, 9.1298e+10, 9.0524e+10, 9.1492e+10, 8.7418e+10, 8.2189e+10};
  } else if (fillNumber == "6016") {
    beam1Intensity = {8.6622e+10, 8.8944e+10, 9.0191e+10, 0.0, 8.5749e+10, 8.2808e+10, 8.9887e+10, 8.9563e+10, 8.8437e+10, 8.3821e+10, 9.1255e+10, 8.9837e+10, 8.7577e+10, 0.0, 7.9491e+10, 8.8273e+10, 9.0202e+10, 8.7774e+10, 0.0, 8.2037e+10, 8.9936e+10, 9.0976e+10, 8.5698e+10, 8.4210e+10, 8.7275e+10, 9.4114e+10, 8.5834e+10, 0.0, 8.5183e+10, 8.8927e+10, 0.0, 8.6262e+10, 8.6495e+10, 8.5669e+10, 8.9498e+10, 8.8483e+10, 8.8214e+10};
    beam2Intensity = {8.3864e+10, 8.7002e+10, 9.0804e+10, 0.0, 8.7052e+10, 8.1125e+10, 8.7541e+10, 9.0651e+10, 8.7450e+10, 8.4690e+10, 9.0548e+10, 9.3033e+10, 8.6274e+10, 0.0, 8.3622e+10, 8.9818e+10, 8.9241e+10, 9.1710e+10, 0.0, 8.4978e+10, 8.5863e+10, 9.0901e+10, 8.9705e+10, 8.6210e+10, 9.1836e+10, 8.8804e+10, 8.7562e+10, 0.0, 8.5498e+10, 9.3430e+10, 0.0, 8.7909e+10, 8.9778e+10, 8.6910e+10, 8.9558e+10, 8.8471e+10, 8.7141e+10};
  } else {
      std::cerr << "Error: don't know bunch intensities for fill " << fillNumber << "; please add them to the script" << std::endl;
    return;
  }

  assert(bunches.size() == beam1Intensity.size());
  assert(bunches.size() == beam2Intensity.size());

  if (whichPlot == 0) {
    PlotTrackLumiVdM();
  } else {
    // Iterate either over bunches (in case 1) or channels (in cases 2 or 3).

    std::vector<float> xVals, xErrs; // either BX (1) or channel (2 or 3)
    std::vector<float> capSigmaX, capSigmaY, capSigmaXErr, capSigmaYErr;
    std::vector<float> sigmaVis, sigmaVisErr;

    // Set up various parameters depending on whether we're plotting by bunch or by channel.
    int nmax;
    float overallBunchProduct;
    std::string xAxisTitle, capSigmaPlotTitle, capSigmaFileName, sigmaVisPlotTitle, sigmaVisFileName;
    if (whichPlot == 1) {
      nmax = bunches.size();
      xAxisTitle = "Bunch ID";
      capSigmaPlotTitle = "Beam overlap size per bunch, fill "+fillNumber+", scan pair "+scanPair+", all channels";
      capSigmaFileName = "TrackLumi_VdM_"+fillNumber+"_"+scanPair+"_CapSigmas_BX.png";
      sigmaVisPlotTitle = "Visible cross section per bunch, fill "+fillNumber+", scan pair "+scanPair+", all channels";
      sigmaVisFileName = "TrackLumi_VdM_"+fillNumber+"_"+scanPair+"_SigmaVis_BX.png";
    } else if (whichPlot == 2) {
      nmax = nPixelChannels;
      xAxisTitle = "Readout channel number";
      capSigmaPlotTitle = "Beam overlap size per channel, fill "+fillNumber+", scan pair "+scanPair+", all bunches";
      capSigmaFileName = "TrackLumi_VdM_"+fillNumber+"_"+scanPair+"_CapSigmas_Chan.png";
      sigmaVisPlotTitle = "Visible cross section per channel, fill "+fillNumber+", scan pair "+scanPair+", all bunches";
      sigmaVisFileName = "TrackLumi_VdM_"+fillNumber+"_"+scanPair+"_SigmaVis_Chan.png";
      
      int nSummedBunches = 0;
      float bunchProductSum = 0.0;
      for (int i=0; i<bunches.size(); ++i) {
	if (beam1Intensity[i] > 1e8 && beam2Intensity[i] > 1e8) {
	  nSummedBunches++;
	  bunchProductSum += beam1Intensity[i]*beam2Intensity[i];
	}
      }
      overallBunchProduct = bunchProductSum/nSummedBunches;
    } else {
      nmax = nPixelChannels;
      xAxisTitle = "Readout channel number";
      capSigmaPlotTitle = "Beam overlap size per channel, fill "+fillNumber+", scan pair "+scanPair+", BX 81";
      capSigmaFileName = "TrackLumi_VdM_"+fillNumber+"_"+scanPair+"_CapSigmas_Chan_BX81.png";
      sigmaVisPlotTitle = "Visible cross section per channel, fill "+fillNumber+", scan pair "+scanPair+", BX 81";
      sigmaVisFileName = "TrackLumi_VdM_"+fillNumber+"_"+scanPair+"_SigmaVis_Chan_BX81.png";

      std::vector<int>::iterator targetBXIt = std::find(bunches.begin(), bunches.end(), 81);
      int targetBXIndex = std::distance(bunches.begin(), targetBXIt);
      //std::cout << "target index is " << targetBXIndex;
      overallBunchProduct = beam1Intensity[targetBXIndex]*beam2Intensity[targetBXIndex];
    }

    for (unsigned int i=0; i<nmax; ++i) {
      std::string fileStringX, fileStringY, plotStringX, plotStringY, titleStringX, titleStringY;
      int targetChan = -1;
      bool useThisPoint = true; // whether to include this point in the summary plots
      float beamIntensityProduct;

      // Set up the file input and output, and the correct X value for the current bunch/channel.
      if (whichPlot == 1) {
	std::stringstream bxStringPad;
	bxStringPad << std::setfill('0') << std::setw(4) << bunches[i];
	std::stringstream bxString;
	bxString << bunches[i];
	
	fileStringX = "TrackLumi_"+fillNumber+"_X"+scanPair+"_BX/TrackLumiZC"+bxStringPad.str()+".txt";
	plotStringX = "TrackLumi_VdM_"+fillNumber+"_X"+scanPair+"_"+bxStringPad.str()+".png";
	titleStringX = "Fill "+fillNumber+", Scan X"+scanPair+", bunch "+bxString.str();
	
	fileStringY = "TrackLumi_"+fillNumber+"_Y"+scanPair+"_BX/TrackLumiZC"+bxStringPad.str()+".txt";
	plotStringY = "TrackLumi_VdM_"+fillNumber+"_Y"+scanPair+"_"+bxStringPad.str()+".png";
	titleStringY = "Fill "+fillNumber+", Scan Y"+scanPair+", bunch "+bxString.str();

	// For the summary plots, don't include noncolliding bunches.
	if (beam1Intensity[i] > 1e8 && beam2Intensity[i] > 1e8) {
	  xVals.push_back(bunches[i]);
	  xErrs.push_back(0);
	} else {
	  useThisPoint = false;
	}
	beamIntensityProduct = beam1Intensity[i]*beam2Intensity[i];
      } else if (whichPlot == 2) {
	targetChan = i;
	int readoutChanNum = convertToReadoutChannel(fillNumber, i);
	std::stringstream chanString;
	chanString << readoutChanNum;
	fileStringX = "TrackLumiZC_"+fillNumber+"_X"+scanPair+".txt";
	plotStringX = "TrackLumi_VdM_"+fillNumber+"_X"+scanPair+"_Ch"+chanString.str()+".png";
	titleStringX = "Fill "+fillNumber+", Scan X"+scanPair+", all BX, channel "+chanString.str();

	fileStringY = "TrackLumiZC_"+fillNumber+"_Y"+scanPair+".txt";
	plotStringY = "TrackLumi_VdM_"+fillNumber+"_Y"+scanPair+"_Ch"+chanString.str()+".png";
	titleStringY = "Fill "+fillNumber+", Scan Y"+scanPair+", all BX, channel "+chanString.str();

	xVals.push_back(readoutChanNum);
	xErrs.push_back(0);
	beamIntensityProduct = overallBunchProduct;
      } else if (whichPlot == 3) {
	// Currently set to always use BX81
	targetChan = i;
	int readoutChanNum = convertToReadoutChannel(fillNumber, i);
	std::stringstream chanString;
	chanString << readoutChanNum;

	fileStringX = "TrackLumi_"+fillNumber+"_X"+scanPair+"_BX/TrackLumiZC0081.txt";
	plotStringX = "TrackLumi_VdM_"+fillNumber+"_X"+scanPair+"_Ch"+chanString.str()+"_BX81.png";
	titleStringX = "Fill "+fillNumber+", Scan X"+scanPair+", BX 81, channel "+chanString.str();

	fileStringY = "TrackLumi_"+fillNumber+"_Y"+scanPair+"_BX/TrackLumiZC0081.txt";
	plotStringY = "TrackLumi_VdM_"+fillNumber+"_Y"+scanPair+"_Ch"+chanString.str()+"_BX81.png";
	titleStringY = "Fill "+fillNumber+", Scan Y"+scanPair+", BX 81, channel "+chanString.str();

	xVals.push_back(readoutChanNum);
	xErrs.push_back(0);
	beamIntensityProduct = overallBunchProduct;
      }
       
      // Do the actual calls to read and fit the data.
      std::tuple<float, float, float, float> resultsX = PlotTrackLumiVdM(fileStringX.c_str(), plotStringX.c_str(), titleStringX.c_str(), targetChan);
      std::tuple<float, float, float, float> resultsY = PlotTrackLumiVdM(fileStringY.c_str(), plotStringY.c_str(), titleStringY.c_str(), targetChan);

      if (useThisPoint) {
	// Process the results and store them in the summary arrays.
	capSigmaX.push_back(std::get<2>(resultsX)*1000);
	capSigmaY.push_back(std::get<2>(resultsY)*1000);
	capSigmaXErr.push_back(std::get<3>(resultsX)*1000);
	capSigmaYErr.push_back(std::get<3>(resultsY)*1000);
	
	// Calculate sigmavis.
	float r0 = (std::get<0>(resultsX) + std::get<0>(resultsY))/2;
	float r0err = (std::get<1>(resultsX)*std::get<1>(resultsX) + std::get<1>(resultsY)*std::get<1>(resultsY))/2;
	// the 1e6 is to convert the capsigmas from mm
	float thisSigmaVis = 2*M_PI*std::get<2>(resultsX)*std::get<2>(resultsY)*r0/(beamIntensityProduct*1e6);
	float r0RelErr = r0err/r0;
	float capSigmaXRelErr = std::get<3>(resultsX)/std::get<2>(resultsX);
	float capSigmaYRelErr = std::get<3>(resultsY)/std::get<2>(resultsY);
	float thisSigmaVisErr = (r0RelErr+capSigmaXRelErr+capSigmaYRelErr)*thisSigmaVis;
	// *1e34 to convert to microbarns
	sigmaVis.push_back(thisSigmaVis*1e34);
	sigmaVisErr.push_back(thisSigmaVisErr*1e34);
      }
    } 

    for (int i=0; i<xVals.size(); ++i) {
      std::cout << xVals[i] << " SigmaX=" << capSigmaX[i] << " +/- " << capSigmaXErr[i] << " SigmaY=" << capSigmaY[i]
		<< " +/- " << capSigmaYErr[i] << " sigmaVis=" << sigmaVis[i] << " +/- " << sigmaVisErr[i] << std::endl;
    }

    // Plot the resulting capsigmas for all bunches/channels
    TGraph *gx = new TGraphErrors(xVals.size(), xVals.data(), capSigmaX.data(), xErrs.data(), capSigmaXErr.data());
    TGraph *gy = new TGraphErrors(xVals.size(), xVals.data(), capSigmaY.data(), xErrs.data(), capSigmaYErr.data());
    TCanvas *cs = new TCanvas("cs", "cs", 1000, 600);
    gx->Draw("AP");
    gx->SetTitle(capSigmaPlotTitle.c_str());
    gx->GetXaxis()->SetTitle(xAxisTitle.c_str());
    gx->GetYaxis()->SetTitle("Beam overlap width (#mum)");
    gx->SetMarkerStyle(kFullCircle);
    gx->SetMarkerColor(kBlue);
    gx->SetMarkerSize(1);
    gy->Draw("P same");
    gy->SetMarkerStyle(kFullCircle);
    gy->SetMarkerColor(kRed);
    gy->SetMarkerSize(1);

    // Get mininum and maximum values.
    float ymin = std::min(*std::min_element(capSigmaX.begin(), capSigmaX.end()), *std::min_element(capSigmaY.begin(), capSigmaY.end()));
    float ymax = std::max(*std::max_element(capSigmaX.begin(), capSigmaX.end()), *std::max_element(capSigmaY.begin(), capSigmaY.end()));
    gx->SetMinimum(ymin - 5);
    gx->SetMaximum(ymax + 5);

    TLegend *l = new TLegend(0.65, 0.78, 0.9, 0.88);
    l->AddEntry(gx, "#Sigma_{X}", "P");
    l->AddEntry(gy, "#Sigma_{Y}", "P");
    l->SetBorderSize(0);
    l->SetFillColor(0);
    l->Draw();

    cs->Print(capSigmaFileName.c_str());

    // And finally the sigmavis values.
    TGraph *gsv = new TGraphErrors(xVals.size(), xVals.data(), sigmaVis.data(), xErrs.data(), sigmaVisErr.data());
    TCanvas *csv = new TCanvas("csv", "csv", 1000, 600);
    gsv->Draw("AP");
    gsv->SetTitle(sigmaVisPlotTitle.c_str());
    gsv->GetXaxis()->SetTitle(xAxisTitle.c_str());
    gsv->GetYaxis()->SetTitle("#sigma_{vis} (#mub)");
    gsv->SetMarkerStyle(kFullCircle);
    gsv->SetMarkerColor(kBlue);
    gsv->SetMarkerSize(1);

    TF1 *fsv = new TF1("fsv", "pol0");
    fsv->SetLineColor(kRed);
    fsv->SetLineWidth(2);
    gsv->Fit(fsv);

    std::stringstream fitSigmaVisText;
    fitSigmaVisText << "Average: " << std::fixed << std::setprecision(1) << fsv->GetParameter(0) << " #pm " << fsv->GetParError(0) << " #kern[-0.2]{#mu}#kern[-0.1]{b}";
    TLatex *tsv = new TLatex(0, 0, fitSigmaVisText.str().c_str());
    tsv->SetNDC();
    tsv->SetX(0.75);
    tsv->SetY(0.80);
    tsv->SetTextSize(0.03);
    tsv->Draw();

    csv->Print(sigmaVisFileName.c_str());
  }
}
