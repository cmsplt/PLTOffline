////////////////////////////////////////////////////////////////////
//
// StabilityLinearity.C -- do the rates comparison between the PLT track-counting lumi
// and hfoc/pltzero over a large number of fills to evaluate the long-term stability
// and linearity.
//
// The basic code for reading the files is in PlotRatesCompare.C so have a look at there
// to understand what's going on better. See the README for more docuemntation.
//
// Paul Lujan, September 22 2020
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <time.h>
#include "TROOT.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLegend.h"

const std::vector<std::string> fills = {"4958", "5013", "5052", "5109", "5183", "5261", "5282", "5340", "5394", "5451"};
// Normally we can use the number of bunches automatically determined by the script itself. However, in some
// cases this measurement may not be accurate, so we can set the correct value here to override the automatic
// determination.
const std::map<std::string, int> nbx({std::make_pair("5451", 2208)});
const int nPixelChannels = 13;
// whether or not to attempt to automatically fix channel dropouts; see README
const bool attemptChannelFix = true;
// If this is set to a positive value, it will use this scale factor for all fills. Otherwise, it will
// dynamically calculate the scale factor on a per-fill basis.
//const float constScaleFactor = 92.0;
const float constScaleFactor = -1;

float findLumi(const std::vector<double>& targetTimestamps, const std::vector<double>& targetLumis, int timestamp) {
  // this really shouldn't happen
  if (timestamp < targetTimestamps[0])
    return targetLumis[0];
  for (unsigned int i=1; i<targetTimestamps.size(); ++i) {
    if (timestamp >= targetTimestamps[i-1] && timestamp < targetTimestamps[i])
      return targetLumis[i-1];
  }
  return targetLumis.back();
}

int convertTimestamp(int pltTime, int referenceTime) {
  time_t reftt = referenceTime;
  struct tm *tmp = gmtime(&reftt);
  // Get midnight of this day.
  tmp->tm_hour = 0;
  tmp->tm_min = 0;
  tmp->tm_sec = 0;
  // Get the timestamp corresponding to that.
  time_t dayMidnight = mktime(tmp);
  // I still need to do DST. not sure why!
  return((pltTime/1000)+3600+dayMidnight);
}

void readBrilcalcFile(std::string fileName, std::vector<double>& timestamps, std::vector<double>& deliveredLumi) {
  std::ifstream csvFile(fileName.c_str());
  if (!csvFile.is_open()) {
    std::cerr << "ERROR: cannot open csv file: " << fileName << std::endl;
    return;
  }
    
  // Go through the lines of the file.
  int lsnum = 0;
  std::string line;
  while (1) {
    std::getline(csvFile, line);
    if (csvFile.eof()) break;
    if (line.empty()) continue; // skip blank lines
    if (line.at(0) == '#') continue; // skip comment lines

    // Break into fields
    std::stringstream ss(line);
    std::string field;
    std::vector<std::string> fields;

    while (std::getline(ss, field, ','))
      fields.push_back(field);

    if (fields.size() != 9) {
      std::cout << "Malformed line in csv file: " << line << std::endl;
      continue;
    }

    std::stringstream timestampString(fields[2]);
    int timestamp;
    timestampString >> timestamp;

    std::stringstream lumiDelString(fields[5]);
    float lumiDel;
    lumiDelString >> lumiDel;
    //std::cout << timestamp << " " <<lumiDel << std::endl;

    timestamps.push_back(timestamp);
    deliveredLumi.push_back(lumiDel/1000.0);
  }
}

