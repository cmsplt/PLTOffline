////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Apr 10 17:35:17 CEST 2013
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PLTTesterEvent.h"

#include "TFile.h"
#include "TSystem.h"
#include "TGraphErrors.h"
#include "TH2F.h"



void WriteHTML (TString const);



int TestStandTest (std::string const DataFileName, std::string const GainCalFileName, std::string const OutDir)
{

  // Get rootfile name and make directory for output files
  std::cout << "Output will be sent to: " << OutDir << std::endl;
  TString const OutRootFileName = OutDir + "/TestOut.root";
  gSystem->mkdir(OutDir.c_str(), true);

  // Some basic graphics style
  PLTU::SetStyle();

  // Open the root file and output file
  TFile fOutRoot(OutRootFileName, "recreate");
  std::ofstream OutFile("TestOut.txt");

  // Initalize the Event object and decide what kind of clustering you want
  PLTTesterEvent Event(DataFileName, GainCalFileName, OutDir, &fOutRoot);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_Diamond);


  // Bins and max for pulse height plots
  int   const NBins =     60;
  float const XMin  =  -1000;
  float const XMax  =  50000;


  // Define occupancy plot and attach it to the root file
  TH2F hOccupancy("Occupancy", "Occupancy", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
  hOccupancy.SetDirectory(&fOutRoot);

  // The PH 2D plot is done using a running average for each pixel using these two arrays
  // MUST set these ALL to zero.  Different systems default differently in this case
  double PulseHeightAvg2D[PLTU::NCOL][PLTU::NROW];
  int    PulseHeightN2D[PLTU::NCOL][PLTU::NROW];
  for (int i = 0; i < PLTU::NCOL; ++i) {
    for (int j = 0; j < PLTU::NROW; ++j) {
      PulseHeightAvg2D[i][j] = 0;
      PulseHeightN2D[i][j] = 0;
    }
  }

  // The Charge 2D plot is done using a running average for each pixel using these two arrays
  // MUST set these ALL to zero.  Different systems default differently in this case
  double ChargeAvg2D[PLTU::NCOL][PLTU::NROW];
  int    ChargeN2D[PLTU::NCOL][PLTU::NROW];
  for (int i = 0; i < PLTU::NCOL; ++i) {
    for (int j = 0; j < PLTU::NROW; ++j) {
      ChargeAvg2D[i][j] = 0;
      ChargeN2D[i][j] = 0;
    }
  }

  // This is for the PH vs time averag.  The 0 index is for "all", 3 index is for >= 3 pixels
  double PulseHeightTimeAvg[4] = { 0, 0, 0, 0 };
  int    PulseHeightTimeN[4]   = { 0, 0, 0, 0 };

  // Define the histogram colors for PH plots and the actual graphs
  int const HistColors[4] = { 1, 4, 28, 2 };
  TGraphErrors gPHT[4];
  for (int ig = 0; ig < 4; ++ig) {
    gPHT[ig].Set(0);
    gPHT[ig].SetName( TString::Format("PulseHeightVsTime_Cl%i", ig) );
    gPHT[ig].SetMinimum(0);
    gPHT[ig].SetMaximum(60000);
    gPHT[ig].SetMarkerColor(HistColors[ig]);
    gPHT[ig].SetLineColor(HistColors[ig]);
  }
  gPHT[0].SetTitle("Average Pulse Height");
  gPHT[1].SetTitle("Avg PH for 1 Pixel Clusters");
  gPHT[2].SetTitle("Avg PH for 2 Pixel Clusters");
  gPHT[3].SetTitle("Avg PH for #ge 3 Pixel Clusters");

  // This is to define the time width on the pulseheight vs time plots.
  // EndTimeWindow is to keep track of where the next time window ends
  int const TimeWidth = 5;
  int EndTimeWindow = TimeWidth;


  // Define the 4 PulseHeight plots, attach them to root file, and give them nice colours or colors.
  TH1F* hPulseHeight[4];
  hPulseHeight[0] = new TH1F("PulseHeight_All", "Pulse Height for All Clusters", NBins, XMin, XMax);
  hPulseHeight[0]->SetLineColor(HistColors[0]);
  hPulseHeight[0]->SetDirectory(&fOutRoot);
  for (int ih = 1; ih != 4; ++ih) {
    hPulseHeight[ih] = new TH1F( TString::Format("PulseHeight_Pixels%i", ih), TString::Format("Pulse Height for %i Pixel Clusters", ih), NBins, XMin, XMax);
    hPulseHeight[ih]->SetLineColor(HistColors[ih]);
    hPulseHeight[ih]->SetDirectory(&fOutRoot);
  }




  // This is the main event loop
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << " time: " << Event.Time() << std::endl;
    }

    // What time does this data start at and what time is this event at?
    static int const StartTime = Event.Time();
    int const ThisTime = Event.Time() - StartTime;

    // Have we come to the end of the time window?  If so we make a point on the graphs
    // and then define the next time window
    if (!(ThisTime < EndTimeWindow)) {

      // Loop over all graphs and add points as needed for the PH vs time plot
      for (int ig = 0; ig < 4; ++ig) {

        // If there are an clusters in the average we make a point for it
        if (PulseHeightTimeN[ig] > 0) {
          int const N = gPHT[ig].GetN();
          gPHT[ig].Set(N+1);
          gPHT[ig].SetPoint(N , EndTimeWindow - TimeWidth / 2, PulseHeightTimeAvg[ig]);
          gPHT[ig].SetPointError(N, TimeWidth / 2, PulseHeightTimeAvg[ig] / sqrt((float) PulseHeightTimeN[ig]));
        }

        // Reset the averages and counters for the next time window
        PulseHeightTimeAvg[ig] = 0;
        PulseHeightTimeN[ig] = 0;
      }

      // The end of the next time window moves up by one TimeWidth
      EndTimeWindow += TimeWidth;
    }



    // This is if you want a text dump of the data
    //Event.WriteEventText(OutFile);

    // Loop over all clusters in the event
    for (size_t icluster = 0; icluster != Event.NClusters(); ++icluster) {

      // Grab this cluster
      PLTCluster* Cluster = Event.Cluster(icluster);

      // These are row and column indices for use in array
      int const col = PLTGainCal::ColIndex(Cluster->SeedHit()->Column());
      int const row = PLTGainCal::RowIndex(Cluster->SeedHit()->Row());

      // Grab this cluster charge
      float const ThisClusterCharge = Cluster->Charge();

      // If this charge is something crazy we skip it
      if (ThisClusterCharge > 1000000 || ThisClusterCharge < -1000000) {
        continue;
      }

      // Just out of curosity print zero-charge clusters
      if (ThisClusterCharge == 0) {
        printf("Cluster has zero charge:   NHits: %3i  Column: %2i  Row: %2i\n", (int) Cluster->NHits(), Cluster->Hit(0)->Column(), Cluster->Hit(0)->Row());
        continue;
      }

      // Number of hits in this cluster
      int const NHitsInCluster = Cluster->NHits();

      // Fill in PH histogram
      hPulseHeight[0]->Fill(ThisClusterCharge);
      NHitsInCluster > 2 ? hPulseHeight[3]->Fill(ThisClusterCharge) : hPulseHeight[NHitsInCluster]->Fill(ThisClusterCharge);

      // Add to running averages for the PH vs time plots
      PLTU::AddToRunningAverage(PulseHeightTimeAvg[0], PulseHeightTimeN[0], ThisClusterCharge);
      NHitsInCluster > 2 ?    PLTU::AddToRunningAverage(PulseHeightTimeAvg[3], PulseHeightTimeN[3], ThisClusterCharge) :
                              PLTU::AddToRunningAverage(PulseHeightTimeAvg[NHitsInCluster], PulseHeightTimeN[NHitsInCluster], ThisClusterCharge);

      // If the charge is not crazy use it in our running average
      if (ThisClusterCharge < 5000000 && ThisClusterCharge >= 0) {
        PLTU::AddToRunningAverage(PulseHeightAvg2D[col][row], PulseHeightN2D[col][row], ThisClusterCharge);
      }

    }

    // This loop is over all hits in the event.  This includes unclustered hits.  Depending on the clustering algorithm
    // you may or may not have unclustered hits, but these would need to be dealt elsewhere (not in this loop)
    for (size_t ihit = 0; ihit != Event.NHits(); ++ihit) {

      // Grab this hit
      PLTHit* Hit = Event.Hit(ihit);

      // Fill Occupancy hist
      hOccupancy.Fill(Hit->Column(), Hit->Row());

      // Get Charge
      float const ThisHitCharge = Hit->Charge();
      int const col = PLTGainCal::ColIndex(Hit->Column());
      int const row = PLTGainCal::RowIndex(Hit->Row());

      // If the charge is not crazy use it in our running average
      if (ThisHitCharge < 5000000 && ThisHitCharge >= 0) {
        PLTU::AddToRunningAverage(ChargeAvg2D[col][row], ChargeN2D[col][row], ThisHitCharge);
      }

    }

  } // END OF MAIN EVENT LOOP















  // BEGIN PLOTTING and DRAWING ROUTINES




  // This makes levels histograms
  Event.MakePlots();


  // Loop over all graphs and set titles
  for (int ig = 0; ig < 4; ++ig) {
    gPHT[ig].GetXaxis()->SetTitle("Time (s)");
    gPHT[ig].GetYaxis()->SetTitle("PH (electrons)");
    gPHT[ig].Write();
  }

  // Make the pulse height vs time canvas and draw graphs on the canvas, save
  TCanvas cPHT("PHVsTime", "PHVsTime");
  gPHT[0].Draw("Ap");
  gPHT[1].Draw("p");
  gPHT[2].Draw("p");
  gPHT[3].Draw("p");
  gPHT[0].GetXaxis()->SetTitle("Time (s)");
  gPHT[0].GetYaxis()->SetTitle("PH (electrons)");
  cPHT.Write();
  cPHT.SaveAs(TString(OutDir) + "/PHVsTime.gif");


  // Draw the pulse height 1D distributions
  TCanvas cPH("PH", "PH");
  hPulseHeight[0]->Draw("Hist");
  hPulseHeight[1]->Draw("SameHist");
  hPulseHeight[2]->Draw("SameHist");
  hPulseHeight[3]->Draw("SameHist");
  hPulseHeight[0]->GetXaxis()->SetTitle("PH (electrons)");
  cPH.Write();
  cPH.SaveAs(TString(OutDir) + "/PulseHeight.gif");



  // Set the 2D PulseHeight plot, draw it, and save it
  TCanvas cPulseHeightAvg2D("AveragePulseHeight", "Average Pulse Height");
  cPulseHeightAvg2D.cd();
  // Define the 2D PH plot and attach it to the root file.
  // The PH plots use the SeedHit as the cluster center.  It is the Custer charge that
  // is plotted at the SeedHit column and row
  TH2F hPulseHeightAvg2D("PulseHeightAvg", "Average Pulse Height", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
  hPulseHeightAvg2D.SetDirectory(&fOutRoot);

  for (int ja = 0; ja != PLTU::NROW; ++ja) {
    for (int ia = 0; ia != PLTU::NCOL; ++ia) {
      if (PulseHeightAvg2D[ia][ja] > 0) {
        hPulseHeightAvg2D.SetBinContent(ia+1, ja+1, PulseHeightAvg2D[ia][ja]);
      }
      printf("%6.0f ", (float) PulseHeightAvg2D[ia][ja]);
    }
    std::cout << std::endl;
  }
  hPulseHeightAvg2D.SetMaximum(XMax);
  hPulseHeightAvg2D.SetStats(false);
  hPulseHeightAvg2D.SetXTitle("Column");
  hPulseHeightAvg2D.SetYTitle("Row");
  hPulseHeightAvg2D.SetZTitle("Electrons");
  hPulseHeightAvg2D.Draw("colz");
  hPulseHeightAvg2D.Write();
  cPulseHeightAvg2D.SaveAs(TString(OutDir) + "/PulseHeightAvg.gif");


  // Set the 2D Charge plot, draw it, and save it
  TCanvas cChargeAvg2D("AverageCharge", "Average Pixel Charge");
  cChargeAvg2D.cd();
  // Define the 2D Charge plot and attach it to the root file
  // This plot is really the charge at each pixel with no clustering.
  // It is NOT the pulse height
  TH2F hChargeAvg2D("ChargeAvg", "Average Pixel Charge", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
  hChargeAvg2D.SetDirectory(&fOutRoot);
  for (int ja = 0; ja != PLTU::NROW; ++ja) {
    for (int ia = 0; ia != PLTU::NCOL; ++ia) {
      if (ChargeAvg2D[ia][ja] > 0) {
        hChargeAvg2D.SetBinContent(ia+1, ja+1, ChargeAvg2D[ia][ja]);
      }
      printf("%6.0f ", (float) ChargeAvg2D[ia][ja]);
    }
    std::cout << std::endl;
  }
  hChargeAvg2D.SetMaximum(XMax);
  hChargeAvg2D.SetStats(false);
  hChargeAvg2D.SetXTitle("Column");
  hChargeAvg2D.SetYTitle("Row");
  hChargeAvg2D.SetZTitle("Electrons");
  hChargeAvg2D.Draw("colz");
  hChargeAvg2D.Write();
  cChargeAvg2D.SaveAs(TString(OutDir) + "/ChargeAvg.gif");

  // Make the 1D charge Avg distribution
  TCanvas cChargeAvg1D;
  cChargeAvg1D.cd();
  TH1F* hChargeAvg1D = PLTU::HistFrom2D(&hChargeAvg2D, XMin, XMax, "ChargeAvg1D", 50, true);
  hChargeAvg1D->SetDirectory(&fOutRoot);
  hChargeAvg1D->Draw("hist");
  cChargeAvg1D.SaveAs(TString(OutDir) + "/ChargeAvg1D.gif");



  // Plot charge divided by mean charge
  TCanvas cChargeAvg2D_wrtMean;
  cChargeAvg2D_wrtMean.cd();
  TH2F* hChargeAvg2D_wrtMean = (TH2F*) hChargeAvg2D.Clone("ChargeAvg2D_wrtMean");
  hChargeAvg2D_wrtMean->SetDirectory(&fOutRoot);
  hChargeAvg2D_wrtMean->SetTitle("Charge per Pixel w.r.t. Mean Charge");
  float const MeanCharge = PLTU::GetMeanBinContentSkipEmptyBins(hChargeAvg2D);
  hChargeAvg2D_wrtMean->Scale(1.0 / MeanCharge);
  hChargeAvg2D_wrtMean->SetMinimum(0);
  hChargeAvg2D_wrtMean->SetMaximum(3);
  hChargeAvg2D_wrtMean->Draw("colz");
  cChargeAvg2D_wrtMean.SaveAs(TString(OutDir) + "/ChargeAvg2D_wrtMean.gif");

  // Now the 1D dirtrubution of charge wrt mean
  TCanvas cChargeAvg1D_wrtMean;
  cChargeAvg1D_wrtMean.cd();
  TH1F* hChargeAvg1D_wrtMean = PLTU::HistFrom2D(hChargeAvg2D_wrtMean, 0, 3, "ChargeAvg1D_wrtMean", 50, true);
  hChargeAvg1D_wrtMean->SetDirectory(&fOutRoot);
  hChargeAvg1D_wrtMean->SetTitle("Charge w.r.t. Mean Charge");
  hChargeAvg1D_wrtMean->SetXTitle("Charge w.r.t. Mean Charge");
  hChargeAvg1D_wrtMean->Draw("hist");
  cChargeAvg1D_wrtMean.SaveAs(TString(OutDir) + "/ChargeAvg1D_wrtMean.gif");


  // Let's look at the charge relative to 3x3 neighbors
  TCanvas cCharge3x3;
  cCharge3x3.cd();
  TH2F* hCharge3x3 = PLTU::Get3x3EfficiencyHist(hChargeAvg2D, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::FIRSTROW, PLTU::LASTROW);
  hCharge3x3->SetDirectory(&fOutRoot);
  hCharge3x3->SetTitle("Charge w.r.t. 3x3 neighbors");
  hCharge3x3->SetMinimum(0);
  hCharge3x3->SetMaximum(3);
  hCharge3x3->SetXTitle("Column");
  hCharge3x3->SetYTitle("Row");
  hCharge3x3->Draw("colz");
  cCharge3x3.SaveAs(TString(OutDir) + "/Charge3x3.gif");

  // And get the 1D distribution for the charge wrt 3x3
  TCanvas cCharge3x31D;
  cCharge3x31D.cd();
  TH1F* hCharge3x31D = PLTU::HistFrom2D(hCharge3x3, 0, 3, "Charge3x31D", 50, true);
  hCharge3x31D->SetDirectory(&fOutRoot);
  hCharge3x31D->SetTitle("Charge w.r.t. 3x3 neighbors 1D");
  hCharge3x31D->SetXTitle("Charge w.r.t. 3x3 neighbors 1D");
  hCharge3x31D->Draw("hist");
  cCharge3x31D.SaveAs(TString(OutDir) + "/Charge3x31D.gif");


  // Find Low and High charge collecting pixels
  TH2F* hLowChargePixels = (TH2F*) hCharge3x3->Clone("LowChargePixels");
  hLowChargePixels->Reset();
  hLowChargePixels->SetDirectory(&fOutRoot);
  TH2F* hHighChargePixels = (TH2F*) hCharge3x3->Clone("HighChargePixels");
  hHighChargePixels->Reset();
  hHighChargePixels->SetDirectory(&fOutRoot);
  int NumberOfLowChargePixels = 0;
  int NumberOfHighChargePixels = 0;
  for (int i = 1; i <= hCharge3x3->GetNbinsX(); ++i) {
    for (int j = 1; j <= hCharge3x3->GetNbinsY(); ++j) {
      if (hCharge3x3->GetBinContent(i, j) != 0 && hCharge3x3->GetBinContent(i, j) < 0.75 ) {
        hLowChargePixels->SetBinContent(i, j, 0.5);
        ++NumberOfLowChargePixels;
      } else if (hCharge3x3->GetBinContent(i, j) != 0 && hCharge3x3->GetBinContent(i, j) > 1.25 ) {
        hHighChargePixels->SetBinContent(i, j, 1);
        ++NumberOfHighChargePixels;
      }
    }
  }

  // Draw the Low and High charge pixels
  TCanvas cLowChargePixels;
  cLowChargePixels.cd();
  hLowChargePixels->SetTitle(TString::Format("Low Charge Pixels  nLow = %i", NumberOfLowChargePixels));
  hLowChargePixels->SetXTitle("Column");
  hLowChargePixels->SetYTitle("Row");
  hLowChargePixels->Draw("colz");
  cLowChargePixels.SaveAs(TString(OutDir) + "/LowChargePixels.gif");

  TCanvas cHighChargePixels;
  cHighChargePixels.cd();
  hHighChargePixels->SetTitle(TString::Format("High Charge Pixels  nHigh = %i", NumberOfHighChargePixels));
  hHighChargePixels->SetXTitle("Column");
  hHighChargePixels->SetYTitle("Row");
  hHighChargePixels->Draw("colz");
  cHighChargePixels.SaveAs(TString(OutDir) + "/HighChargePixels.gif");










  // Make the 1D Occupancy histogram
  TCanvas cOccupancy1D_All;
  cOccupancy1D_All.cd();
  TH1F* hOccupancy1D_All = PLTU::HistFrom2D(&hOccupancy, "Occupancy1D_All", 50, true);
  hOccupancy1D_All->SetDirectory(&fOutRoot);
  hOccupancy1D_All->Draw("hist");
  cOccupancy1D_All.SaveAs(TString(OutDir) + "/Occupancy1D_All.gif");

  // Grab the quantile you're interested in here
  Double_t QProbability[2] = { 0.0, 0.95 };
  Double_t QValue[2];
  hOccupancy1D_All->GetQuantiles(2, QValue, QProbability);
  float const XMinOccupancy = QValue[0];
  float const XMaxOccupancy = QValue[1];

  // Make the zoomed in plot of 1D occupancy
  TCanvas cOccupancy1D_Zoom;
  cOccupancy1D_Zoom.cd();
  TH1F* hOccupancy1D_Zoom = PLTU::HistFrom2D(&hOccupancy, XMinOccupancy, XMaxOccupancy, "Occupancy1D_Zoom", 50, true);
  hOccupancy1D_Zoom->SetDirectory(&fOutRoot);
  hOccupancy1D_Zoom->Draw("hist");
  cOccupancy1D_Zoom.SaveAs(TString(OutDir) + "/Occupancy1D_Zoom.gif");

  // Draw the 2D Occupancy plot
  TCanvas cOccupancy;
  cOccupancy.cd();
  hOccupancy.SetXTitle("Column");
  hOccupancy.SetYTitle("Row");
  hOccupancy.Draw("colz");
  cOccupancy.SaveAs(TString(OutDir) + "/Occupancy.gif");

  // Draw the 2D Occupancy plot Zoomed
  TCanvas cOccupancy_Zoom;
  cOccupancy_Zoom.cd();
  TH2F* hOccupancy_Zoom = (TH2F*) hOccupancy.Clone("Occupancy_Zoom");
  hOccupancy_Zoom->SetXTitle("Column");
  hOccupancy_Zoom->SetYTitle("Row");
  hOccupancy_Zoom->SetTitle("Occupancy Zoom");
  hOccupancy_Zoom->SetMinimum(XMinOccupancy);
  hOccupancy_Zoom->SetMaximum(XMaxOccupancy);
  hOccupancy_Zoom->Draw("colz");
  cOccupancy_Zoom.SaveAs(TString(OutDir) + "/Occupancy_Zoom.gif");


  // Make the 2D occupancy divided by mean
  TCanvas cOccupancy_wrtMean;
  cOccupancy_wrtMean.cd();
  TH2F* hOccupancy_wrtMean = (TH2F*) hOccupancy.Clone("Occupancy_wrtMean");
  hOccupancy_wrtMean->SetDirectory(&fOutRoot);
  hOccupancy_wrtMean->SetTitle("Occupancy wrt Mean Occupancy");
  float const MeanOccupancy = PLTU::GetMeanBinContentSkipEmptyBins(hOccupancy);
  hOccupancy_wrtMean->Scale(1.0 / MeanOccupancy);
  hOccupancy_wrtMean->SetMinimum(0);
  hOccupancy_wrtMean->SetMaximum(3);
  hOccupancy_wrtMean->SetXTitle("Column");
  hOccupancy_wrtMean->SetYTitle("Row");
  hOccupancy_wrtMean->Draw("colz");
  cOccupancy_wrtMean.SaveAs(TString(OutDir) + "/Occupancy_wrtMean.gif");

  // Make the 1D Occupancy histogram divided by the mean
  TCanvas cOccupancy1D_wrtMean;
  cOccupancy1D_wrtMean.cd();
  TH1F* hOccupancy1D_wrtMean = PLTU::HistFrom2D(hOccupancy_wrtMean, 0, 3, "Occupancy1D_wrtMean", 50, true);
  hOccupancy1D_wrtMean->SetDirectory(&fOutRoot);
  hOccupancy1D_wrtMean->SetXTitle("Occupancy w.r.t. Mean Occupancy");
  hOccupancy1D_wrtMean->Draw("hist");
  cOccupancy1D_wrtMean.SaveAs(TString(OutDir) + "/Occupancy1D_wrtMean.gif");



  // 3x3 Occupancy Efficiency
  TCanvas cOccupancy3x3Efficiency;
  cOccupancy3x3Efficiency.cd();
  TH2F* hOccupancy3x3Efficiency = PLTU::Get3x3EfficiencyHist(hOccupancy, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::FIRSTROW, PLTU::LASTROW);
  hOccupancy3x3Efficiency->SetDirectory(&fOutRoot);
  hOccupancy3x3Efficiency->SetTitle("Occupancy 3x3 Efficiency");
  hOccupancy3x3Efficiency->SetMinimum(0);
  hOccupancy3x3Efficiency->SetMaximum(3);
  hOccupancy3x3Efficiency->SetXTitle("Column");
  hOccupancy3x3Efficiency->SetYTitle("Row");
  hOccupancy3x3Efficiency->Draw("colz");
  cOccupancy3x3Efficiency.SaveAs(TString(OutDir) + "/Occupancy3x3Efficiency.gif");

  // 3x3 Occupancy Efficiency 1D histogram
  TCanvas cOccupancy3x3Efficiency_1D;
  cOccupancy3x3Efficiency_1D.cd();
  TH1F* hOccupancy3x3Efficiency_1D = PLTU::HistFrom2D(hOccupancy3x3Efficiency, 0, 3, "Occupancy3x3Efficiency_1D", 50, true);
  hOccupancy3x3Efficiency_1D->SetDirectory(&fOutRoot);
  hOccupancy3x3Efficiency_1D->SetTitle("Occupancy 3x3 Efficiency 1D");
  hOccupancy3x3Efficiency_1D->SetXTitle("Occupancy w.r.t. 3x3 Neighbors");
  hOccupancy3x3Efficiency_1D->Draw("hist");
  cOccupancy3x3Efficiency_1D.SaveAs(TString(OutDir) + "/Occupancy3x3Efficiency_1D.gif");


  // Dead pixels map
  TCanvas cDeadPixels;
  cDeadPixels.cd();
  TH2F* hDeadPixels = (TH2F*) hOccupancy.Clone("DeadPixels");
  hDeadPixels->Reset();
  int NumberOfDeadPixels = 0;
  for (int i = 1; i <= hOccupancy.GetNbinsX(); ++i) {
    for (int j = 1; j <= hOccupancy.GetNbinsY(); ++j) {
      if (hOccupancy.GetBinContent(i, j) == 0) {
        hDeadPixels->SetBinContent(i, j, 1);
        ++NumberOfDeadPixels;
      }
    }
  }
  hDeadPixels->SetMinimum(0);
  hDeadPixels->SetMaximum(1);
  hDeadPixels->SetTitle( TString::Format("Dead Pixel Map.  Number Dead = %i", NumberOfDeadPixels) );
  hDeadPixels->SetXTitle("Column");
  hDeadPixels->SetYTitle("Row");
  hDeadPixels->Draw("colz");
  cDeadPixels.SaveAs(TString(OutDir) + "/DeadPixels.gif");


  // Low Efficiency w.r.t. neighbors pixel map
  TCanvas cLowEfficiencyPixels;
  cLowEfficiencyPixels.cd();
  TH2F* hLowEfficiencyPixels = (TH2F*) hOccupancy3x3Efficiency->Clone("LowEfficiencyPixels");
  hLowEfficiencyPixels->SetDirectory(&fOutRoot);
  hLowEfficiencyPixels->Reset();
  int NumberOfLowEfficiencyPixels = 0;
  for (int i = 1; i <= hOccupancy3x3Efficiency->GetNbinsX(); ++i) {
    for (int j = 1; j <= hOccupancy3x3Efficiency->GetNbinsY(); ++j) {
      if (hOccupancy3x3Efficiency->GetBinContent(i, j) != 0 && hOccupancy3x3Efficiency->GetBinContent(i, j) < 0.75 ) {
        hLowEfficiencyPixels->SetBinContent(i, j, 0.5);
        ++NumberOfLowEfficiencyPixels;
      }
    }
  }
  hLowEfficiencyPixels->SetMinimum(0);
  hLowEfficiencyPixels->SetMaximum(1);
  hLowEfficiencyPixels->SetTitle( TString::Format("Low Efficiency Pixel Map.  Number Low = %i", NumberOfLowEfficiencyPixels) );
  hLowEfficiencyPixels->SetXTitle("Column");
  hLowEfficiencyPixels->SetYTitle("Row");
  hLowEfficiencyPixels->Draw("colz");
  cLowEfficiencyPixels.SaveAs(TString(OutDir) + "/LowEfficiencyPixels.gif");


  fOutRoot.Write();
  fOutRoot.Close();
  OutFile.close();


  WriteHTML(OutDir);

  return 0;
}









void WriteHTML (TString const OutDirName)
{
  // This function writes the HTML in the output folder

  std::ofstream f(OutDirName + "/index.html");
  if (!f.is_open()) {
    std::cerr << "ERROR: cannot open output file for writing: " << OutDirName << "/index.html" << std::endl;
    throw;
  }

  f << "<html>\n";
  f << "<body>\n";


  f << "<img width=\"300\" src=\"CL_LevelsROC.gif\"><img width=\"300\" src=\"CL_LevelsTBM.gif\"><br>\n";
  f << "<img width=\"300\" src=\"CL_LevelsROCUB.gif\"><img width=\"300\" src=\"CL_LevelsTBMUB.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"Occupancy.gif\"><img width=\"300\" src=\"Occupancy_Zoom.gif\"><img width=\"300\" src=\"Occupancy_wrtMean.gif\"><br>\n";
  f << "<img width=\"300\" src=\"Occupancy1D_All.gif\"><img width=\"300\" src=\"Occupancy1D_Zoom.gif\"><img width=\"300\" src=\"Occupancy1D_wrtMean.gif\"><br>\n";




  f << "<hr>\n";
  f << "<img width=\"300\" src=\"Occupancy3x3Efficiency.gif\"><img width=\"300\" src=\"DeadPixels.gif\"><br>\n";
  f << "<img width=\"300\" src=\"Occupancy3x3Efficiency_1D.gif\"><img width=\"300\" src=\"LowEfficiencyPixels.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"PulseHeightAvg.gif\"><img width=\"300\" src=\"PulseHeight.gif\"><img width=\"300\" src=\"PHVsTime.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"ChargeAvg.gif\"><img width=\"300\" src=\"ChargeAvg2D_wrtMean.gif\"><br>\n";
  f << "<img width=\"300\" src=\"ChargeAvg1D.gif\"><img width=\"300\" src=\"ChargeAvg1D_wrtMean.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"Charge3x3.gif\"><img width=\"300\" src=\"LowChargePixels.gif\"><br>\n";
  f << "<img width=\"300\" src=\"Charge3x31D.gif\"><img width=\"300\" src=\"HighChargePixels.gif\"><br>\n";



  f.close();

  return;
}










int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [GainCalFileName] [OutDir]" << std::endl;
    return 1;
  }

  TestStandTest(argv[1], argv[2], argv[3]);

  return 0;
}
