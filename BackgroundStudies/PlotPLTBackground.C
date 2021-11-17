////////////////////////////////////////////////////////////////////
//
// PlotPLTBackground.C -- this takes the background files generated
// by computerPLTBackground.py and plots them. In addition, it
// plots the vacuum pressure provided in another CSV file.
//
// Three files are needed:
// - background_bcm1f.csv and background_pltz.csv, the CSV files produced by computePLTBackground.py
// - background_vacuum.csv. This file I extracted from Timber for the six variables VGPB.7.4L5.X.PR,
//   VGPB.7.4R5.X.PR, VGPB.220.1L5.X.PR, VGPB.220.1R5.X.PR, VPIAN.904.4L5.B.PR, and VPIAN.904.4R5.B.PR for the
//   time period we're interested in (2016-06-11 from 12:00 to 16:40) and then exporting as a file with "Group
//   by timestamp" and "Time in UNIX format" selected.
//   For more details on the vacuum gauges used see Moritz and Laza's talk here:
//   https://indico.cern.ch/event/522880/contributions/2208385/attachments/1294851/1930109/BeamGasStudyInCMS.pdf
//   Timber is at timber.cern.ch but can only be accessed from within CERN (or via proxy).
//
// Paul Lujan, November 11, 2021
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

const std::string bcm1fFileName = "background_bcm1f.csv";
const std::string pltzFileName = "background_pltz.csv";
const std::string vacuumFileName = "background_vacuum.csv";
const int mycolors[6] = {4, 8, 2, 6, 28, 7};

void readCSVFile(std::string fileName, int n, std::vector<double>& timestamps, std::vector<double>& y1, std::vector<double>& y2, std::vector<double>& y3, std::vector<double>& y4) {
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

    if (fields.size() != 5+n) {
      std::cout << "Malformed line in csv file: " << line << std::endl;
      continue;
    }

    std::stringstream timestampSecString(fields[0]);
    int timestamp_sec;
    timestampSecString >> timestamp_sec;

    std::stringstream timestampMSString(fields[1]);
    int timestamp_ms;
    timestampMSString >> timestamp_ms;

    double timestamp = (double)timestamp_sec + (double)timestamp_ms/1000.0;
    timestamps.push_back(timestamp);

    for (int i=0; i<n; ++i) {
      std::stringstream fieldString(fields[5+i]);
      double fieldVal;
      fieldString >> fieldVal;
      if (i==0) y1.push_back(fieldVal);
      if (i==1) y2.push_back(fieldVal);
      if (i==2) y3.push_back(fieldVal);
      if (i==3) y4.push_back(fieldVal);
    }
  } // loop over lines
} // routine