int readSingleFill(const std::string fillNumber, std::vector<double>& hfoc_ratios, std::vector<double>& pltz_ratios,
		   std::vector<double>& hfoc_linearity, std::vector<double>& pltz_linearity, int i) {
  // Read brilcalc files.
  std::vector<double> hfoc_timestamps;
  std::vector<double> hfoc_lumis;
  std::string brilcalc_name_hfoc = "hfoc_"+fillNumber+".csv";
  readBrilcalcFile(brilcalc_name_hfoc, hfoc_timestamps, hfoc_lumis);
  std::vector<double> pltz_timestamps;
  std::vector<double> pltz_lumis;
  std::string pltz_brilcalc_name = "pltzero_"+fillNumber+".csv";
  readBrilcalcFile(pltz_brilcalc_name, pltz_timestamps, pltz_lumis);

  // Read input file.
  std::vector<double> trackTimestamps;
  std::vector<double> trackLumiAll;
  std::vector<double> trackLumiGood;
  std::vector<double> trackLumiErr;

  std::string tracklumi_name = "TrackLumiZC_"+fillNumber+".txt";
  FILE *rfile = fopen(tracklumi_name.c_str(), "r");
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file!" << std::endl;
    return -1;
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig, nFilledTrig;
  float tracksAll, tracksGood, nEmpty, nFull;
  std::vector<bool> channelStillGood(nPixelChannels, true);
  std::vector<float> channelRatios(nPixelChannels, 0);
  float totalNormalization = 1;
  bool ratiosFilled = false;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  if (nbx.count(fillNumber) > 0) {
    std::cout << "Overriding nBX read " << nBunches << " with nBX specified " << nbx.at(fillNumber) << std::endl;
    nBunches = nbx.at(fillNumber);
  }
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d %f %f %d %f %f", &tBegin, &tEnd, &nTrig, &tracksAll, &tracksGood,
	   &nFilledTrig, &nEmpty, &nFull);
    int channelTracks[nPixelChannels];
    int sumChannelTracks = 0;
    for (int j=0; j<nPixelChannels; ++j) {
      fscanf(rfile, "%d", &(channelTracks[j]));
      sumChannelTracks += channelTracks[j];
    }

    // If this is the first data point, store the channel ratios so we can use them again.
    if (!ratiosFilled) {
      for (int j=0; j<nPixelChannels; ++j) {
	channelRatios[j] = (float)channelTracks[j]/sumChannelTracks;
	//std::cout << channelRatios[j] << std::endl;
      }
      ratiosFilled = true;
    }

    // Check to see if any channel has gone bad.
    for (int j=0; j<nPixelChannels; ++j) {
      if (!channelStillGood[j]) continue;
      float thisChannelRatio = (float)channelTracks[j]/sumChannelTracks;
      if (std::abs(thisChannelRatio/channelRatios[j] - 1) > 0.1) {
	if (attemptChannelFix)
	  std::cout << "Channel " << j << " gone bad at " << tBegin << std::endl;
	channelStillGood[j] = false;
	totalNormalization -= channelRatios[j];
      }
    }

    // Recalculate the average based on the good channels.
    int sumGoodChannels = 0;
    int nGoodChannels = 0;
    for (int j=0; j<nPixelChannels; ++j) {
      if (channelStillGood[j]) {
	sumGoodChannels += channelTracks[j];
      }
    }

    if (attemptChannelFix)
      nFull = float(sumGoodChannels)/(nPixelChannels*totalNormalization);

    // Process the data.
    int convertedBeginning = convertTimestamp(tBegin, hfoc_timestamps[0]);
    int convertedEnd = convertTimestamp(tEnd, hfoc_timestamps[0]);
    float convertedMiddle = (float(convertedBeginning)+float(convertedEnd))/2.0;

    trackTimestamps.push_back(convertedMiddle);
    trackLumiAll.push_back(-log(1.0-(double)tracksAll/nTrig));
    double goodTrackZC = -log(1.0-(double)nFull/nFilledTrig);
    //std::cout << goodTrackZC << std::endl;
    trackLumiGood.push_back(goodTrackZC);
    trackLumiErr.push_back(0); // not implemented yet
  }
  fclose(rfile);

  // Determine the scale factor we need.
  float scaleTarget = findLumi(pltz_timestamps, pltz_lumis, trackTimestamps[30]);
  float fillScaleFactor = scaleTarget/trackLumiGood[30];

  float scaleFactor;
  if (constScaleFactor > 0) {
    scaleFactor = constScaleFactor;
  } else {
    scaleFactor = fillScaleFactor;
    std::cout << "scale factor is " << scaleFactor << std::endl;
  }
  for (std::vector<double>::iterator it = trackLumiGood.begin(); it != trackLumiGood.end(); ++it) {
    *it *= scaleFactor;
  }

  // These plots store the ratios for this fill, so we can fit them and get the slopes for this fill.  We only
  // store the values within 0.05 of 1, so as to avoid periods with bad data. Note that because of this
  // criterion, we have to use the per-fill scale factor for this, even if we're using the constant scale
  // factor overall. Confusing, I know.

  std::vector<double> ratio_hfoc_clean;
  std::vector<double> ratio_pltz_clean;
  std::vector<double> hfoc_sbil_clean;
  std::vector<double> pltz_sbil_clean;

  // Calculate the ratios and append them to the overall plots.
  for (unsigned int i=0; i<trackTimestamps.size(); ++i) {
    float hfoc_lumi = findLumi(hfoc_timestamps, hfoc_lumis, trackTimestamps[i]);
    hfoc_ratios.push_back(trackLumiGood[i]/hfoc_lumi);
    float pltz_lumi = findLumi(pltz_timestamps, pltz_lumis, trackTimestamps[i]);
    pltz_ratios.push_back(trackLumiGood[i]/pltz_lumi);

    // Calculate the "clean" ratios (which also may have a different scale factor!) and append them to those
    // vectors if they pass.
    double clean_ratio_hfoc = (trackLumiGood[i]/hfoc_lumi)*(fillScaleFactor/scaleFactor);
    if (std::abs(clean_ratio_hfoc-1) < 0.05) {
      ratio_hfoc_clean.push_back(clean_ratio_hfoc);
      hfoc_sbil_clean.push_back(hfoc_lumi*1000/nBunches);
    }
    
    double clean_ratio_pltz = (trackLumiGood[i]/pltz_lumi)*(fillScaleFactor/scaleFactor);
    if (std::abs(clean_ratio_pltz-1) < 0.05) {
      ratio_pltz_clean.push_back(clean_ratio_pltz);
      pltz_sbil_clean.push_back(pltz_lumi*1000/nBunches);
    }

  }

  // Now fit the ratios vs. SBIL from this fill to extract the overall slope and append it to those vectors.
  TGraph *g_rlp = new TGraph(pltz_sbil_clean.size(), &(pltz_sbil_clean[0]), &(ratio_pltz_clean[0]));
  TGraph *g_rlh = new TGraph(hfoc_sbil_clean.size(), &(hfoc_sbil_clean[0]), &(ratio_hfoc_clean[0]));
  TF1 *f_rp = new TF1("f_rp", "pol1");
  g_rlp->Fit(f_rp);
  TF1 *f_rh = new TF1("f_rh", "pol1");
  g_rlh->Fit(f_rh);
  
  // don't forget to convert to %
  hfoc_linearity.push_back(f_rh->GetParameter(1)*100);
  pltz_linearity.push_back(f_rp->GetParameter(1)*100);

  // hooray, we're done
  return 0;
}

