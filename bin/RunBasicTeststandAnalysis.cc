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
  TH2I hOccupancy("Occupancy", "Occupancy", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
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

    if (ThisTime < EndTimeWindow) {
      // This time is still in the current time window

      // Loop over all clusters
      for (size_t icluster = 0; icluster != Event.NClusters(); ++icluster) {

        // Grab this cluster
        PLTCluster* Cluster = Event.Cluster(icluster);

        // Get cluster charge
        float const Charge = Cluster->Charge();

        // If this charge is something crazy we skip it
        if (Charge > 1000000 || Charge < -1000000) {
          continue;
        }

        // Just out of curosity print zero-charge clusters
        if (Charge == 0) {
          printf("Cluster has zero charge:   NHits: %3i  Column: %2i  Row: %2i\n", (int) Cluster->NHits(), Cluster->Hit(0)->Column(), Cluster->Hit(0)->Row());
          continue;
        }

        // Number of hits in this cluster
        int const NHitsInCluster = Cluster->NHits();

        // Fill in PH histogram
        hPulseHeight[0]->Fill(Charge);
        NHitsInCluster > 2 ? hPulseHeight[3]->Fill(Charge) : hPulseHeight[NHitsInCluster]->Fill(Charge);

        // Add to running averages for the PH vs time plots
        PLTU::AddToRunningAverage(PulseHeightTimeAvg[0], PulseHeightTimeN[0], Charge);
        NHitsInCluster > 2 ?    PLTU::AddToRunningAverage(PulseHeightTimeAvg[3], PulseHeightTimeN[3], Charge) :
                                PLTU::AddToRunningAverage(PulseHeightTimeAvg[NHitsInCluster], PulseHeightTimeN[NHitsInCluster], Charge);
      }

    } else {
      // We have come to the end of this time window

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

  TCanvas cOccupancy;
  cOccupancy.cd();
  hOccupancy.Draw("colz");
  cOccupancy.SaveAs(TString(OutDir) + "/Occupancy.gif");



  fOutRoot.Write();
  fOutRoot.Close();
  OutFile.close();

  return 0;
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
