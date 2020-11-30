////////////////////////////////////////////////////////////////////
//
// PlotTrackLumiFill -- given the data for a single fill, compare the
// HFOC, PLT fast-or, and PLT track-counting lumis together.
// Based (more or less) on PlotRatesZCTime. See the README for more
// documentation.
//
// Paul Lujan, September 11 2020
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
// Normally we can use the number of bunches automatically determined by the script itself. However, in some
// cases this measurement may not be accurate, so we can set the correct value here to override the automatic
// determination.
const std::map<std::string, int> nbx({std::make_pair("5451", 2208)});
// whether or not to attempt to automatically fix channel dropouts; see README
const bool attemptChannelFix = true;

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

void PlotTrackLumiFill(int fillNumber_i) {
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
  std::stringstream ss;
  ss << fillNumber_i;
  std::string fillNumber = ss.str();

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
    return;
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

    // The following lines attempt to do the automatic channel dropout detection. Basically, this stores all
    // of the ratios of the channels to the total at the start of the fill, and if that ratio changes by more
    // than 10%, we flag the channel bad and exclude it from the total for the remainder of the fill. Note
    // that the calculations are always done, but we only replace the final value with the recalculated value
    // if attemptChannelFix is set above.

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

    // Recalculate the average based on the good channels. Note -- the fully correct thing to do would be to
    // average after calculating the mu values, rather than before, but when I quickly checked the results
    // seem to be nearly indistinguishable, so I think it's OK to do it the simple way.
    int sumGoodChannels = 0;
    for (int j=0; j<nPixelChannels; ++j) {
      if (channelStillGood[j]) {
	sumGoodChannels += channelTracks[j];
      }
    }

    // Use the recalculated value if the flag is set.
    if (attemptChannelFix)
      nFull = float(sumGoodChannels)/(nPixelChannels*totalNormalization);

    // Process the timestamps and store the final data.
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
  float scaleFactor = scaleTarget/trackLumiGood[30];
  std::cout << "scale factor is " << scaleFactor << std::endl;
  for (std::vector<double>::iterator it = trackLumiGood.begin(); it != trackLumiGood.end(); ++it) {
    *it *= scaleFactor;
  }

  // Make ratio plots both vs. time and vs. instantaneous luminosity. For the latter, only use points in the
  // range 0.95-1.05.
  std::vector<double> ratio_hfoc;
  std::vector<double> ratio_pltz;

  std::vector<double> ratio_hfoc_clean;
  std::vector<double> ratio_pltz_clean;
  std::vector<double> hfoc_sbil_clean;
  std::vector<double> pltz_sbil_clean;

  for (unsigned int i=0; i<trackTimestamps.size(); ++i) {
    float hfoc_lumi = findLumi(hfoc_timestamps, hfoc_lumis, trackTimestamps[i]);
    double this_ratio_hfoc = trackLumiGood[i]/hfoc_lumi;
    ratio_hfoc.push_back(this_ratio_hfoc);
    if (std::abs(this_ratio_hfoc-1) < 0.05) {
      ratio_hfoc_clean.push_back(this_ratio_hfoc);
      hfoc_sbil_clean.push_back(hfoc_lumi*1000/nBunches);
    }

    float pltz_lumi = findLumi(pltz_timestamps, pltz_lumis, trackTimestamps[i]);
    double this_ratio_pltz = trackLumiGood[i]/pltz_lumi;
    ratio_pltz.push_back(this_ratio_pltz);
    if (std::abs(this_ratio_pltz-1) < 0.05) {
      ratio_pltz_clean.push_back(this_ratio_pltz);
      pltz_sbil_clean.push_back(pltz_lumi*1000/nBunches);
    }
  }

  // Plot it all.

  // graphs of luminosity for hfoc, pltzero, tracklumi
  TGraph *g_lhf = new TGraph(hfoc_lumis.size(), &(hfoc_timestamps[0]), &(hfoc_lumis[0]));
  TGraph *g_lpz = new TGraph(hfoc_lumis.size(), &(hfoc_timestamps[0]), &(hfoc_lumis[0]));
  TGraph *g_ltr = new TGraph(trackTimestamps.size(), &(trackTimestamps[0]), &(trackLumiGood[0]));
  // graphs of ratios vs. time for pltzero and hfoc
  TGraph *g_rtp = new TGraph(nsteps, &(trackTimestamps[0]), &(ratio_pltz[0]));
  TGraph *g_rth = new TGraph(nsteps, &(trackTimestamps[0]), &(ratio_hfoc[0]));
  // graphs of ratios vs. SBIL for pltzero and hfoc
  TGraph *g_rlp = new TGraph(pltz_sbil_clean.size(), &(pltz_sbil_clean[0]), &(ratio_pltz_clean[0]));
  TGraph *g_rlh = new TGraph(hfoc_sbil_clean.size(), &(hfoc_sbil_clean[0]), &(ratio_hfoc_clean[0]));

  TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
  g_lhf->Draw("AP");
  std::string titleString = "Track zero-counting rate vs. time, fill "+fillNumber;
  g_lhf->SetTitle(titleString.c_str());
  g_lhf->GetXaxis()->SetTitle("CERN time");
  g_lhf->GetYaxis()->SetTitle("Luminosity (Hz/nb)");
  g_lhf->GetYaxis()->SetTitleOffset(1.2);
  g_lhf->SetMarkerStyle(kFullCircle);
  g_lhf->SetMarkerColor(kRed);
  g_lhf->SetMarkerSize(1);
  g_lhf->GetXaxis()->SetTimeDisplay(1);
  g_lhf->GetXaxis()->SetTimeFormat("%H:%M");
  g_lhf->GetXaxis()->SetTimeOffset(0,"gmt");

  g_lpz->Draw("P same");
  g_lpz->SetMarkerStyle(kFullCircle);
  g_lpz->SetMarkerColor(kBlue);
  g_lpz->SetMarkerSize(1);

  g_ltr->Draw("P same");
  g_ltr->SetMarkerStyle(kFullCircle);
  g_ltr->SetMarkerColor(kGreen);
  g_ltr->SetMarkerSize(1);

  TLegend *l = new TLegend(0.5, 0.73, 0.8, 0.88);
  l->AddEntry(g_lhf, "HFOC lumi", "LP");
  l->AddEntry(g_lpz, "PLTZ lumi", "LP");
  l->AddEntry(g_ltr, "Track lumi, zero counting", "LP");
  l->SetBorderSize(0);
  l->SetFillColor(0);
  l->Draw();

  TCanvas *c2 = new TCanvas("c2", "c2", 600, 600);
  g_rtp->Draw("AP");
  titleString = "Luminosity ratios vs. time, fill "+fillNumber;
  g_rtp->SetTitle(titleString.c_str());
  g_rtp->GetXaxis()->SetTitle("CERN time");
  g_rtp->GetYaxis()->SetTitle("Ratio");
  g_rtp->GetYaxis()->SetTitleOffset(1.5);
  g_rtp->SetMarkerStyle(kFullCircle);
  g_rtp->SetMarkerColor(kBlue);
  g_rtp->SetLineColor(kBlue);
  g_rtp->SetMarkerSize(1);
  g_rtp->SetMaximum(1.05);
  g_rtp->SetMinimum(0.95);
  g_rtp->GetXaxis()->SetTimeDisplay(1);
  g_rtp->GetXaxis()->SetTimeFormat("%H:%M");
  g_rtp->GetXaxis()->SetTimeOffset(0,"gmt");

  g_rth->Draw("P same");
  g_rth->SetMarkerStyle(kFullCircle);
  g_rth->SetMarkerColor(kRed);
  g_rth->SetMarkerSize(1);
  TLegend *l2 = new TLegend(0.5, 0.73, 0.8, 0.88);
  l2->AddEntry(g_rtp, "Track lumi/PLTZ", "LP");
  l2->AddEntry(g_rth, "Track lumi/HFOC", "LP");
  l2->SetBorderSize(0);
  l2->SetFillColor(0);
  l2->Draw();

  TCanvas *c3 = new TCanvas("c3", "c3", 600, 600);
  g_rlp->Draw("AP");
  titleString = "Luminosity ratios vs. SBIL, fill "+fillNumber;
  g_rlp->SetTitle(titleString.c_str());
  g_rlp->GetXaxis()->SetTitle("SBIL (Hz/#mub)");
  g_rlp->GetYaxis()->SetTitle("Ratio");
  g_rlp->GetYaxis()->SetTitleOffset(1.5);
  g_rlp->SetMarkerStyle(kFullCircle);
  g_rlp->SetMarkerColor(kBlue);
  g_rlp->SetLineColor(kBlue);
  g_rlp->SetMarkerSize(1);
  g_rlp->SetMaximum(1.05);
  g_rlp->SetMinimum(0.95);

  g_rlh->Draw("P same");
  g_rlh->SetMarkerStyle(kFullCircle);
  g_rlh->SetMarkerColor(kRed);
  g_rlh->SetMarkerSize(1);

  TF1 *f_rp = new TF1("f_rp", "pol1");
  f_rp->SetLineColor(kBlue);
  f_rp->SetLineWidth(2);
  g_rlp->Fit(f_rp);

  TF1 *f_rh = new TF1("f_rh", "pol1");
  f_rh->SetLineColor(kRed);
  f_rh->SetLineWidth(2);
  g_rlh->Fit(f_rh);

  TLegend *l3 = new TLegend(0.5, 0.73, 0.8, 0.88);
  std::stringstream legtextp, legtexth;
  legtextp << "#splitline{Track lumi/PLTZ}{slope=" << std::fixed << std::setprecision(1) << f_rp->GetParameter(1)*100 << "%/(Hz/#mub)}";
  legtexth << "#splitline{Track lumi/HFOC}{slope=" << std::fixed << std::setprecision(1) << f_rh->GetParameter(1)*100 << "%/(Hz/#mub)}";
  l3->AddEntry(g_rlp, legtextp.str().c_str(), "LP");
  l3->AddEntry(g_rlh, legtexth.str().c_str(), "LP");
  l3->SetBorderSize(0);
  l3->SetFillColor(0);
  l3->Draw();

  std::string suffix = ".png";
  if (attemptChannelFix)
    suffix = "_fix.png";
  std::string outFile = "TrackLumiVsTime_"+fillNumber+suffix;
  c1->Print(outFile.c_str());
  outFile = "TrackLumiRatiosVsTime_"+fillNumber+suffix;
  c2->Print(outFile.c_str());
  outFile = "TrackLumiRatiosVsSBIL_"+fillNumber+suffix;
  c3->Print(outFile.c_str());
}
