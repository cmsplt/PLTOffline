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
#include "TFitResult.h"
#include "TH2F.h"
#include "TDatime.h"
#include "TLegend.h"
#include "TLegendEntry.h"



// THIS IS THE BASE PATH!!!!!
// Please put a trailing '/'
TString const kDATAPATH = "/Users/dhidas/TeststandData/";
TString const kOUTPUTPATH = "/Users/dhidas/TeststandData/output/";





void WriteHTML_Analysis(TString const, TString const, TString const, TString const);
void WriteHTML_Calibration (TString const, TString const, TString const);



int TestStandTest (TString const PlaneNumber, TString const RunNumber, TString const CalibrationNumber)
{
//    std::string const DataFileName, std::string const GainCalFileName, std::string const OutDir)

  // Input data file
  TString const DataFileName = TString::Format("%s/data/sr90-s%s-run%s.dat", kDATAPATH.Data(), PlaneNumber.Data(), RunNumber.Data());

  // Get rootfile name and make directory for output files
  TString const OutDir = TString::Format(kOUTPUTPATH + PlaneNumber + "/analysis/" + RunNumber + "/");
  std::cout << "Output will be sent to: " << OutDir << std::endl;

  // gaincal file name
  TString const GainCalFileName = TString::Format(kOUTPUTPATH + "%s/calibration/%s/calibration_coeficients_s%s_run%s.txt", PlaneNumber.Data(), CalibrationNumber.Data(), PlaneNumber.Data(), CalibrationNumber.Data());
  std::cout << "GainCal filename: " << GainCalFileName << std::endl;

  // Output root file
  TString const OutRootFileName = OutDir + "/output_s" + PlaneNumber + "_run" + RunNumber + ".root";
  gSystem->mkdir(OutDir.Data(), true);


  // Some basic graphics style
  PLTU::SetStyle();

  // Open the root file and output file
  TFile fOutRoot(OutRootFileName, "recreate");
  std::ofstream OutFile("TestOut.txt");

  // Initalize the Event object and decide what kind of clustering you want
  PLTTesterEvent Event(DataFileName.Data(), GainCalFileName.Data(), OutDir.Data(), &fOutRoot);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_Diamond);


  // Bins and max for pulse height plots
  int   const NBins =     60;
  float const XMin  =  -1000;
  float const XMax  =  50000;


  // Define occupancy plot and attach it to the root file
  TH2F hOccupancy("Occupancy", "Occupancy", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
  hOccupancy.SetDirectory(&fOutRoot);

  // Define one and two pixel occupancy histograms and attach it to the root file
  TH2F hOccupancy1Pix("Occupancy1Pix", "Occupancy for 1-pixel Clusters", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
  hOccupancy1Pix.SetDirectory(&fOutRoot);

  // Define one and two pixel occupancy histograms and attach it to the root file
  TH2F hOccupancy2Pix("Occupancy2Pix", "Occupancy for 2-piexl Clusters", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
  hOccupancy2Pix.SetDirectory(&fOutRoot);

  // Define histogram for highest charge pixel in each event
  TH1F hHighestChargePixel("HighestChargePixel", "Highest Charge Pixel", 50, XMin, XMax);
  hHighestChargePixel.SetDirectory(&fOutRoot);



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
  hPulseHeight[0] = new TH1F("PulseHeight_All", "Pulse Height", NBins, XMin, XMax);
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

      // Fill 1 and 2 pixel occupancy histograms
      if (Cluster->NHits() == 1) {
        hOccupancy1Pix.Fill(Cluster->Hit(0)->Column(), Cluster->Hit(0)->Row());
      } else if (Cluster->NHits() == 2) {
        hOccupancy2Pix.Fill(Cluster->Hit(0)->Column(), Cluster->Hit(0)->Row());
        hOccupancy2Pix.Fill(Cluster->Hit(1)->Column(), Cluster->Hit(1)->Row());
      }

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



    // For the highest charge pixel in each event
    float HighestChargePixel = -999999999;

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

      // If this hit has more charge than a previous one, save the charge
      if (ThisHitCharge > HighestChargePixel) {
        HighestChargePixel = ThisHitCharge;
      }

    }

    // Fill histogram with highest charge pixel
    if (Event.NHits() > 0) {
      hHighestChargePixel.Fill(HighestChargePixel);
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
  hPulseHeight[0]->SetStats(kFALSE);
  hPulseHeight[0]->GetXaxis()->SetTitle("PH (electrons)");
  hPulseHeight[0]->Draw("Hist");
  hPulseHeight[1]->Draw("SameHist");
  hPulseHeight[2]->Draw("SameHist");
  hPulseHeight[3]->Draw("SameHist");
  TLegend lPulseHeight(0.6, 0.6, 0.82, 0.9, "Mean:");
  lPulseHeight.SetTextAlign(32);
  lPulseHeight.SetFillStyle(0);
  lPulseHeight.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeight[0]->GetMean()), "")->SetTextColor(HistColors[0]);
  lPulseHeight.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeight[1]->GetMean()), "")->SetTextColor(HistColors[1]);
  lPulseHeight.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeight[2]->GetMean()), "")->SetTextColor(HistColors[2]);
  lPulseHeight.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeight[3]->GetMean()), "")->SetTextColor(HistColors[3]);
  lPulseHeight.Draw("same");
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





  // Draw the highest charge pixel charge distribution
  TCanvas cHighestChargePixel;
  cHighestChargePixel.cd();
  hHighestChargePixel.SetXTitle("Charge (electrons)");
  hHighestChargePixel.Draw("hist");
  cHighestChargePixel.SaveAs(TString(OutDir) + "/HighestChargePixel.gif");








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




  // One pixel occupancy plot
  TCanvas cOccupancy1Pix;
  cOccupancy1Pix.cd();
  hOccupancy1Pix.SetXTitle("Column");
  hOccupancy1Pix.SetYTitle("Row");
  hOccupancy1Pix.Draw("colz");
  cOccupancy1Pix.SaveAs(TString(OutDir) + "/Occupancy1Pix.gif");

  // The 1D distribution for 1-pix clusters
  TCanvas cOccupancy1Pix1D;
  cOccupancy1Pix1D.cd();
  TH1F* hOccupancy1Pix1D = PLTU::HistFrom2D(&hOccupancy1Pix, "Occupancy1Pix1D", 50, true);
  hOccupancy1Pix1D->SetDirectory(&fOutRoot);
  hOccupancy1Pix1D->SetTitle("Pixel Occupancy 1-pixel Clusters");
  hOccupancy1Pix1D->Draw("hist");
  cOccupancy1Pix1D.SaveAs(TString(OutDir) + "/Occupancy1Pix1D.gif");



  // Two pixel occupancy plot
  TCanvas cOccupancy2Pix;
  cOccupancy2Pix.cd();
  hOccupancy2Pix.SetXTitle("Column");
  hOccupancy2Pix.SetYTitle("Row");
  hOccupancy2Pix.Draw("colz");
  cOccupancy2Pix.SaveAs(TString(OutDir) + "/Occupancy2Pix.gif");

  // The 1D distribution for 1-pix clusters
  TCanvas cOccupancy2Pix1D;
  cOccupancy2Pix1D.cd();
  TH1F* hOccupancy2Pix1D = PLTU::HistFrom2D(&hOccupancy2Pix, "Occupancy2Pix1D", 50, true);
  hOccupancy2Pix1D->SetDirectory(&fOutRoot);
  hOccupancy2Pix1D->SetTitle("Pixel Occupancy 2-pixel Clusters");
  hOccupancy2Pix1D->Draw("hist");
  cOccupancy2Pix1D.SaveAs(TString(OutDir) + "/Occupancy2Pix1D.gif");




  // Ration of One pixel clusters to Two pixel clusters
  TCanvas cOccupancyRatio2to1;
  cOccupancyRatio2to1.cd();
  TH2F* hOccupancyRatio2to1 = (TH2F*) hOccupancy2Pix.Clone("OccupancyRatio2to1");
  hOccupancyRatio2to1->SetDirectory(&fOutRoot);
  hOccupancyRatio2to1->SetTitle("Ratio of 2-pixel to 1-pixel clusters");
  hOccupancyRatio2to1->Divide(&hOccupancy1Pix);
  hOccupancyRatio2to1->SetMinimum(0);
  hOccupancyRatio2to1->SetMaximum(3);
  hOccupancyRatio2to1->Draw("colz");
  cOccupancyRatio2to1.SaveAs(TString(OutDir) + "/OccupancyRatio2to1.gif");

  // The 1D distribution for the ratio of 1,2 pixel clusters
  TCanvas cOccupancyRatio2to11D;
  cOccupancyRatio2to11D.cd();
  TH1F* hOccupancyRatio2to11D = PLTU::HistFrom2D(hOccupancyRatio2to1, "OccupancyRatio2to11D", 50, true);
  hOccupancyRatio2to11D->SetDirectory(&fOutRoot);
  hOccupancyRatio2to11D->SetTitle("Ration of 2 to 1 Pixel Clusters");
  hOccupancyRatio2to11D->Draw("hist");
  cOccupancyRatio2to11D.SaveAs(TString(OutDir) + "/OccupancyRatio2to11D.gif");













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


  WriteHTML_Analysis(OutDir, PlaneNumber, RunNumber, CalibrationNumber);

  return 0;
}









