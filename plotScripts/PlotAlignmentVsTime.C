////////////////////////////////////////////////////////////////////
//
// PlotAlignmentVsTime -- a script to plot the alignment changes
//  over the course of time. The alignment files to use are defined
//  in the array below (please also define the names in the other
//  array). Two plots are produced for each channel: one showing
//  the absolute alignment values, and one showing the alignment
//  values relative to their average over all fills.
//
// IMPORTANT NOTE: You can't run this script interactively -- it
// has to be compiled (i.e. .x PlotAlignmentVsTime.C++).
//
//    Paul Lujan, February 23, 2016
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <time.h>
#include <math.h>

#include "TROOT.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TStyle.h"
#include "TAxis.h"
#include "TLegend.h"

const int nScopes = 16; // but not really
const int nFiles = 12;
const char *fileNames[nFiles] = {
  "../ALIGNMENT/Trans_Alignment_4201.dat",
  "../ALIGNMENT/Trans_Alignment_4208.dat",
  "../ALIGNMENT/Trans_Alignment_4256.dat",
  "../ALIGNMENT/Trans_Alignment_4341.dat",
  "../ALIGNMENT/Trans_Alignment_4381.dat",
  "../ALIGNMENT/Trans_Alignment_4449.dat",
  "../ALIGNMENT/Trans_Alignment_4545.dat",
  "../ALIGNMENT/Trans_Alignment_4555.dat",
  "../ALIGNMENT/Trans_Alignment_4565.dat",
  "../ALIGNMENT/Trans_Alignment_4879.dat",
  "../ALIGNMENT/Trans_Alignment_4892.dat",
  "../ALIGNMENT/Trans_Alignment_4895.dat"
};
const char *fillNames[nFiles] = {"4201", "[4208]", "4256", "[4341]", "4381", "4449",
				 "[4545]", "4555", "4565", "4879", "4892", "4895"};
const bool magnetOn[nFiles] = {true, false, true, false, true, true, false, true, true, true, true, true};

std::map<int, std::vector<double> > rot1;
std::map<int, std::vector<double> > rot2;
std::map<int, std::vector<double> > transX1;
std::map<int, std::vector<double> > transX2;
std::map<int, std::vector<double> > transY1;
std::map<int, std::vector<double> > transY2;

void makeHistograms(TH1F* &hAbs, TH1F* &hAvg, const std::vector<double> &v, double &minAbs, double &maxAbs,
		    double &minAvg, double &maxAvg, TH1F* summaryOff, TH1F* summaryOn) {
  // This does the actual work of making the histograms from the data values.
  // It makes two histograms: one with the absolute values, and one with the
  // values relative to the average. It also updates the minimum & maximum values.
  // Finally, it also fills the appropriate summary plot.

  static int iHist=0;
  char name[32];
  sprintf(name, "h%d", iHist);
  ++iHist;
  hAbs = new TH1F(name, name, nFiles, -0.5, nFiles-0.5);

  sprintf(name, "h%d", iHist);
  ++iHist;
  hAvg = new TH1F(name, name, nFiles, -0.5, nFiles-0.5);

  double sum = 0;
  
  // First pass: fill absolute histo, calculate sum for averages
  for (unsigned int i=0; i<v.size(); ++i) {
    hAbs->Fill(i, v[i]);
    if (v[i] < minAbs) minAbs = v[i];
    if (v[i] > maxAbs) maxAbs = v[i];
    sum += v[i];
  }

  // Second pass: fill average histo & summary plots
  for (unsigned int i=0; i<v.size(); ++i) {
    double diff = v[i]-sum/v.size();
    hAvg->Fill(i, diff);
    if (diff < minAvg) minAvg = diff;
    if (diff > maxAvg) maxAvg = diff;
    if (magnetOn[i])
      summaryOn->Fill(diff);
    else
      summaryOff->Fill(diff);
  }
}