void StabilityLinearity(void) {
  gROOT->SetStyle("Plain");
  gStyle->SetPadTopMargin(0.07);
  gStyle->SetPadLeftMargin(0.08);
  gStyle->SetPadRightMargin(0.03);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleX(0.1);
  gStyle->SetTitleY(1.0);
  gStyle->SetTitleH(0.06);
  gStyle->SetTitleW(0.7);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetLegendBorderSize(0);

  // Track/HFOC and track/PLTZ ratios for each lumisection
  std::vector<double> hfoc_ratios;
  std::vector<double> pltz_ratios;
  // Track/HFOC and track/PLTZ linearities for each fill
  std::vector<double> hfoc_linearity;
  std::vector<double> pltz_linearity;

  for (unsigned int i=0; i<fills.size(); ++i) {
    std::cout << "Reading data for fill " << fills[i] << std::endl;
    readSingleFill(fills[i], hfoc_ratios, pltz_ratios, hfoc_linearity, pltz_linearity, i);
  }

  // Make the x values.
  std::vector<double> xvals_rat(hfoc_ratios.size(), 0);
  std::vector<double> xvals_lin(hfoc_linearity.size(), 0);
  for (unsigned int i=0; i < hfoc_ratios.size(); ++i)
    xvals_rat[i] = i;
  for (unsigned int i=0; i < hfoc_linearity.size(); ++i)
    xvals_lin[i] = i;

  // Plot it all.
  TGraph *g_rtp = new TGraph(xvals_rat.size(), &(xvals_rat[0]), &(pltz_ratios[0]));
  TGraph *g_rth = new TGraph(xvals_rat.size(), &(xvals_rat[0]), &(hfoc_ratios[0]));
  
  TCanvas *c1 = new TCanvas("c1", "c1", 1200, 600);
  g_rtp->Draw("AP");
  g_rtp->SetTitle("Luminosity ratios vs. time");
  g_rtp->GetXaxis()->SetTitle("LS number (arbitrary)");
  g_rtp->GetYaxis()->SetTitle("Ratio");
  g_rtp->GetYaxis()->SetTitleOffset(1.0);
  g_rtp->SetMarkerStyle(kFullCircle);
  g_rtp->SetMarkerColor(kBlue);
  g_rtp->SetLineColor(kBlue);
  g_rtp->SetMarkerSize(1);
  if (constScaleFactor > 0) {
    g_rtp->SetMaximum(1.3);
    g_rtp->SetMinimum(0.5);
  }
  else {
    g_rtp->SetMaximum(1.1);
    g_rtp->SetMinimum(0.85);
  }

  g_rth->Draw("P same");
  g_rth->SetMarkerStyle(kFullCircle);
  g_rth->SetMarkerColor(kRed);
  g_rth->SetMarkerSize(1);
  TLegend *l1 = new TLegend(0.5, 0.73, 0.8, 0.88);
  l1->AddEntry(g_rtp, "Track lumi/PLTZ", "LP");
  l1->AddEntry(g_rth, "Track lumi/HFOC", "LP");
  l1->SetBorderSize(0);
  l1->SetFillColor(0);
  l1->Draw();

  // And similarly for the long-term linearity plots.
  TGraph *g_ltp = new TGraph(xvals_lin.size(), &(xvals_lin[0]), &(pltz_linearity[0]));
  TGraph *g_lth = new TGraph(xvals_lin.size(), &(xvals_lin[0]), &(hfoc_linearity[0]));
  
  TCanvas *c2 = new TCanvas("c2", "c2", 1200, 600);
  g_ltp->Draw("AP");
  g_ltp->SetTitle("Relative slopes per fill");
  g_ltp->GetXaxis()->SetTitle("Fill number");
  g_ltp->GetYaxis()->SetTitle("Linearity slope [%/(Hz/#mub)]");
  g_ltp->GetYaxis()->SetTitleOffset(1.0);
  g_ltp->SetMarkerStyle(kFullCircle);
  g_ltp->SetMarkerColor(kBlue);
  g_ltp->SetLineColor(kBlue);
  g_ltp->SetMarkerSize(1);

  g_lth->Draw("P same");
  g_lth->SetMarkerStyle(kFullCircle);
  g_lth->SetMarkerColor(kRed);
  g_lth->SetMarkerSize(1);
  TLegend *l2 = new TLegend(0.65, 0.76, 0.95, 0.91);
  l2->AddEntry(g_ltp, "Track lumi/PLTZ slope", "LP");
  l2->AddEntry(g_lth, "Track lumi/HFOC slope", "LP");
  l2->SetBorderSize(0);
  l2->SetFillColor(0);
  l2->Draw();

  for (unsigned int i=0; i<fills.size(); ++i)    
    g_ltp->GetXaxis()->SetBinLabel(g_ltp->GetXaxis()->FindBin(i), fills[i].c_str());
  g_ltp->GetXaxis()->LabelsOption("h");

  std::string outFile = "RatioVsTimeLongterm";
  if (constScaleFactor > 0)
    outFile += "_constscale";
  if (attemptChannelFix)
    outFile += "_fix";
  outFile += ".png";
  c1->Print(outFile.c_str());

  outFile = "LinearityVsFill";
  if (attemptChannelFix)
    outFile += "_fix";
  outFile += ".png";
  c2->Print(outFile.c_str());
}