void WriteHTML_Analysis (TString const OutDirName, TString const PlaneNumber, TString const RunNumber, TString const CalibrationNumber)
{
  // This function writes the HTML in the output folder

  std::ofstream f(OutDirName + "/index.html");
  if (!f.is_open()) {
    std::cerr << "ERROR: cannot open output file for writing: " << OutDirName << "/index.html" << std::endl;
    throw;
  }

  f << "<html>\n";
  f << "<body>\n";

  f << "<h1>Analysis - Plane s" << PlaneNumber << " Run " << RunNumber << " using calibration run " << CalibrationNumber << "</h1><br>\n";

  f << "<a href=\"../../calibration/" << CalibrationNumber << "/index.html\">Calibration reference: Run " << CalibrationNumber << "</a><br>\n";
  f << "<a href=\"output_s" << PlaneNumber << "_run" << RunNumber << ".root\">Analysis Root File</a><br>\n";

  f << "<img width=\"300\" src=\"CL_LevelsROC.gif\"><img width=\"300\" src=\"CL_LevelsTBM.gif\"><br>\n";
  f << "<img width=\"300\" src=\"CL_LevelsROCUB.gif\"><img width=\"300\" src=\"CL_LevelsTBMUB.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"Occupancy.gif\"><img width=\"300\" src=\"Occupancy_Zoom.gif\"><img width=\"300\" src=\"Occupancy_wrtMean.gif\"><br>\n";
  f << "<img width=\"300\" src=\"Occupancy1D_All.gif\"><img width=\"300\" src=\"Occupancy1D_Zoom.gif\"><img width=\"300\" src=\"Occupancy1D_wrtMean.gif\"><br>\n";
  f << "<br /><br />\n";
  f << "<img width=\"300\" src=\"Occupancy1Pix.gif\"><img width=\"300\" src=\"Occupancy2Pix.gif\"><img width=\"300\" src=\"OccupancyRatio2to1.gif\"><br>\n";
  f << "<img width=\"300\" src=\"Occupancy1Pix1D.gif\"><img width=\"300\" src=\"Occupancy2Pix1D.gif\"><img width=\"300\" src=\"OccupancyRatio2to11D.gif\"><br>\n";




  f << "<hr>\n";
  f << "<img width=\"300\" src=\"Occupancy3x3Efficiency.gif\"><img width=\"300\" src=\"DeadPixels.gif\"><br>\n";
  f << "<img width=\"300\" src=\"Occupancy3x3Efficiency_1D.gif\"><img width=\"300\" src=\"LowEfficiencyPixels.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"PulseHeightAvg.gif\"><img width=\"300\" src=\"PulseHeight.gif\"><br>\n";
  f << "<img width=\"300\" src=\"PHVsTime.gif\"><img width=\"300\" src=\"HighestChargePixel.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"ChargeAvg.gif\"><img width=\"300\" src=\"ChargeAvg2D_wrtMean.gif\"><br>\n";
  f << "<img width=\"300\" src=\"ChargeAvg1D.gif\"><img width=\"300\" src=\"ChargeAvg1D_wrtMean.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"Charge3x3.gif\"><img width=\"300\" src=\"LowChargePixels.gif\"><br>\n";
  f << "<img width=\"300\" src=\"Charge3x31D.gif\"><img width=\"300\" src=\"HighChargePixels.gif\"><br>\n";


  TDatime d;
  f << "<address>Created on: " << d.AsSQLString() << "</address>\n";


  f << "</body>\n";
  f << "<hhtml>\n";


  f.close();

  return;
}