void readAlignmentFile(const char *fileName) {
  int scopesRead = 0;
  FILE *afile = fopen(fileName, "r");
  if (afile == NULL) {
    std::cerr << "Couldn't open alignment file " << fileName << "!" << std::endl;
    return;
  }
  int ichan, iroc;
  double rXY, rZ, trX, trY, trZ;
  char dummy[1024];

  // skip header lines
  fgets(dummy, sizeof(dummy), afile);
  fgets(dummy, sizeof(dummy), afile);

  while (1) {
    // scope header line
    fscanf(afile, "%d %d %lf %lf %lf %lf %lf", &ichan, &iroc, &rZ, &rXY, &trX, &trY, &trZ);
    if (feof(afile)) {
      std::cout << "Read in " << scopesRead << " scopes from " << fileName << std::endl;
      return;
    }
    if (iroc != -1) {
      std::cerr << "Warning: expected -1 for ROC (channel " << ichan << ") and found " << iroc << " instead!" << std::endl;
      return;
    }
    // individual roc lines
    for (int i=0; i<3; ++i) {
      int thischan;
      fscanf(afile, "%d %d %lf %lf %lf %lf", &thischan, &iroc, &rXY, &trX, &trY, &trZ);
      if (thischan != ichan) {
	std::cerr << "Warning: expected " << ichan << " for channel and found " << thischan << " instead!" << std::endl;
	return;
      }
      if (iroc != i) {
	std::cerr << "Warning: expected " << i << " for ROC (channel " << ichan << ") and found " << iroc << " instead!" << std::endl;
	return;
      }
      // We found the data! Now save the bits that we want to save.
      if (i==0) {
	if (rXY != 0 || trX != 0 || trY != 0 || trZ != 0) {
	  std::cerr << "Warning: found non-zero alignment values for ROC 0; that shouldn't happen!" << std::endl;
	}
      }
      if (i==1) {
	// std::cout << ichan << " ROC1 " << rXY << " " << trX << " " << trY << " " << trZ << std::endl;
	rot1[ichan].push_back(rXY);
	transX1[ichan].push_back(trX);
	transY1[ichan].push_back(trY);
	if (fabs(trZ-3.77)>0.0001) {
	  std::cerr << "Found unexpected z alignment for ROC 1 (expected 3.77, got " << trZ << ")" << std::endl;
	}
      }
      if (i==2) {
	// std::cout << ichan << " ROC2 " << rXY << " " << trX << " " << trY << " " << trZ << std::endl;
	rot2[ichan].push_back(rXY);
	transX2[ichan].push_back(trX);
	transY2[ichan].push_back(trY);
	if (fabs(trZ-7.54)>0.0001) {
	  std::cerr << "Found unexpected z alignment for ROC 1 (expected 7.54, got " << trZ << ")" << std::endl;
	}
      }
    } // loop over ROCs
    scopesRead++;
  }

  fclose(afile);
}

