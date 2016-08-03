////////////////////////////////////////////////////////////////////
//
//  PlotChannelOccupancy -- a script to plot the number of events
//   with valid tracks as a function of time, on a channel-
//   by-channel basis. 
//    Daniel Gift, July 14, 2016
//    Adapted from PlotRatesZC.C by Paul Lujan
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

void PlotChannelOccupancy(const std::string fileName, const int fill) {
  // style from PLTU
  gROOT->SetStyle("Plain");                  
  gStyle->SetPalette(1);
  gStyle->SetPadLeftMargin(0.17);
  gStyle->SetPadRightMargin(0.10);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleX(0.18);
  gStyle->SetTitleY(1.0);
  gStyle->SetTitleH(0.09);
  gStyle->SetTitleW(0.7);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetOptFit(111);
  gStyle->SetStatY(0.2);

  // Declare arrays where we'll store all the info
  std::vector<double> time;
  std::vector<double> ch2CountRate;
  std::vector<double> ch4CountRate;
  std::vector<double> ch5CountRate;
  std::vector<double> ch7CountRate;
  std::vector<double> ch8CountRate;
  std::vector<double> ch10CountRate;
  std::vector<double> ch11CountRate;
  std::vector<double> ch13CountRate;
  std::vector<double> ch14CountRate;
  std::vector<double> ch16CountRate;
  std::vector<double> ch17CountRate;
  std::vector<double> ch19CountRate;
  std::vector<double> ch20CountRate;
  std::vector<double> ch2ErrorRate;
  std::vector<double> ch4ErrorRate;
  std::vector<double> ch5ErrorRate;
  std::vector<double> ch7ErrorRate;
  std::vector<double> ch8ErrorRate;
  std::vector<double> ch10ErrorRate;
  std::vector<double> ch11ErrorRate;
  std::vector<double> ch13ErrorRate;
  std::vector<double> ch14ErrorRate;
  std::vector<double> ch16ErrorRate;
  std::vector<double> ch17ErrorRate;
  std::vector<double> ch19ErrorRate;
  std::vector<double> ch20ErrorRate;
  std::vector<double> zeros;
  int startTime;

  // Read input file.
  FILE *rfile = fopen(fileName.c_str(), "r");
  if (rfile == NULL) {
    std::cerr << "Couldn't open combined rates file!" << std::endl;
    return(1);
  }
  int nsteps, nBunches, tBegin, tEnd, nTrig;
  int ch2, ch4, ch5, ch7, ch8, ch10, ch11, ch13, ch14, ch16, ch17, ch19, ch20;
  int nFilledTrig;
  double tracksAll, tracksGood;
  double nEmpty, nFull;

  fscanf(rfile, "%d %d", &nsteps, &nBunches);
  for (int i=0; i<nsteps; ++i) {
    fscanf(rfile, "%d %d %d %lf %lf %d %lf %lf", &tBegin, &tEnd, &nTrig, &tracksAll, &tracksGood,
           &nFilledTrig, &nEmpty, &nFull);
    fscanf(rfile, "%d %d %d %d %d %d %d %d %d", &ch2, &ch4, &ch5, &ch7, &ch8, &ch10,
	   &ch11, &ch13, &ch14);
    fscanf(rfile, "%d %d %d %d", &ch16, &ch17, &ch19, &ch20);
    
    // Process the data.
    if (i == 0) // Note the start time of the fill so everything can be based on it
      startTime = tBegin;

    // Add the data to the arrays
    ch2CountRate.push_back((double)ch2);
    ch4CountRate.push_back((double)ch4);
    ch5CountRate.push_back((double)ch5);
    ch7CountRate.push_back((double)ch7);
    ch8CountRate.push_back((double)ch8);
    ch10CountRate.push_back((double)ch10);
    ch11CountRate.push_back((double)ch11);
    ch13CountRate.push_back((double)ch13);
    ch14CountRate.push_back((double)ch14);
    ch16CountRate.push_back((double)ch16);
    ch17CountRate.push_back((double)ch17);
    ch19CountRate.push_back((double)ch19);
    ch20CountRate.push_back((double)ch20);

    // Errors: square root of the value
    ch2ErrorRate.push_back(sqrt(ch2));
    ch4ErrorRate.push_back(sqrt(ch4));
    ch5ErrorRate.push_back(sqrt(ch5));
    ch7ErrorRate.push_back(sqrt(ch7));
    ch8ErrorRate.push_back(sqrt(ch8));
    ch10ErrorRate.push_back(sqrt(ch10));
    ch11ErrorRate.push_back(sqrt(ch11));
    ch13ErrorRate.push_back(sqrt(ch13));
    ch14ErrorRate.push_back(sqrt(ch14));
    ch16ErrorRate.push_back(sqrt(ch16));
    ch17ErrorRate.push_back(sqrt(ch17));
    ch19ErrorRate.push_back(sqrt(ch19));
    ch20ErrorRate.push_back(sqrt(ch20));
    
    //Average time of this step
    time.push_back(((double)(tBegin+tEnd)/(2.) - startTime)/3600000.);

    // Zero vector, will be used for x-error
    zeros.push_back(0.);
  }
  fclose(rfile); 

  // Plot it all.
  
  TGraph *g2 = new TGraphErrors(nsteps, &(time[0]), &(ch2CountRate[0]),
			       &(zeros[0]), &(ch2ErrorRate[0]));
  TGraph *g4 = new TGraphErrors(nsteps, &(time[0]), &(ch4CountRate[0]),
                               &(zeros[0]), &(ch4ErrorRate[0]));
  TGraph *g5 = new TGraphErrors(nsteps, &(time[0]), &(ch5CountRate[0]),
                               &(zeros[0]), &(ch5ErrorRate[0]));
  TGraph *g7 = new TGraphErrors(nsteps, &(time[0]), &(ch7CountRate[0]),
                               &(zeros[0]), &(ch7ErrorRate[0]));
  TGraph *g8 = new TGraphErrors(nsteps, &(time[0]), &(ch8CountRate[0]),
                               &(zeros[0]), &(ch8ErrorRate[0]));
  TGraph *g10 = new TGraphErrors(nsteps, &(time[0]), &(ch10CountRate[0]),
                               &(zeros[0]), &(ch10ErrorRate[0]));
  TGraph *g11 = new TGraphErrors(nsteps, &(time[0]), &(ch11CountRate[0]),
                               &(zeros[0]), &(ch11ErrorRate[0]));
  TGraph *g13 = new TGraphErrors(nsteps, &(time[0]), &(ch13CountRate[0]),
                               &(zeros[0]), &(ch13ErrorRate[0]));
  TGraph *g14 = new TGraphErrors(nsteps, &(time[0]), &(ch14CountRate[0]),
                               &(zeros[0]), &(ch14ErrorRate[0]));
  TGraph *g16 = new TGraphErrors(nsteps, &(time[0]), &(ch16CountRate[0]),
                               &(zeros[0]), &(ch16ErrorRate[0]));
  TGraph *g17 = new TGraphErrors(nsteps, &(time[0]), &(ch17CountRate[0]),
                               &(zeros[0]), &(ch17ErrorRate[0]));
  TGraph *g19 = new TGraphErrors(nsteps, &(time[0]), &(ch19CountRate[0]),
                               &(zeros[0]), &(ch19ErrorRate[0]));
  TGraph *g20 = new TGraphErrors(nsteps, &(time[0]), &(ch20CountRate[0]),
                               &(zeros[0]), &(ch20ErrorRate[0]));


  std::stringstream plotTitle;

  TCanvas *c2 = new TCanvas("c2", "c2", 750, 600);
  g2->Draw("AP");
  plotTitle << "Channel 2 Hit rate, Fill " << fill;
  g2->SetTitle((plotTitle.str()).c_str());
  g2->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g2->GetYaxis()->SetTitle("Channel Event Counts");
  g2->GetYaxis()->SetTitleOffset(1.8);
  g2->SetMarkerStyle(kFullCircle);
  g2->SetMarkerColor(kBlue);
  g2->SetMarkerSize(1);

  TCanvas *c4 = new TCanvas("c4", "c4", 750, 600);
  g4->Draw("AP");
  plotTitle.str("");  
  plotTitle << "Channel 4 Hit rate, Fill " << fill;
  g4->SetTitle((plotTitle.str()).c_str());
  g4->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g4->GetYaxis()->SetTitle("Channel Event Counts");
  g4->GetYaxis()->SetTitleOffset(1.8);
  g4->SetMarkerStyle(kFullCircle);
  g4->SetMarkerColor(kBlue);
  g4->SetMarkerSize(1);

  TCanvas *c5 = new TCanvas("c5", "c5", 750, 600);
  g5->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 5 Hit rate, Fill " << fill;
  g5->SetTitle((plotTitle.str()).c_str());
  g5->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g5->GetYaxis()->SetTitle("Channel Event Counts");
  g5->GetYaxis()->SetTitleOffset(1.8);
  g5->SetMarkerStyle(kFullCircle);
  g5->SetMarkerColor(kBlue);
  g5->SetMarkerSize(1);
  
  TCanvas *c7 = new TCanvas("c7", "c7", 750, 600);
  g7->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 7 Hit rate, Fill " << fill;
  g7->SetTitle((plotTitle.str()).c_str());
  g7->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g7->GetYaxis()->SetTitle("Channel Event Counts");
  g7->GetYaxis()->SetTitleOffset(1.8);
  g7->SetMarkerStyle(kFullCircle);
  g7->SetMarkerColor(kBlue);
  g7->SetMarkerSize(1);

  TCanvas *c8 = new TCanvas("c8", "c8", 750, 600);
  g8->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 8 Hit rate, Fill " << fill;
  g8->SetTitle((plotTitle.str()).c_str());
  g8->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g8->GetYaxis()->SetTitle("Channel Event Counts");
  g8->GetYaxis()->SetTitleOffset(1.8);
  g8->SetMarkerStyle(kFullCircle);
  g8->SetMarkerColor(kBlue);
  g8->SetMarkerSize(1);

  TCanvas *c10 = new TCanvas("c10", "c10", 750, 600);
  g10->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 10 Hit rate, Fill " << fill;
  g10->SetTitle((plotTitle.str()).c_str());
  g10->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g10->GetYaxis()->SetTitle("Channel Event Counts");
  g10->GetYaxis()->SetTitleOffset(1.8);
  g10->SetMarkerStyle(kFullCircle);
  g10->SetMarkerColor(kBlue);
  g10->SetMarkerSize(1);

  TCanvas *c11 = new TCanvas("c11", "c11", 750, 600);
  g11->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 11 Hit rate, Fill " << fill;
  g11->SetTitle((plotTitle.str()).c_str());
  g11->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g11->GetYaxis()->SetTitle("Channel Event Counts");
  g11->GetYaxis()->SetTitleOffset(1.8);
  g11->SetMarkerStyle(kFullCircle);
  g11->SetMarkerColor(kBlue);
  g11->SetMarkerSize(1);

  TCanvas *c13 = new TCanvas("c13", "c13", 750, 600);
  g13->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 13 Hit rate, Fill " << fill;
  g13->SetTitle((plotTitle.str()).c_str());
  g13->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g13->GetYaxis()->SetTitle("Channel Event Counts");
  g13->GetYaxis()->SetTitleOffset(1.8);
  g13->SetMarkerStyle(kFullCircle);
  g13->SetMarkerColor(kBlue);
  g13->SetMarkerSize(1);

  TCanvas *c14 = new TCanvas("c14", "c14", 750, 600);
  g14->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 14 Hit rate, Fill " << fill;
  g14->SetTitle((plotTitle.str()).c_str());
  g14->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g14->GetYaxis()->SetTitle("Channel Event Counts");
  g14->GetYaxis()->SetTitleOffset(1.8);
  g14->SetMarkerStyle(kFullCircle);
  g14->SetMarkerColor(kBlue);
  g14->SetMarkerSize(1);

  TCanvas *c16 = new TCanvas("c16", "c16", 750, 600);
  g16->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 16 Hit rate, Fill " << fill;
  g16->SetTitle((plotTitle.str()).c_str());
  g16->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g16->GetYaxis()->SetTitle("Channel Event Counts");
  g16->GetYaxis()->SetTitleOffset(1.8);
  g16->SetMarkerStyle(kFullCircle);
  g16->SetMarkerColor(kBlue);
  g16->SetMarkerSize(1);

  TCanvas *c17 = new TCanvas("c17", "c17", 750, 600);
  g17->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 17 Hit rate, Fill " << fill;
  g17->SetTitle((plotTitle.str()).c_str());
  g17->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g17->GetYaxis()->SetTitle("Channel Event Counts");
  g17->GetYaxis()->SetTitleOffset(1.8);
  g17->SetMarkerStyle(kFullCircle);
  g17->SetMarkerColor(kBlue);
  g17->SetMarkerSize(1);

  TCanvas *c19 = new TCanvas("c19", "c19", 750, 600);
  g19->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 19 Hit rate, Fill " << fill;
  g19->SetTitle((plotTitle.str()).c_str());
  g19->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g19->GetYaxis()->SetTitle("Channel Event Counts");
  g19->GetYaxis()->SetTitleOffset(1.8);
  g19->SetMarkerStyle(kFullCircle);
  g19->SetMarkerColor(kBlue);
  g19->SetMarkerSize(1);

  TCanvas *c20 = new TCanvas("c20", "c20", 750, 600);
  g20->Draw("AP");
  plotTitle.str("");
  plotTitle << "Channel 20 Hit rate, Fill " << fill;
  g20->SetTitle((plotTitle.str()).c_str());
  g20->GetXaxis()->SetTitle("Time from start of fill (hours)");
  g20->GetYaxis()->SetTitle("Channel Event Counts");
  g20->GetYaxis()->SetTitleOffset(1.8);
  g20->SetMarkerStyle(kFullCircle);
  g20->SetMarkerColor(kBlue);
  g20->SetMarkerSize(1);
}