void GainCalibrationAnalysisAndFits (TString const PlaneNumber, TString const RunNumber)
{
  // This function will produce the calibration coefficients (gain cal)
  // text file and root file.

  // Get rootfile name and make directory for output files
  TString const OutDir = TString::Format(kOUTPUTPATH + PlaneNumber + "/calibration/" + RunNumber + "/");
  std::cout << "Output will be sent to: " << OutDir << std::endl;

  // Output root file
  TString const OutRootFileName = OutDir + "/calibration_s" + PlaneNumber + "_run" + RunNumber + ".root";
  gSystem->mkdir(OutDir.Data(), true);

  // Input data file
  TString const DataFileName = TString::Format("%s/data/vcal-s%s-run%s.dat", kDATAPATH.Data(), PlaneNumber.Data(), RunNumber.Data());

  // Some basic graphics style
  PLTU::SetStyle();

  // Open the root file and output file
  TFile fOutRoot(OutRootFileName, "recreate");
  FILE* OutFile = fopen(TString::Format(OutDir + "/calibration_coeficients_s%s_run%s.txt", PlaneNumber.Data(), RunNumber.Data()).Data(), "w");

  //

  // Initalize the Event object and decide what kind of clustering you want
  PLTTesterEvent Event(DataFileName.Data(), "", OutDir, &fOutRoot);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_Diamond);



  // For keeping track of gain cal vcal and PH information
  // I'm using PLTU::LASTCOL etc just to be safe for the indices
  // The 'key' for the map is col*100+row
  //std::map<int, double> GainAvg[PLTU::LASTCOL][PLTU::LASTROW];
  //std::map<int, int>      GainN[PLTU::LASTCOL][PLTU::LASTROW];
  int const MAXCOL = 52;
  int const MAXROW = 80;

  std::map<int, double> GainAvg[MAXCOL][MAXROW];
  std::map<int, int>      GainN[MAXCOL][MAXROW];

  std::map<int, TGraph> VCalVsADCGraph;

  // This is the main event loop
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << " time: " << Event.Time() << std::endl;
    }

    for (size_t ihit = 0; ihit != Event.NHits(); ++ihit) {
      PLTHit* Hit = Event.Hit(ihit);

      // Just check that this pixel is within the array limits
      if (Hit->Column() >= MAXCOL || Hit->Row() >= MAXROW) {
        printf("Skipping: This pixel is out of bounds: %2i %2i\n", Hit->Column(), Hit->Row());
        continue;
      }

      // Add this hit to the running average for this vcal
      PLTU::AddToRunningAverage(GainAvg[ Hit->Column()][Hit->Row()][Event.VCal()], GainN[Hit->Column()][Hit->Row()][Event.VCal()], (double) Hit->ADC());

      // Add a point for this hit on the graph of all readouts
      int const id = Hit->Column() * 100 + Hit->Row();
      int const NPoints = VCalVsADCGraph[id].GetN() + 1;
      VCalVsADCGraph[id].Set(NPoints);
      VCalVsADCGraph[id].SetPoint(NPoints - 1, Hit->ADC(), Event.VCal());
    }


  } // END MAIN EVENT LOOP




  // Plot for the Chi2 and Chi2/ndf of fits
  TH2F hChi2ndf("chi2ndf_2d", "Chi2/ndf", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
  TH2F hChi2("chi2_2d", "Chi2", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
  hChi2.SetDirectory(&fOutRoot);
  hChi2ndf.SetDirectory(&fOutRoot);


  // Now time to do the fitting!
  for (int i = 0; i != MAXCOL; ++i) {
    for (int j = 0; j != MAXROW; ++j) {
      if (GainAvg[i][j].empty()) {
        continue;
      }

      // This graph is a graph of the average of readouts.  Used for the gaincal fit
      TGraphErrors g((int) GainAvg[i][j].size());
      g.SetName( TString::Format("ic_%i_ir_%i_adc_vs_vcal_graph", i, j).Data() );

      // We'll record the min and max adc to try to gauge where the fit range should be from and to
      float ADCMin = 99999;
      float ADCMax = 0;

      // p is just to count the number of points in this graph
      int p = 0;

      // Loop over all vcal entries for this pixel
      for (std::map<int, double>::iterator It = GainAvg[i][j].begin(); It != GainAvg[i][j].end(); ++It) {

        // Set this points in this graph
        g.SetPoint(p++, It->second, It->first);

        // Record me if I am a min or a max
        if (It->second > ADCMax) {
          ADCMax = It->second;
        }
        if (It->second < ADCMin) {
          ADCMin = It->second;
        }
      }

      // Here we fit the resulting graph with a 3-parameter function
      TString const FitName = TString::Format("f1_%i_%i", i, j);
      TF1 FitFunction(FitName, "[0]+[1]*x+[2]*x*x", 1000, 5000);
      FitFunction.SetParameter(0, 2.51747e+06);
      FitFunction.SetParameter(1, -1.80594e+03);
      FitFunction.SetParameter(2, 3.24677e-01);
      TFitResultPtr FitResultPtr = g.Fit(FitName, "QS", "", ADCMin, ADCMin + 0.7 * (ADCMax - ADCMin));
      TFitResult* FitResult = FitResultPtr.Get();

      // Grab the Chi2 values and add them to the 2D plots
      hChi2ndf.Fill(i, j, FitResult->Chi2() / FitResult->Ndf());
      hChi2.Fill(i, j, FitResult->Chi2());

      // Make sure to save this graph (with it) to the root file
      g.Write();

      // Make a plot of the residuals for the points and fit
      TH1F hResiduals( TString::Format("ic_%i_ir_%i_residual", i, j), TString::Format("ic_%i_ir_%i_residual", i, j), 100, -50, 50);
      hResiduals.SetDirectory(&fOutRoot);
      int const NPoints = g.GetN();
      Double_t x, y;
      for (int ip = 0; ip != NPoints; ++ip) {
        g.GetPoint(ip, x, y);
        hResiduals.Fill( y - FitFunction.Eval(x) );
      }
      hResiduals.Write(); // I should not have to do this...

      // Print the parameters from the fit to the calibration coficients file
      float const P0 = FitFunction.GetParameter(0);
      float const P1 = FitFunction.GetParameter(1);
      float const P2 = FitFunction.GetParameter(2);
      fprintf(OutFile,"%d %d %f %f %f\n", i, j, P0, P1, P2);
    }
  }



  // This makes levels histograms
  Event.MakePlots();


  // Draw Chi2 distributions
  TCanvas cChi2;
  cChi2.cd();
  hChi2.SetXTitle("Column");
  hChi2.SetYTitle("Row");
  hChi2.SetMaximum(3000);
  hChi2.Draw("colz");
  cChi2.SaveAs(OutDir + "/Chi2.gif");

  // And the 1D distribution
  TCanvas cChi21D;
  cChi21D.cd();
  TH1F* hChi21D = PLTU::HistFrom2D(&hChi2, 0, 3000, "Chi21D", 300, true);
  hChi21D->SetDirectory(&fOutRoot);
  hChi21D->SetTitle("Chi2/ndf");
  hChi21D->SetXTitle("Chi2/ndf");
  hChi21D->Draw("hist");
  cChi21D.SaveAs(OutDir + "/Chi21D.gif");


  // Draw Chi2 distributions
  TCanvas cChi2ndf;
  cChi2ndf.cd();
  hChi2ndf.SetXTitle("Column");
  hChi2ndf.SetYTitle("Row");
  hChi2ndf.SetMaximum(100);
  hChi2ndf.Draw("colz");
  cChi2ndf.SaveAs(OutDir + "/Chi2ndf.gif");

  // And the 1D distribution
  TCanvas cChi2ndf1D;
  cChi2ndf1D.cd();
  TH1F* hChi2ndf1D = PLTU::HistFrom2D(&hChi2ndf, 0, 100, "Chi2ndf1D", 100, true);
  hChi2ndf1D->SetDirectory(&fOutRoot);
  hChi2ndf1D->SetTitle("Chi2ndf/ndf");
  hChi2ndf1D->SetXTitle("Chi2ndf/ndf");
  hChi2ndf1D->Draw("hist");
  cChi2ndf1D.SaveAs(OutDir + "/Chi2ndf1D.gif");



  // Save each adc vcal graph and calculate residual plot for each
  for (std::map<int, TGraph>::iterator It = VCalVsADCGraph.begin(); It != VCalVsADCGraph.end(); ++It) {

    // Get Column and Row
    int const col = It->first / 100;
    int const row = It->first % 100;

    // Grab the graph
    TGraph& Graph = It->second;

    // Name the graph
    Graph.SetName( TString::Format("ic_%i_ir_%i_adc_vs_vcal", col, row) );
    Graph.SetTitle( TString::Format("ADC vs VCal: Col %i  Row %i", col, row) );

    Graph.Write();

  }



  // close the output text file if we're all done
  fclose(OutFile);

  WriteHTML_Calibration(OutDir, PlaneNumber, RunNumber);

  fOutRoot.Write();

  return;
}









void WriteHTML_Calibration (TString const OutDirName, TString const PlaneNumber, TString const RunNumber)
{
  // This function writes the HTML in the output folder

  std::ofstream f(OutDirName + "/index.html");
  if (!f.is_open()) {
    std::cerr << "ERROR: cannot open output file for writing: " << OutDirName << "/index.html" << std::endl;
    throw;
  }

  f << "<html>\n";
  f << "<body>\n";

  f << "<h1>Calibration - Plane s" << PlaneNumber << " Run " << RunNumber << "</h1><br>\n";

  f << "<a href=\"calibration_coeficients_s" << PlaneNumber << "_run" << RunNumber << ".txt\">Calibration Coeficients</a><br>\n";
  f << "<a href=\"calibration_s" << PlaneNumber << "_run" << RunNumber << ".root\">Calibration Root File</a><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"CL_LevelsROC.gif\"><img width=\"300\" src=\"CL_LevelsTBM.gif\"><br>\n";
  f << "<img width=\"300\" src=\"CL_LevelsROCUB.gif\"><img width=\"300\" src=\"CL_LevelsTBMUB.gif\"><br>\n";



  f << "<hr>\n";
  f << "<img width=\"300\" src=\"Chi2.gif\"><img width=\"300\" src=\"Chi2ndf.gif\"><br>\n";
  f << "<img width=\"300\" src=\"Chi21D.gif\"><img width=\"300\" src=\"Chi2ndf1D.gif\"><br>\n";

  TDatime d;
  f << "<address>Created on: " << d.AsSQLString() << "</address>\n";


  f << "</body>\n";
  f << "</html>\n";

  return;
}









int main (int argc, char* argv[])
{
  if (argc == 4 && TString(argv[1]) == "calibration") {
    printf("I think you want to do a calibration of plane s%s calibration run %s\n", argv[2], argv[3]);
    GainCalibrationAnalysisAndFits(argv[2], argv[3]);
  } else if (argc == 5 && TString(argv[1]) == "analysis") {
    printf("I think you want run the analysis on plane s%s run %s using calibration run %s\n", argv[2], argv[3], argv[4]);
    TestStandTest(argv[2], argv[3], argv[4]);
  } else {
    std::cerr << "Usage: " << argv[0] << " [calibration] [plane #] [run #]" << std::endl;
    std::cerr << "Usage: " << argv[0] << " [analysis]    [plane #] [run #] [calibration run #]" << std::endl;
    return 1;
  }

  //TestStandTest(argv[1], argv[2], argv[3]);

  return 0;
}