void PlotAlignmentVsTime(void) {
  // style from PLTU
  gROOT->SetStyle("Plain");                  
  gStyle->SetPalette(1);
  gStyle->SetPadLeftMargin(0.17);
  gStyle->SetPadRightMargin(0.17);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleX(0.1);
  gStyle->SetTitleY(1.0);
  gStyle->SetTitleH(0.09);
  gStyle->SetTitleW(0.7);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetOptStat(0);

  double xvals[nFiles];
  for (int i=0; i<nFiles; ++i) {
    readAlignmentFile(fileNames[i]);
    xvals[i] = i;
  }

  // Make some plots!

  const int nGraphs = 6;
  const char *plotLabel[nGraphs] = {"Rot ROC 1",
				    "Rot ROC 2",
				    "Trans X ROC 1",
				    "Trans X ROC 2",
				    "Trans Y ROC 1",
				    "Trans Y ROC 2"};
  TH1F *hAbs[nScopes][nGraphs];
  TH1F *hAvg[nScopes][nGraphs];
  TCanvas *c[nScopes];
  int nScope = 0;

  // Book the summary histograms.
  char name[32];
  char title[128];

  TH1F *summaryOff[nGraphs];
  TH1F *summaryOn[nGraphs];
  for (int i=0; i<nGraphs; ++i) {
    sprintf(name, "hsumoff_%d", i);
    sprintf(title, "Alignment change vs. average, magnet off, %s", plotLabel[i]);
    summaryOff[i] = new TH1F(name, title, 50, -0.01, 0.01);
    sprintf(name, "hsumon_%d", i);
    sprintf(title, "Alignment change vs. average, magnet on, %s", plotLabel[i]);
    summaryOn[i] = new TH1F(name, title, 50, -0.01, 0.01);
  }

  for (std::map<int, std::vector<double> >::const_iterator it = rot1.begin(); it != rot1.end(); ++it) {
    int chan = it->first;
    // Skip the channels that were dead in 2015.
    if (chan == 22 || chan == 23) continue;
    double minAbs = 1, minAvg = 1, maxAbs = -1, maxAvg = -1;
    makeHistograms(hAbs[nScope][0], hAvg[nScope][0], rot1[chan], minAbs, maxAbs, minAvg, maxAvg, summaryOff[0], summaryOn[0]);
    makeHistograms(hAbs[nScope][1], hAvg[nScope][1], rot2[chan], minAbs, maxAbs, minAvg, maxAvg, summaryOff[1], summaryOn[1]);
    makeHistograms(hAbs[nScope][2], hAvg[nScope][2], transX1[chan], minAbs, maxAbs, minAvg, maxAvg, summaryOff[2], summaryOn[2]);
    makeHistograms(hAbs[nScope][3], hAvg[nScope][3], transX2[chan], minAbs, maxAbs, minAvg, maxAvg, summaryOff[3], summaryOn[3]);
    makeHistograms(hAbs[nScope][4], hAvg[nScope][4], transY1[chan], minAbs, maxAbs, minAvg, maxAvg, summaryOff[4], summaryOn[4]);
    makeHistograms(hAbs[nScope][5], hAvg[nScope][5], transY2[chan], minAbs, maxAbs, minAvg, maxAvg, summaryOff[5], summaryOn[5]);

    sprintf(name, "c%d", nScope);
    c[nScope] = new TCanvas(name, name, 1400, 600);
    c[nScope]->Divide(2, 1);

    c[nScope]->cd(1);
    gPad->SetRightMargin(0.3);
    hAbs[nScope][0]->Draw("LP");

    for (int i=1; i<nGraphs; ++i) {
      hAbs[nScope][i]->Draw("LP same");
    }
    for (int i=0; i<nGraphs; ++i) {
      hAbs[nScope][i]->SetLineColor(i+1);
      hAbs[nScope][i]->SetMarkerColor(i+1);
      hAbs[nScope][i]->SetMarkerStyle(kFullSquare);
    }

    sprintf(title, "Alignment values, FED channel %d", chan);
    hAbs[nScope][0]->SetTitle(title);
    hAbs[nScope][0]->GetXaxis()->SetTitle("Fill (brackets = magnet off)");
    hAbs[nScope][0]->GetYaxis()->SetTitle("Alignment ([rad] or [cm])");
    hAbs[nScope][0]->GetYaxis()->SetRangeUser(minAbs-0.01, maxAbs+0.01);
    hAbs[nScope][0]->GetYaxis()->SetTitleOffset(1.4);

    for (int i=0; i<nFiles; ++i)
      hAbs[nScope][0]->GetXaxis()->SetBinLabel(i+1, fillNames[i]);

    TLegend *l1 = new TLegend(0.72, 0.35, 0.99, 0.9);
    for (int i=0; i<nGraphs; ++i)
      l1->AddEntry(hAbs[nScope][i], plotLabel[i], "LP");
    // l1->AddEntry(hAbs[nScope][0], "#splitline{All rot. & trans. are}{with respect to ROC 0}", "");
    l1->SetFillColor(0);
    l1->Draw();

    c[nScope]->cd(2);
    gPad->SetRightMargin(0.3);
    hAvg[nScope][0]->Draw("LP");

    for (int i=1; i<nGraphs; ++i) {
      hAvg[nScope][i]->Draw("LP same");
    }
    for (int i=0; i<nGraphs; ++i) {
      hAvg[nScope][i]->SetLineColor(i+1);
      hAvg[nScope][i]->SetMarkerColor(i+1);
      hAvg[nScope][i]->SetMarkerStyle(kFullSquare);
    }

    sprintf(title, "Alignment difference from average value, FED channel %d", chan);
    hAvg[nScope][0]->SetTitle(title);
    hAvg[nScope][0]->GetXaxis()->SetTitle("Fill (brackets = magnet off)");
    hAvg[nScope][0]->GetYaxis()->SetTitle("Alignment difference ([rad] or [cm])");
    hAvg[nScope][0]->GetYaxis()->SetRangeUser(minAvg*1.1, maxAvg*1.1);
    hAvg[nScope][0]->GetYaxis()->SetTitleOffset(1.8);

    for (int i=0; i<nFiles; ++i)
      hAvg[nScope][0]->GetXaxis()->SetBinLabel(i+1, fillNames[i]);

    TLegend *l2 = new TLegend(0.72, 0.35, 0.99, 0.9);
    for (int i=0; i<nGraphs; ++i)
      l2->AddEntry(hAvg[nScope][i], plotLabel[i], "LP");
    // l2->AddEntry(hAvg[nScope][0], "#splitline{All rot. & trans. are}{with respect to ROC 0}", "");
    l2->SetFillColor(0);
    l2->Draw();

    sprintf(title, "../plots/AlignmentVsTime_Ch%d.png", chan);
    c[nScope]->Print(title);
    // std::cout << "Saved plot to " << title << std::endl;

    nScope++;
  }
  // Finally, plot the summary plots.
  TCanvas *csum = new TCanvas("csum", "Summary plots", 1200, 600);
  csum->Divide(3,2);
  for (int i=0; i<6; ++i) {
    int padNum = i/2 + 1 + 3*(i%2);
    csum->cd(padNum);
    summaryOn[i]->Draw();
    summaryOn[i]->SetLineColor(kRed);
    sprintf(title, "Alignment difference vs. average, all telecopes, %s", plotLabel[i]);
    summaryOn[i]->SetTitle(title);
    if (i<2)
      summaryOn[i]->GetXaxis()->SetTitle("Alignment difference [rad]");
    else
      summaryOn[i]->GetXaxis()->SetTitle("Alignment difference [cm]");
    summaryOn[i]->GetYaxis()->SetTitle("# of occurrences");
    summaryOff[i]->Draw("same");
    TLegend *ls = new TLegend(0.20, 0.65, 0.47, 0.88);
    ls->AddEntry(summaryOn[i], "Magnet on");
    ls->AddEntry(summaryOff[i], "Magnet off");
    ls->SetFillColor(0);
    ls->Draw();
  }
  csum->Print("../plots/AlignmentVsTime_Summary.png");
}