void PlotPLTBackground(void) {
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
  gStyle->SetOptStat(0);
  //gStyle->SetOptFit(1111);
  //gStyle->SetTitleX(0.12); // to avoid the white space in the title hitting the numbers on the axis label

  // Read brilcalc files.
  std::vector<double> bcm1f_timestamps;
  std::vector<double> bcm1f_bkgnd1;
  std::vector<double> bcm1f_bkgnd2;
  readCSVFile(bcm1fFileName, 2, bcm1f_timestamps, bcm1f_bkgnd2, bcm1f_bkgnd1, bcm1f_bkgnd1, bcm1f_bkgnd2);

  std::vector<double> pltz_timestamps;
  std::vector<double> pltz_bkgndA1;
  std::vector<double> pltz_bkgndA2;
  std::vector<double> pltz_bkgndB1;
  std::vector<double> pltz_bkgndB2;
  readCSVFile(pltzFileName, 4, pltz_timestamps, pltz_bkgndA1, pltz_bkgndA2, pltz_bkgndB1, pltz_bkgndB2);

  // Normalize the PLT values to the BCM1F values.
  double bcm1f_b1_sum = 0;
  double bcm1f_b2_sum = 0;
  double pltz_bA1_sum = 0;
  double pltz_bA2_sum = 0;
  double pltz_bB1_sum = 0;
  double pltz_bB2_sum = 0;
  for (int i=0; i<bcm1f_timestamps.size(); ++i) {
    bcm1f_b1_sum += bcm1f_bkgnd1[i];
    bcm1f_b2_sum += bcm1f_bkgnd2[i];
  }
  for (int i=0; i<pltz_timestamps.size(); ++i) {
    pltz_bA1_sum += pltz_bkgndA1[i];
    pltz_bA2_sum += pltz_bkgndA2[i];
    pltz_bB1_sum += pltz_bkgndB1[i];
    pltz_bB2_sum += pltz_bkgndB2[i];
  }
  std::cout << "Normalization factor, method A, beam 1: " << bcm1f_b1_sum/pltz_bA1_sum << std::endl;
  std::cout << "Normalization factor, method A, beam 2: " << bcm1f_b2_sum/pltz_bA2_sum << std::endl;
  std::cout << "Normalization factor, method B, beam 1: " << bcm1f_b1_sum/pltz_bB1_sum << std::endl;
  std::cout << "Normalization factor, method B, beam 2: " << bcm1f_b2_sum/pltz_bB2_sum << std::endl;
  for (int i=0; i<pltz_timestamps.size(); ++i) {
    pltz_bkgndA1[i] *= bcm1f_b1_sum/pltz_bA1_sum;
    pltz_bkgndA2[i] *= bcm1f_b2_sum/pltz_bA2_sum;
    pltz_bkgndB1[i] *= bcm1f_b1_sum/pltz_bB1_sum;
    pltz_bkgndB2[i] *= bcm1f_b2_sum/pltz_bB2_sum;
  }

  // Plot it all.
  TGraph *g_b1 = new TGraph(bcm1f_timestamps.size(), &(bcm1f_timestamps[0]), &(bcm1f_bkgnd1[0]));
  TGraph *g_b2 = new TGraph(bcm1f_timestamps.size(), &(bcm1f_timestamps[0]), &(bcm1f_bkgnd2[0]));
  TGraph *g_pA1 = new TGraph(pltz_timestamps.size(), &(pltz_timestamps[0]), &(pltz_bkgndA1[0]));
  TGraph *g_pA2 = new TGraph(pltz_timestamps.size(), &(pltz_timestamps[0]), &(pltz_bkgndA2[0]));
  TGraph *g_pB1 = new TGraph(pltz_timestamps.size(), &(pltz_timestamps[0]), &(pltz_bkgndB1[0]));
  TGraph *g_pB2 = new TGraph(pltz_timestamps.size(), &(pltz_timestamps[0]), &(pltz_bkgndB2[0]));

  TCanvas *c[2];
  std::string cnames[2] = {"c1", "c2"};

  for (int ic=0; ic<2; ++ic) {
    c[ic] = new TCanvas(cnames[ic].c_str(), cnames[ic].c_str(), 600, 600);

    // Set up the two pads for the main plot and the residual plot below.
    TPad *p1 = new TPad((cnames[ic]+"p1").c_str(), (cnames[ic]+"p1").c_str(), 0, 0.3, 1, 1);
    TPad *p2 = new TPad((cnames[ic]+"p2").c_str(), (cnames[ic]+"p2").c_str(), 0, 0, 1, 0.3);
    p1->SetBottomMargin(0.1);
    p1->SetBorderMode(0);
    p2->SetTopMargin(1e-5);
    p2->SetBottomMargin(0.15);
    p2->SetBorderMode(0);
    p1->Draw();
    p2->Draw();
    p1->cd();

    g_b2->Draw("AL");
    //g_b2->SetTitle("Measured background rates");
    g_b2->SetTitle("");
    g_b2->GetXaxis()->SetTitle("CERN time");
    g_b2->GetYaxis()->SetTitle("Background rate [Hz/cm^{2}/(10^{11} protons)]");
    g_b2->GetYaxis()->SetTitleOffset(1.3);
    g_b2->SetLineColor(mycolors[1]);
    g_b2->GetXaxis()->SetTimeDisplay(1);
    g_b2->GetXaxis()->SetTimeFormat("%H:%M");
    g_b2->GetXaxis()->SetTimeOffset(0,"gmt");

    // give us a bit more headroom
    g_b2->SetMaximum(*std::max_element(bcm1f_bkgnd2.begin(), bcm1f_bkgnd2.end())*1.32);

    g_b1->Draw("L same");
    g_b1->SetLineColor(mycolors[0]);

    if (ic==0) {
      g_pA1->Draw("L same");
      g_pA1->SetLineColor(mycolors[2]);
      
      g_pA2->Draw("L same");
      g_pA2->SetLineColor(mycolors[3]);
    } else {
      g_pB1->Draw("L same");
      g_pB1->SetLineColor(mycolors[2]);
      
      g_pB2->Draw("L same");
      g_pB2->SetLineColor(mycolors[3]);
    }
  
    TLegend *l = new TLegend(0.19, 0.57, 0.54, 0.87);
    l->SetHeader("Backgrounds");
    l->AddEntry(g_b1, "BCM1F beam 1", "L");
    l->AddEntry(g_b2, "BCM1F beam 2", "L");
    if (ic==0) {
      l->AddEntry(g_pA1, "PLT beam 1 (noncolliding)", "L");
      l->AddEntry(g_pA2, "PLT beam 2 (noncolliding)", "L");
    } else {
      l->AddEntry(g_pB1, "PLT beam 1 (precolliding)", "L");
      l->AddEntry(g_pB2, "PLT beam 2 (precolliding)", "L");
    }
    l->SetBorderSize(0);
    l->SetFillColor(0);
    l->Draw();

    TText *t1 = new TText(0, 0, "CMS");
    t1->SetNDC();
    t1->SetX(0.65);
    t1->SetY(0.84);
    t1->SetTextFont(61);
    t1->SetTextSize(0.05);
    t1->Draw();
    TText *t2 = new TText(0, 0, "Preliminary");
    t2->SetNDC();
    t2->SetX(0.73);
    t2->SetY(0.84);
    t2->SetTextFont(52);
    t2->SetTextSize(0.05);
    t2->Draw();
    TText *t3 = new TText(0, 0, "2016");
    t3->SetNDC();
    t3->SetX(0.81);
    t3->SetY(0.78);
    t3->SetTextSize(0.05);
    t3->Draw();

    // Draw vacuum plots. Make a dummy histogram first with the same x-axis as the main histogram.
    p2->cd();
    TH1F *hvac = new TH1F((cnames[ic]+"hvac").c_str(), (cnames[ic]+"hvac").c_str(), g_b2->GetXaxis()->GetNbins(),
			  g_b2->GetXaxis()->GetXmin(), g_b2->GetXaxis()->GetXmax());
    hvac->Draw();
    hvac->SetTitle("");
    hvac->GetXaxis()->SetLabelSize(0.09);
    hvac->GetYaxis()->SetLabelSize(0.09);
    hvac->GetXaxis()->SetTimeDisplay(1);
    hvac->GetXaxis()->SetTimeFormat("%H:%M");
    hvac->GetXaxis()->SetTimeOffset(0,"gmt");
    hvac->GetYaxis()->SetTitle("Pressure [bar]");
    hvac->GetYaxis()->SetTitleSize(0.10);
    hvac->GetYaxis()->SetTitleOffset(0.5);
    p2->SetLogy();

    const int nvac = 6;

    // Now read the vacuum data. This is ALMOST the same as ReadCSVFile above, but a little more complicated because
    // each item does not have data for every time stamp, so we have to create separate X arrays for each one. Whee.

    std::vector<double> vacuum_timestamps[nvac];
    std::vector<double> vacuum_pressure[nvac];
  
    std::ifstream csvFile(vacuumFileName.c_str());
    if (!csvFile.is_open()) {
      std::cerr << "ERROR: cannot open csv file: " << vacuumFileName << std::endl;
      return;
    }
    
    // Go through the lines of the file.
    int lsnum = 0;
    std::string line;
    std::getline(csvFile, line); // skip header line
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

      // to account for cases when there's no item in last field
      if (fields.size() == nvac)
	fields.push_back("");

      if (fields.size() != nvac+1) {
	std::cout << "Malformed line in csv file: " << fields.size() << " " << line << std::endl;
	continue;
      }

      std::stringstream timestampString(fields[0]);
      double timestamp;
      timestampString >> timestamp;

      // Apparently the timestamps from Timber aren't true Unix timestamps but also include (with no divider
      // whatsoever) the time in MICROSECONDS, so convert.
      timestamp /= 1e6;

      for (int i=0; i<nvac; ++i) {
	std::stringstream fieldString(fields[1+i]);
	if (fieldString.str().length() > 0) {
	  double fieldVal;
	  fieldString >> fieldVal;
	  vacuum_timestamps[i].push_back(timestamp);
	  // convert mbar to bar, since having something like 10^-7 mbar is a little silly
	  vacuum_pressure[i].push_back(fieldVal/1000.0);
	}
      }
    } // loop over lines
  
    hvac->SetMinimum(4e-14);
    hvac->SetMaximum(5e-9);

    // Whew, now plot them.
    TGraph *g_vac[nvac];
    for (int i=0; i<nvac; ++i) {
      g_vac[i] = new TGraph(vacuum_timestamps[i].size(), &(vacuum_timestamps[i][0]), &(vacuum_pressure[i][0]));
      g_vac[i]->Draw("L same");
      g_vac[i]->SetLineColor(mycolors[i]);
    }

    TLegend *lv = new TLegend(0.15, 0.30, 0.29, 0.99);
    //lv->SetHeader("Vacuum gauges");
    lv->AddEntry(g_vac[4], "L 148m", "L");
    lv->AddEntry(g_vac[5], "R 148m", "L");
    lv->AddEntry(g_vac[2], "L 58m", "L");
    lv->AddEntry(g_vac[3], "R 58m", "L");
    lv->AddEntry(g_vac[0], "L 22m", "L");
    lv->AddEntry(g_vac[1], "R 22m", "L");
    lv->SetBorderSize(0);
    lv->SetFillColor(0);
    lv->Draw();

    if (ic==0) {
      c[ic]->Print("PLTBackgroundNoncoll.png");
      c[ic]->Print("PLTBackgroundNoncoll.pdf");
    } else {
      c[ic]->Print("PLTBackgroundPrecoll.png");
      c[ic]->Print("PLTBackgroundPrecoll.pdf");
    }
  }
}
