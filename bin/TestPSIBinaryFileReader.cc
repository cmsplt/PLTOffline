////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Apr  9 13:49:09 CEST 2014
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cmath>
#include <stdlib.h>


#include "PSIBinaryFileReader.h"
#include "PLTPlane.h"
#include "PLTAlignment.h"

#include "TLegend.h"
#include "TLegendEntry.h"
#include "TString.h"
#include "TSystem.h"
#include "TFile.h"
#include "TGraphErrors.h"



void WriteHTML (TString const, TString const);

int TestPSIBinaryFileReader (std::string const InFileName, std::string const CalibrationList, TString const RunNumber)
{



  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";

  TFile out_f( OutDir + "histos.root","new");
  
  std::cout<<OutDir<<std::endl;
  /* TestPSIBinaryFileReaderAlign: Default run analysis.
  */

  gStyle->SetOptStat(0);

  // Open Alignment
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope.dat");

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName, CalibrationList);
  BFR.SetTrackingAlignment(&Alignment);
  //BFR.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  FILE* f = fopen("MyGainCal.dat", "w");
  BFR.GetGainCal()->PrintGainCal(f);
  fclose(f);
  BFR.CalculateLevels(10000, OutDir);

  // Prepare Occupancy histograms
  // x == columns
  // y == rows
  std::vector< TH2F > hOccupancy;
  for (int iroc = 0; iroc != 6; ++iroc){
    hOccupancy.push_back( TH2F( Form("Occupancy_ROC%i",iroc),
                                Form("Occupancy_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
  }

  // Track slope plots
  TH1F hTrackSlopeX("TrackSlopeX", "TrackSlopeX", 50, -0.05, 0.05);
  TH1F hTrackSlopeY("TrackSlopeY", "TrackSlopeY", 50, -0.05, 0.05);

  std::vector<TH2F> hOccupancyTrack6;
  for (int iroc = 0; iroc != 6; ++iroc){
    hOccupancyTrack6.push_back( TH2F( Form("OccupancyTrack6_ROC%i",iroc),
                                Form("OccupancyTrack6_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
  }

  std::vector<TH2F> hOccupancyLowPH;
  for (int iroc = 0; iroc != 6; ++iroc){
    hOccupancyLowPH.push_back( TH2F( Form("OccupancyLowPH_ROC%i",iroc),
                                Form("OccupancyLowPH_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
  }
  std::vector<TH2F> hOccupancyHighPH;
  for (int iroc = 0; iroc != 6; ++iroc){
    hOccupancyHighPH.push_back( TH2F( Form("OccupancyHighPH_ROC%i",iroc),
                                Form("OccupancyHighPH_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
  }

  std::vector<TH2F> hOccupancyHot;
  for (int iroc = 0; iroc != 6; ++iroc){
    hOccupancyHot.push_back( TH2F( Form("OccupancyHot_ROC%i",iroc),
				      Form("OccupancyHot_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
  }

  std::vector<TH1F> hNHitsPerCluster;
  for (int iroc = 0; iroc != 6; ++iroc){
    hNHitsPerCluster.push_back( TH1F( Form("NHitsPerCluster_ROC%i",iroc),
                                Form("NHitsPerCluster_ROC%i",iroc), 10, 0, 10));
  }

  std::vector<TH1F> hNClusters;
  for (int iroc = 0; iroc != 6; ++iroc){
    hNClusters.push_back( TH1F( Form("NClusters_ROC%i",iroc),
                                Form("NClusters_ROC%i",iroc), 10, 0, 10));
  }


  // Coincidence histogram
  TH1F hCoincidenceMap("CoincidenceMap", "CoincidenceMap", 0x3f, 0, 0x3f);
  char *bin[0x40] = {
      (char*)"000000"
    , (char*)"000001"
    , (char*)"000010"
    , (char*)"000011"
    , (char*)"000100"
    , (char*)"000101"
    , (char*)"000110"
    , (char*)"000111"
    , (char*)"001000"
    , (char*)"001001"
    , (char*)"001010"
    , (char*)"001011"
    , (char*)"001100"
    , (char*)"001101"
    , (char*)"001110"
    , (char*)"001111"
    , (char*)"010000"
    , (char*)"010001"
    , (char*)"010010"
    , (char*)"010011"
    , (char*)"010100"
    , (char*)"010101"
    , (char*)"010110"
    , (char*)"010111"
    , (char*)"011000"
    , (char*)"011001"
    , (char*)"011010"
    , (char*)"011011"
    , (char*)"011100"
    , (char*)"011101"
    , (char*)"011110"
    , (char*)"011111"
    , (char*)"100000"
    , (char*)"100001"
    , (char*)"100010"
    , (char*)"100011"
    , (char*)"100100"
    , (char*)"100101"
    , (char*)"100110"
    , (char*)"100111"
    , (char*)"101000"
    , (char*)"101001"
    , (char*)"101010"
    , (char*)"101011"
    , (char*)"101100"
    , (char*)"101101"
    , (char*)"101110"
    , (char*)"101111"
    , (char*)"110000"
    , (char*)"110001"
    , (char*)"110010"
    , (char*)"110011"
    , (char*)"110100"
    , (char*)"110101"
    , (char*)"110110"
    , (char*)"110111"
    , (char*)"111000"
    , (char*)"111001"
    , (char*)"111010"
    , (char*)"111011"
    , (char*)"111100"
    , (char*)"111101"
    , (char*)"111110"
    , (char*)"111111"
  };
  hCoincidenceMap.SetBit(TH1::kCanRebin);
  for (int r = 0; r < 0x40; ++r) 
  {
    hCoincidenceMap.Fill(bin[r], 0);
  }

  hCoincidenceMap.LabelsDeflate();
  hCoincidenceMap.SetFillColor(40);
  hCoincidenceMap.SetYTitle("Number of Hits");
  hCoincidenceMap.GetYaxis()->SetTitleOffset(1.9);
  hCoincidenceMap.GetYaxis()->CenterTitle();

  // Prepare PulseHeight histograms
  TH1F* hPulseHeight[6][4];
  int const phMin = 0;
  int const phMax = 50000;
  int const phNBins = 50;
  for (int iroc = 0; iroc != 6; ++iroc) {
    TString Name = TString::Format("PulseHeight_ROC%i_All", iroc);
    hPulseHeight[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeight_ROC%i_NPix1", iroc);
    hPulseHeight[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeight_ROC%i_NPix2", iroc);
    hPulseHeight[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeight_ROC%i_NPix3Plus", iroc);
    hPulseHeight[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phMax);
  }
  TH1F* hPulseHeightTrack6[6][4];
  for (int iroc = 0; iroc != 6; ++iroc) {
    TString Name = TString::Format("PulseHeightTrack6_ROC%i_All", iroc);
    hPulseHeightTrack6[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightTrack6_ROC%i_NPix1", iroc);
    hPulseHeightTrack6[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightTrack6_ROC%i_NPix2", iroc);
    hPulseHeightTrack6[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightTrack6_ROC%i_NPix3Plus", iroc);
    hPulseHeightTrack6[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phMax);
  }
  TH1F* hPulseHeightLong[6][4];
  int const phLongMax = 300000;
  for (int iroc = 0; iroc != 6; ++iroc) {
    TString Name = TString::Format("PulseHeightLong_ROC%i_All", iroc);
    hPulseHeightLong[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phLongMax);
    Name = TString::Format("PulseHeightLong_ROC%i_NPix1", iroc);
    hPulseHeightLong[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phLongMax);
    Name = TString::Format("PulseHeightLong_ROC%i_NPix2", iroc);
    hPulseHeightLong[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phLongMax);
    Name = TString::Format("PulseHeightLong_ROC%i_NPix3Plus", iroc);
    hPulseHeightLong[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phLongMax);
  }

  TH1F* hPulseHeightHot[6];
  for (int iroc = 0; iroc != 6; ++iroc) {
    TString Name = TString::Format("PulseHeightHot_ROC%i", iroc);
    hPulseHeightHot[iroc] = new TH1F(Name, Name, phNBins, phMin, phMax);
  }



  int const HistColors[4] = { 1, 4, 28, 2 };
  for (int iroc = 0; iroc != 6; ++iroc) {
    for (int inpix = 0; inpix != 4; ++inpix) {
    hPulseHeight[iroc][inpix]->SetXTitle("Charge (electrons)");
    hPulseHeight[iroc][inpix]->SetYTitle("Number of Clusters");
    hPulseHeight[iroc][inpix]->SetLineColor(HistColors[inpix]);
    hPulseHeightTrack6[iroc][inpix]->SetXTitle("Charge (electrons)");
    hPulseHeightTrack6[iroc][inpix]->SetYTitle("Number of Clusters");
    hPulseHeightTrack6[iroc][inpix]->SetLineColor(HistColors[inpix]);
    hPulseHeightLong[iroc][inpix]->SetXTitle("Charge (electrons)");
    hPulseHeightLong[iroc][inpix]->SetYTitle("Number of Clusters");
    hPulseHeightLong[iroc][inpix]->SetLineColor(HistColors[inpix]);
    hPulseHeightHot[inpix]->SetXTitle("Charge (electrons)");
    hPulseHeightHot[inpix]->SetYTitle("Number of Hits");
    hPulseHeightHot[inpix]->SetLineColor(HistColors[inpix]);
    }
  }

  // 2D Pulse Height maps for All and Track6
  double AvgPH2D[6][PLTU::NCOL][PLTU::NROW];
  int NAvgPH2D[6][PLTU::NCOL][PLTU::NROW];
  double AvgPH2DTrack6[6][PLTU::NCOL][PLTU::NROW];
  int NAvgPH2DTrack6[6][PLTU::NCOL][PLTU::NROW];
  for (int i = 0; i != 6; ++i) {
    for (int icol = 0; icol != PLTU::NCOL; ++icol) {
      for (int irow = 0; irow != PLTU::NROW; ++irow) {
        AvgPH2D[i][icol][irow] = 0;
        NAvgPH2D[i][icol][irow] = 0;
        AvgPH2DTrack6[i][icol][irow] = 0;
        NAvgPH2DTrack6[i][icol][irow] = 0;
      }
    }
  }

  // Pulse height average counts and averages.  Also define TGraphs
  int NAvgPH[6][4];
  double AvgPH[6][4];
  TGraphErrors gAvgPH[6][4];
  for (int i = 0; i != 6; ++i) {
    for (int j = 0; j != 4; ++j) {
      NAvgPH[i][j] = 0;
      AvgPH[i][j] = 0;
      gAvgPH[i][j].SetName( Form("PulseHeightTime_ROC%i_NPix%i", i, j) );
      gAvgPH[i][j].SetTitle( Form("Average Pulse Height ROC %i NPix %i", i, j) );
      gAvgPH[i][j].GetXaxis()->SetTitle("Event Number");
      gAvgPH[i][j].GetYaxis()->SetTitle("Average Pulse Height (electrons)");
      gAvgPH[i][j].SetLineColor(HistColors[j]);
      gAvgPH[i][j].SetMarkerColor(HistColors[j]);
      gAvgPH[i][j].SetMinimum(0);
      gAvgPH[i][j].SetMaximum(60000);
      gAvgPH[i][j].GetXaxis()->SetTitle("Event Number");
      gAvgPH[i][j].GetYaxis()->SetTitle("Average Pulse Height (electrons)");
    }
  }

  // Prepare Residual histograms
  // hResidual:    x=dX / y=dY
  // hResidualXdY: x=X  / y=dY
  // hResidualYdX: x=Y  / y=dX
  std::vector< TH2F > hResidual;
  std::vector< TH2F > hResidualXdY;
  std::vector< TH2F > hResidualYdX;

  for (int iroc = 0; iroc != 6; ++iroc){
    hResidual.push_back( TH2F(  Form("Residual_ROC%i",iroc),
				Form("Residual_ROC%i",iroc), 100, -.15, .15, 100, -.15, .15));
    hResidualXdY.push_back( TH2F(  Form("ResidualXdY_ROC%i",iroc),
				   Form("ResidualXdY_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
    hResidualYdX.push_back( TH2F(  Form("ResidualYdX_ROC%i",iroc),
				   Form("ResidualYdX_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
  }


  int const TimeWidth = 20000;
  int NGraphPoints = 0;


  // "times" for counting
  int const StartTime = 0;
  int ThisTime;

  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {
    ThisTime = ievent;
    // print progress
    if (ievent % 10000 == 0) {
      std::cout << "Processing event: " << ievent << std::endl;
    }

    //if (BFR.HitPlaneBits() != 0x0) {
      hCoincidenceMap.Fill(BFR.HitPlaneBits());
    //}

    if (ThisTime - (StartTime + NGraphPoints * TimeWidth) > TimeWidth) {
      for (int i = 0; i != 6; ++i) {
        for (int j = 0; j != 4; ++j) {
          gAvgPH[i][j].Set(NGraphPoints+1);
          gAvgPH[i][j].SetPoint(NGraphPoints, ThisTime - TimeWidth/2, AvgPH[i][j]);
          gAvgPH[i][j].SetPointError(NGraphPoints, TimeWidth/2, AvgPH[i][j]/sqrt((float) NAvgPH[i][j]));
          printf("AvgCharge: %i %i N:%9i : %13.3E\n", i, j, NAvgPH[i][j], AvgPH[i][j]);
          NAvgPH[i][j] = 0;
          AvgPH[i][j] = 0;
        }
      }
      ++NGraphPoints;
    }

    // draw tracks
    static int ieventdraw = 0;
    if (ieventdraw < 20 && BFR.NClusters() >= 6) {
      BFR.DrawTracksAndHits( TString::Format(OutDir + "/Tracks_Ev%i.gif", ++ieventdraw).Data() );
    }

    for (size_t iplane = 0; iplane != BFR.NPlanes(); ++iplane) {
      PLTPlane* Plane = BFR.Plane(iplane);

      hNClusters[Plane->ROC()].Fill(Plane->NClusters());

      for (size_t icluster = 0; icluster != Plane->NClusters(); ++icluster) {
        PLTCluster* Cluster = Plane->Cluster(icluster);

        //printf("Event %6i   ROC %i   NHits %3i   Charge %9.0f   Col %3i  Row %3i",
        //    ievent, iplane, Cluster->NHits(), Cluster->Charge(), Cluster->SeedHit()->Column(), Cluster->SeedHit()->Row());
        //for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {
        //  printf(" %5i", Cluster->Hit(ihit)->ADC());
        //}
        //printf("\n");
        if (iplane < 6) {
          hPulseHeight[iplane][0]->Fill(Cluster->Charge());
          hPulseHeightLong[iplane][0]->Fill(Cluster->Charge());
          if (Cluster->Charge() > 300000) {
              //printf("High Charge: %13.3E\n", Cluster->Charge());
              continue;
          }
          PLTU::AddToRunningAverage(AvgPH2D[iplane][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], NAvgPH2D[iplane][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], Cluster->Charge());
          PLTU::AddToRunningAverage(AvgPH[iplane][0], NAvgPH[iplane][0], Cluster->Charge());
          if (Cluster->NHits() == 1) {
            hPulseHeight[iplane][1]->Fill(Cluster->Charge());
            hPulseHeightLong[iplane][1]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][1], NAvgPH[iplane][1], Cluster->Charge());
          } else if (Cluster->NHits() == 2) {
            hPulseHeight[iplane][2]->Fill(Cluster->Charge());
            hPulseHeightLong[iplane][2]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][2], NAvgPH[iplane][2], Cluster->Charge());
          } else if (Cluster->NHits() >= 3) {
            hPulseHeight[iplane][3]->Fill(Cluster->Charge());
            hPulseHeightLong[iplane][3]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][3], NAvgPH[iplane][3], Cluster->Charge());
          }
        }
      }

      for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {
        PLTHit* Hit = Plane->Hit(ihit);


        if (Hit->ROC() < 6) {
          hOccupancy[Hit->ROC()].Fill(Hit->Column(), Hit->Row());
        } else {
          std::cerr << "Oops, ROC >= 6?" << std::endl;
        }
      }

      for (size_t icluster = 0; icluster != Plane->NClusters(); ++icluster) {
        PLTCluster* Cluster = Plane->Cluster(icluster);
        hNHitsPerCluster[Cluster->ROC()].Fill(Cluster->NHits());
        if (Cluster->Charge() > 50000) {
          for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {
            hOccupancyHighPH[Cluster->ROC()].Fill( Cluster->Hit(ihit)->Column(), Cluster->Hit(ihit)->Row() );
          }
        } else if (Cluster->Charge() > 10000 && Cluster->Charge() < 40000) {
          for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {
            hOccupancyLowPH[Cluster->ROC()].Fill( Cluster->Hit(ihit)->Column(), Cluster->Hit(ihit)->Row() );
          }
        }
      }


    }

    if (BFR.NTracks() == 1 &&
        BFR.Track(0)->NClusters() == 6 &&
        BFR.Track(0)->Cluster(0)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(1)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(2)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(3)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(4)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(5)->Charge() < 300000 ) {

        PLTTrack* Track = BFR.Track(0);
        hTrackSlopeX.Fill(Track->fTVX / Track->fTVZ);
        hTrackSlopeY.Fill(Track->fTVY / Track->fTVZ);

        for (size_t icluster = 0; icluster != Track->NClusters(); ++icluster) {
          PLTCluster* Cluster = Track->Cluster(icluster);

          if (Cluster->Charge() > 300000) {
              //printf("High Charge: %13.3E\n", Cluster->Charge());
              continue;
          }
          PLTU::AddToRunningAverage(AvgPH2DTrack6[Cluster->ROC()][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], NAvgPH2DTrack6[Cluster->ROC()][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], Cluster->Charge());

          if (Track->IsFiducial(1, 5, Alignment, PLTPlane::kFiducialRegion_Diamond_m2_m2)) {
            hOccupancyTrack6[Cluster->ROC()].Fill(Cluster->PX(), Cluster->PY());
          }
          hPulseHeightTrack6[Cluster->ROC()][0]->Fill(Cluster->Charge());
          if (Cluster->NHits() == 1) {
            hPulseHeightTrack6[Cluster->ROC()][1]->Fill(Cluster->Charge());
          } else if (Cluster->NHits() == 2) {
            hPulseHeightTrack6[Cluster->ROC()][2]->Fill(Cluster->Charge());
          } else if (Cluster->NHits() >= 3) {
            hPulseHeightTrack6[Cluster->ROC()][3]->Fill(Cluster->Charge());
          }
        }

    }

    // Fill Residual histograms
    for (size_t itrack = 0; itrack < BFR.NTracks(); itrack++){
      // Need at least three hits for the residual to make sense
      if (BFR.Track(itrack)->NClusters() > 2){
          // Loop over clusters
          for (size_t icluster = 0; icluster < BFR.Track(itrack)->NClusters(); icluster++){

          // Get the ROC in which this cluster was recorded and fill the
          // corresponding residual.
          int ROC = BFR.Track(itrack)->Cluster(icluster)->ROC();

          // dX vs dY
          hResidual[ROC].Fill( BFR.Track(itrack)->LResidualX( ROC ),
                               BFR.Track(itrack)->LResidualY( ROC ));
          // X vs dY
          hResidualXdY[ROC].Fill( BFR.Track(itrack)->Cluster(icluster)->LX(),
                                  BFR.Track(itrack)->LResidualY( ROC ));
          // Y vs dX
          hResidualYdX[ROC].Fill( BFR.Track(itrack)->Cluster(icluster)->LY(),
                                  BFR.Track(itrack)->LResidualX( ROC ));

        } // end of loop over clusters
      } // end >2 clusters
    } // end of loop over tracks (filling Residial histograms)

  } // End of Event Loop


  // Catch up on PH by time graph
    for (int i = 0; i != 6; ++i) {
      for (int j = 0; j != 4; ++j) {
        gAvgPH[i][j].Set(NGraphPoints+1);
        gAvgPH[i][j].SetPoint(NGraphPoints, NGraphPoints*TimeWidth + TimeWidth/2, AvgPH[i][j]);
        gAvgPH[i][j].SetPointError(NGraphPoints, TimeWidth/2, AvgPH[i][j]/sqrt((float) NAvgPH[i][j]));
        printf("AvgCharge: %i %i N:%9i : %13.3E\n", i, j, NAvgPH[i][j], AvgPH[i][j]);
        NAvgPH[i][j] = 0;
        AvgPH[i][j] = 0;
      }
    }
    ++NGraphPoints;

  TCanvas Can;
  Can.cd();

  // Hot Pixels
  // Per ROC: [ [row, col], [row,col], ... ]
  std::vector< std::vector< std::vector<int> > > hot_pixels; 
  for (int iroc = 0; iroc != 6; ++iroc) {
    std::vector< std::vector<int> > tmp;
    hot_pixels.push_back( tmp );    
  }
  
  for (int iroc = 0; iroc != 6; ++iroc) {

    // Draw Occupancy histograms
    hOccupancy[iroc].SetMinimum(0);
    hOccupancy[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hOccupancy[iroc].GetName()) + ".gif");
    hOccupancy[iroc].Write();

    TH1F* hOccupancy1DZ = PLTU::HistFrom2D(&hOccupancy[iroc]);
    Can.cd();
    hOccupancy1DZ->Draw("hist");
    if (hOccupancy1DZ->GetEntries() > 0) {
      Can.SetLogy(1);
    }
    Can.SaveAs(OutDir+TString(hOccupancy1DZ->GetName()) + ".gif");
    hOccupancy1DZ->Write();
    Can.SetLogy(0);

    // Grab the quantile you're interested in here
    Double_t QProbability[1] = { 0.95 }; // Quantile positions in [0, 1]
    Double_t QValue[1];                  // Quantile values
    hOccupancy1DZ->GetQuantiles(1, QValue, QProbability);
    if(QValue[0] > 1 && hOccupancy[iroc].GetMaximum() > QValue[0]) {
      hOccupancy[iroc].SetMaximum(QValue[0]);
    }
    Can.cd();
    hOccupancy[iroc].Draw("colz");
    Can.SaveAs( OutDir+Form("Occupancy_ROC%i_Quantile.gif", iroc) );
    delete hOccupancy1DZ;

    Can.cd();
    hOccupancy1DZ = PLTU::HistFrom2D(&hOccupancy[iroc], 0, QValue[0], TString::Format("Occupancy1DZ_ROC%i_Quantile", iroc), 20);
    hOccupancy1DZ->Draw("hist");
    Can.SaveAs(OutDir+TString(hOccupancy1DZ->GetName()) + ".gif");
    delete hOccupancy1DZ;


    // Get 3x3 efficiency hists and draw
    TH2F* h3x3 = PLTU::Get3x3EfficiencyHist(hOccupancy[iroc], 0, 51, 0, 79);
    h3x3->SetTitle( TString::Format("Occupancy Efficiency 3x3 ROC%i", iroc) );
    Can.cd();
    h3x3->SetMinimum(0);
    h3x3->SetMaximum(3);
    h3x3->Draw("colz");
    Can.SaveAs(OutDir+TString(h3x3->GetName()) + ".gif");

    Can.cd();
    TH1F* h3x3_1DZ = PLTU::HistFrom2D(h3x3, "", 50);
    h3x3_1DZ->Draw("hist");
    Can.SaveAs(OutDir+TString(h3x3_1DZ->GetName()) + ".gif");
    delete h3x3;

    Can.cd();
    hNClusters[iroc].SetMinimum(0);
    hNClusters[iroc].SetXTitle("Number of clusters per event");
    hNClusters[iroc].SetYTitle("Events");
    hNClusters[iroc].Draw("hist");
    Can.SaveAs( OutDir+TString(hNClusters[iroc].GetName()) + ".gif");

    // Draw Hits per cluster histograms
    Can.cd();
    hNHitsPerCluster[iroc].SetMinimum(0);
    hNHitsPerCluster[iroc].SetXTitle("Number of hits per cluster");
    hNHitsPerCluster[iroc].SetYTitle("Number of Clusters");
    hNHitsPerCluster[iroc].Draw("hist");
    Can.SaveAs( OutDir+TString(hNHitsPerCluster[iroc].GetName()) + ".gif");

    Can.cd();
    hOccupancyHighPH[iroc].SetMinimum(0);
    hOccupancyHighPH[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hOccupancyHighPH[iroc].GetName()) + ".gif");

    hOccupancyLowPH[iroc].SetMinimum(0);
    hOccupancyLowPH[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hOccupancyLowPH[iroc].GetName()) + ".gif");


    // Draw OccupancyTrack6 histograms
    hOccupancyTrack6[iroc].SetMinimum(0);
    hOccupancyTrack6[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hOccupancyTrack6[iroc].GetName()) + ".gif");



    // Draw the PulseHeights
    gStyle->SetOptStat(0);
    TLegend Leg(0.75, 0.7, 0.90, 0.88, "");
    Leg.SetFillColor(0);
    Leg.SetBorderSize(0);
    Leg.AddEntry(hPulseHeight[iroc][0], "All", "l");
    Leg.AddEntry(hPulseHeight[iroc][1], "1 Pix", "l");
    Leg.AddEntry(hPulseHeight[iroc][2], "2 Pix", "l");
    Leg.AddEntry(hPulseHeight[iroc][3], "3+ Pix", "l");

    hPulseHeight[iroc][0]->SetTitle( TString::Format("Pulse Height ROC%i", iroc) );
    hPulseHeight[iroc][0]->Draw("hist");
    hPulseHeight[iroc][1]->Draw("samehist");
    hPulseHeight[iroc][2]->Draw("samehist");
    hPulseHeight[iroc][3]->Draw("samehist");
    TLegend lPulseHeight(0.75, 0.4, 0.90, 0.7, "Mean:");
    lPulseHeight.SetTextAlign(11);
    lPulseHeight.SetFillStyle(0);
    lPulseHeight.SetBorderSize(0);
    lPulseHeight.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeight[iroc][0]->GetMean()), "")->SetTextColor(HistColors[0]);
    lPulseHeight.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeight[iroc][1]->GetMean()), "")->SetTextColor(HistColors[1]);
    lPulseHeight.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeight[iroc][2]->GetMean()), "")->SetTextColor(HistColors[2]);
    lPulseHeight.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeight[iroc][3]->GetMean()), "")->SetTextColor(HistColors[3]);
    lPulseHeight.Draw("same");
    Leg.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeight_ROC%i.gif", iroc));

    hPulseHeight[iroc][0]->Write();
    hPulseHeight[iroc][1]->Write();
    hPulseHeight[iroc][2]->Write();
    hPulseHeight[iroc][3]->Write();


    Can.cd();
    hPulseHeightTrack6[iroc][0]->SetTitle( TString::Format("Pulse Height Track6 ROC%i", iroc) );
    hPulseHeightTrack6[iroc][0]->Draw("hist");
    hPulseHeightTrack6[iroc][1]->Draw("samehist");
    hPulseHeightTrack6[iroc][2]->Draw("samehist");
    hPulseHeightTrack6[iroc][3]->Draw("samehist");
    TLegend lPulseHeightTrack6(0.75, 0.4, 0.90, 0.7, "Mean:");
    lPulseHeightTrack6.SetTextAlign(11);
    lPulseHeightTrack6.SetFillStyle(0);
    lPulseHeightTrack6.SetBorderSize(0);
    lPulseHeightTrack6.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeightTrack6[iroc][0]->GetMean()), "")->SetTextColor(HistColors[0]);
    lPulseHeightTrack6.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeightTrack6[iroc][1]->GetMean()), "")->SetTextColor(HistColors[1]);
    lPulseHeightTrack6.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeightTrack6[iroc][2]->GetMean()), "")->SetTextColor(HistColors[2]);
    lPulseHeightTrack6.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeightTrack6[iroc][3]->GetMean()), "")->SetTextColor(HistColors[3]);
    lPulseHeightTrack6.Draw("same");
    Leg.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeightTrack6_ROC%i.gif", iroc));

    hPulseHeightTrack6[iroc][0]->Write();
    hPulseHeightTrack6[iroc][1]->Write();
    hPulseHeightTrack6[iroc][2]->Write();
    hPulseHeightTrack6[iroc][3]->Write();
    
    Can.cd();
    hPulseHeightLong[iroc][0]->SetTitle( TString::Format("Pulse Height ROC%i", iroc) );
    hPulseHeightLong[iroc][0]->Draw("hist");
    hPulseHeightLong[iroc][1]->Draw("samehist");
    hPulseHeightLong[iroc][2]->Draw("samehist");
    hPulseHeightLong[iroc][3]->Draw("samehist");
    TLegend lPulseHeightLong(0.75, 0.4, 0.90, 0.7, "Mean:");
    lPulseHeightLong.SetTextAlign(11);
    lPulseHeightLong.SetFillStyle(0);
    lPulseHeightLong.SetBorderSize(0);
    lPulseHeightLong.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeightLong[iroc][0]->GetMean()), "")->SetTextColor(HistColors[0]);
    lPulseHeightLong.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeightLong[iroc][1]->GetMean()), "")->SetTextColor(HistColors[1]);
    lPulseHeightLong.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeightLong[iroc][2]->GetMean()), "")->SetTextColor(HistColors[2]);
    lPulseHeightLong.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeightLong[iroc][3]->GetMean()), "")->SetTextColor(HistColors[3]);
    lPulseHeightLong.Draw("same");
    Leg.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeightLong_ROC%i.gif", iroc));

    Can.cd();
    gAvgPH[iroc][0].SetTitle( TString::Format("Average Pulse Height ROC%i", iroc) );
    gAvgPH[iroc][0].Draw("Ape");
    gAvgPH[iroc][1].Draw("samepe");
    gAvgPH[iroc][2].Draw("samepe");
    gAvgPH[iroc][3].Draw("samepe");
    Leg.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeightTime_ROC%i.gif", iroc));


    // Use AvgPH2D to draw PH 2D maps
    TString Name = TString::Format("PulseHeightAvg2D_ROC%i", iroc);
    TH2F hPulseHeightAvg2D(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
    for (int icol = 0; icol != PLTU::NCOL; ++icol) {
      for (int irow = 0; irow != PLTU::NROW; ++irow) {
        hPulseHeightAvg2D.SetBinContent(icol+1, irow+1, AvgPH2D[iroc][icol][irow]);
      }
    }
    Can.cd();
    hPulseHeightAvg2D.SetMinimum(0);
    hPulseHeightAvg2D.SetMaximum(100000);
    hPulseHeightAvg2D.Draw("colz");
    Can.SaveAs(OutDir+hPulseHeightAvg2D.GetName() + ".gif");

    // Use AvgPH2D Track6 to draw PH 2D maps
    Name = TString::Format("PulseHeightAvg2DTrack6_ROC%i", iroc);
    TH2F hPulseHeightAvg2DTrack6(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
    for (int icol = 0; icol != PLTU::NCOL; ++icol) {
      for (int irow = 0; irow != PLTU::NROW; ++irow) {
        hPulseHeightAvg2DTrack6.SetBinContent(icol+1, irow+1, AvgPH2DTrack6[iroc][icol][irow]);
      }
    }
    Can.cd();
    hPulseHeightAvg2DTrack6.SetMinimum(0);
    hPulseHeightAvg2DTrack6.SetMaximum(100000);
    hPulseHeightAvg2DTrack6.Draw("colz");
    Can.SaveAs(OutDir+hPulseHeightAvg2DTrack6.GetName() + ".gif");


    // 2D Residuals
    Can.cd();
    hResidual[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + ".gif");

    // 2D Residuals X/dY
    gStyle->SetOptStat(1111);
    hResidualXdY[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hResidualXdY[iroc].GetName()) + ".gif");

    // 2D Residuals Y/dX
    gStyle->SetOptStat(1111);
    hResidualYdX[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hResidualYdX[iroc].GetName()) + ".gif");

    // Residual X-Projection
    Can.cd();
    hResidual[iroc].ProjectionX()->Draw();
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + "_X.gif");

    // Residual Y-Projection
    Can.cd();
    hResidual[iroc].ProjectionY()->Draw();
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + "_Y.gif");

    gStyle->SetOptStat(0);

    // Looking for Hot Pixels
    //asdasd<--suck it dean
    // Calculate mean occupancy of nonzero pixels
    double sum = 0;
    int n_nonzero_pixels = 0;
    for (int icol=1; icol != hOccupancy[iroc].GetNbinsX()+1; icol++){ 	
      for (int irow=1; irow != hOccupancy[iroc].GetNbinsY()+1; irow++){
 	
	if (hOccupancy[iroc].GetBinContent(icol, irow) > 0){
	  sum += hOccupancy[iroc].GetBinContent( icol, irow);
	  n_nonzero_pixels++;
	}	
      }
    }    
    float mean_occupancy;
    if (n_nonzero_pixels>0)
      mean_occupancy = sum/n_nonzero_pixels;
    else
      mean_occupancy = -1;

    
    // Look for pixels with an occupancy of more than 10 times the mean
    for (int icol=1; icol != hOccupancy[iroc].GetNbinsX()+1; icol++){ 	
      for (int irow=1; irow != hOccupancy[iroc].GetNbinsY()+1; irow++){

	if (hOccupancy[iroc].GetBinContent(icol, irow) > 10*mean_occupancy){
	  std::vector<int> colrow;
	  colrow.push_back( icol );
	  colrow.push_back( irow );
	  hot_pixels[iroc].push_back( colrow );
	}
      }
    }

  } // end of loop over ROCs

  TCanvas Can2("CoincidenceMap", "CoincidenceMap", 1200, 400);
  Can2.cd();
  hCoincidenceMap.Draw("");
  Can2.SaveAs(OutDir+"Occupancy_Coincidence.gif");

  Can.cd();
  hTrackSlopeX.Draw("hist");
  Can.SaveAs(OutDir+"TrackSlopeX.gif");

  Can.cd();
  hTrackSlopeY.Draw("hist");
  Can.SaveAs(OutDir+"TrackSlopeY.gif");

  // Count the total number of hot pixels
  int total_hotpixels = 0;
  for (int iroc = 0; iroc != 6; ++iroc){
    total_hotpixels += hot_pixels[iroc].size();
  }

  // Now do a second look and measure the charges of hot pixel
  if (total_hotpixels >0){

    // Re- Initialize Reader
    PSIBinaryFileReader  BFR2(InFileName, CalibrationList);
    BFR2.SetTrackingAlignment(&Alignment);
    f = fopen("MyGainCal.dat", "w");
    BFR2.GetGainCal()->PrintGainCal(f);
    fclose(f);
    BFR2.CalculateLevels(10000, OutDir);
  
    // Event Loop  
    for (int ievent = 0; BFR2.GetNextEvent() >= 0; ++ievent) {
  
      if (ievent % 10000 == 0)
        std::cout << "Processing event: " << ievent << std::endl;
      
      for (size_t iplane = 0; iplane != BFR2.NPlanes(); ++iplane) {
        PLTPlane* Plane = BFR2.Plane(iplane);
              
        for ( std::vector< std::vector< int > >::iterator hot = hot_pixels[iplane].begin(); 
  	    hot != hot_pixels[iplane].end(); 
  	    ++hot){			
  	for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {
  	  PLTHit* Hit = Plane->Hit(ihit);
  	  if ( (Hit->Column()== (*hot)[0]) and (Hit->Row()==(*hot)[1]) ){
  	    hPulseHeightHot[iplane]->Fill( Hit->Charge() );
	    hOccupancyHot[iplane].Fill( Hit->Column(), Hit->Row(),1);
  	  }
  	}
        }
      }
    } 
    
    // Produce the HOT histograms
    for (int iroc = 0; iroc != 6; ++iroc){
      Can.cd();
      hPulseHeightHot[iroc]->SetTitle( TString::Format("Hot Pixel Pulse Height ROC%i (N_{Hot}=%i)", 
						       iroc,
						       hot_pixels[iroc].size()) );
      hPulseHeightHot[iroc]->Draw("hist");
      Can.SaveAs(OutDir+TString::Format("PulseHeightHot_ROC%i.gif", iroc));

      // Draw OccupancyHot histograms
      hOccupancyHot[iroc].SetMinimum(0);
      hOccupancyHot[iroc].Draw("colz");
      Can.SaveAs( OutDir+TString(hOccupancyHot[iroc].GetName()) + ".gif");

    }

  } // end of hotpixels>0



  WriteHTML(PlotsDir + RunNumber, CalibrationList);

  return 0;
}

int TestPSIBinaryFileReaderAlign (std::string const InFileName, std::string const CalibrationList, TString const RunNumber)
{
  /* TestPSIBinaryFileReaderAlign: Produce alignment constants and save
  them to NewAlignment.dat
  */

	TString const PlotsDir = "plots/";
	TString const OutDir = PlotsDir + RunNumber;
  // Repeat up to 100 times. Cancel if the squared sum of residuals
  // improves by less than 0.01%
  int NMaxAlignmentIterations = 100;

  gStyle->SetOptStat(0);

  // Start with initial Alignment (X,Y offsets and rotations set to zero)
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope_initial.dat");

  // Prepare Residual histograms
  // hResidual:    x=dX / y=dY
  // hResidualXdY: x=X  / y=dY
  // hResidualYdX: x=Y  / y=dX
  std::vector< TH2F > hResidual;
  std::vector< TH2F > hResidualXdY;
  std::vector< TH2F > hResidualYdX;

  // Keep track of the squarted sum of residuals and use it as exit
  // criterion
  double sumResSquareCurrent = 0.;
  double sumResSquareLast    = -1;

  // Alignment loop
  for (int ialign = 0; ialign < NMaxAlignmentIterations; ialign++){

    std::cout << "At iteration " << ialign << std::endl;

    PSIBinaryFileReader BFR(InFileName, CalibrationList);
    BFR.SetTrackingAlignment(&Alignment);
    FILE* f = fopen("MyGainCal.dat", "w");
    BFR.GetGainCal()->PrintGainCal(f);
    fclose(f);
    BFR.CalculateLevels(10000 ,OutDir);

    // Reset residual histograms
    hResidual.clear();
    hResidualXdY.clear();
    hResidualYdX.clear();
    for (int iroc = 0; iroc != 6; ++iroc){
      hResidual.push_back( TH2F(  Form("Residual_ROC%i",iroc),
                                  Form("Residual_ROC%i",iroc), 100, -.2, .2, 100, -.2, .2));
      hResidualXdY.push_back( TH2F(  Form("ResidualXdY_ROC%i",iroc),
                                     Form("ResidualXdY_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
      hResidualYdX.push_back( TH2F(  Form("ResidualYdX_ROC%i",iroc),
                                     Form("ResidualYdX_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
    }

    sumResSquareCurrent = 0;

    // Event Loop
    for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

      if (ievent % 10000 == 0)
        std::cout << "Processing event: " << ievent << std::endl;

      // Fill Residual histograms
      for (size_t itrack = 0; itrack < BFR.NTracks(); itrack++){
        // Need at least three hits for the residual to make sense
        if (BFR.Track(itrack)->NClusters() > 2){
            // Loop over clusters
            for (size_t icluster = 0; icluster < BFR.Track(itrack)->NClusters(); icluster++){

            // Get the ROC in which this cluster was recorded and fill the
            // corresponding residual.
            int ROC = BFR.Track(itrack)->Cluster(icluster)->ROC();

            // dX vs dY
            hResidual[ROC].Fill( BFR.Track(itrack)->LResidualX( ROC ),
                                 BFR.Track(itrack)->LResidualY( ROC ));
            // X vs dY
            hResidualXdY[ROC].Fill( BFR.Track(itrack)->Cluster(icluster)->LX(),
                                    BFR.Track(itrack)->LResidualY( ROC ));
            // Y vs dX
            hResidualYdX[ROC].Fill( BFR.Track(itrack)->Cluster(icluster)->LY(),
                                    BFR.Track(itrack)->LResidualX( ROC ));

            // Also measure the squared sum of residuals
            // check against self so we don't get NaNs
            if (BFR.Track(itrack)->Cluster(icluster)->LX()==  BFR.Track(itrack)->Cluster(icluster)->LX())
              sumResSquareCurrent +=  BFR.Track(itrack)->Cluster(icluster)->LX()* BFR.Track(itrack)->Cluster(icluster)->LX();
            if (BFR.Track(itrack)->Cluster(icluster)->LY()==  BFR.Track(itrack)->Cluster(icluster)->LY())
              sumResSquareCurrent +=  BFR.Track(itrack)->Cluster(icluster)->LY()* BFR.Track(itrack)->Cluster(icluster)->LY();

          } // end of loop over clusters
        } // end >2 clusters
      } // end of loop over tracks
    } // end event loop

    // First iteration, init sumResSquareLast
    if (ialign == 0){
      sumResSquareLast = sumResSquareCurrent;
    }
    else{
      // Improvement wrt/ last iteration of less than 0.01%. Quit.
      if (fabs(sumResSquareLast-sumResSquareCurrent)/sumResSquareLast < 0.0001 ){
        break;
      }
      // Otherwise: update last residual and try again
      else{
        sumResSquareLast = sumResSquareCurrent;
      }
    }

    // Loop over ROCs to update alignment
    for (int iroc = 0; iroc != 6; ++iroc) {
      Alignment.AddToLX( 1, iroc, hResidual[iroc].GetMean(1));
      Alignment.AddToLY( 1, iroc, hResidual[iroc].GetMean(2));
      float angle = atan(hResidualXdY[iroc].GetCorrelationFactor()) ;
      Alignment.AddToLR( 1, iroc, angle/10. );
    }
  } // end alignment loop

  // Loop over ROCs to draw final per-plane histos
  for (int iroc = 0; iroc != 6; ++iroc) {
    TCanvas Can;
    Can.cd();

    // 2D Residuals
    hResidual[iroc].Draw("colz");
    Can.SaveAs( OutDir+"/"+TString(hResidual[iroc].GetName()) + ".gif");

    // Residual X-Projection
    gStyle->SetOptStat(1111);
    hResidual[iroc].ProjectionX()->Draw();
    Can.SaveAs( OutDir+"/"+TString(hResidual[iroc].GetName()) + "_X.gif");

    // Residual Y-Projection
    hResidual[iroc].ProjectionY()->Draw();
    Can.SaveAs( OutDir+"/"+TString(hResidual[iroc].GetName()) + "_Y.gif");

    // 2D Residuals X/dY
    hResidualXdY[iroc].Draw("colz");
    Can.SaveAs( OutDir+"/"+TString(hResidualXdY[iroc].GetName()) + ".gif");

    // 2D Residuals Y/dX
    hResidualYdX[iroc].Draw("colz");
    Can.SaveAs( OutDir+"/"+TString(hResidualYdX[iroc].GetName()) + ".gif");

  } // end loop over ROCs
  Alignment.WriteAlignmentFile("NewAlignment.dat");

  return 0;
}







void WriteHTML (TString const OutDir, TString const CalFile)
{
  // This function to write the HTML output for a run

  // Make output dir
  if (gSystem->mkdir(OutDir, true) != 0) {
    std::cerr << "WARNING: either OutDir exists or it is un-mkdir-able: " << OutDir << std::endl;
  }

  TString FileName;
  if (OutDir.Length() == 0) {
    FileName = OutDir + "/index.html";
  } else {
    FileName = OutDir + "/index.html";
  }
  std::ofstream f(FileName.Data());
  if (!f.is_open()) {
    std::cerr << "ERROR: Cannot open HTML file: " << FileName << std::endl;
    return;
  }



  f << "<html><body>\n";

  // RUN SUMMARY
  f << "<h1>Run Summary: </h1>\n";
  std::ifstream fCL(CalFile.Data());
  if (!fCL.is_open()) {
      std::cerr << "ERROR: cannot open calibratin list: " << CalFile << std::endl;
      throw;
  }
  std::string line;
  while (!fCL.eof()) {
      std::getline(fCL, line);
    f << line << "<br>\n";
  }
  fCL.close();

  // LEVELS
  f << "<hr />\n";
  f << "<h2>Levels</h2>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"Levels_ROC%i.gif\"><img width=\"150\" src=\"Levels_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;

  // OCCUPANCY
  f << "<hr />\n";
  f << "<h2>Occupancy</h2>" << std::endl;
  f << "<a href=\"Occupancy_Coincidence.gif\"><img width=\"900\" src=\"Occupancy_Coincidence.gif\"></a>\n<br>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i.gif\"><img width=\"150\" src=\"Occupancy_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i_1DZ.gif\"><img width=\"150\" src=\"Occupancy_ROC%i_1DZ.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i_Quantile.gif\"><img width=\"150\" src=\"Occupancy_ROC%i_Quantile.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"Occupancy1DZ_ROC%i_Quantile.gif\"><img width=\"150\" src=\"Occupancy1DZ_ROC%i_Quantile.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;

  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i_3x3Efficiency.gif\"><img width=\"150\" src=\"Occupancy_ROC%i_3x3Efficiency.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i_3x3Efficiency_1DZ.gif\"><img width=\"150\" src=\"Occupancy_ROC%i_3x3Efficiency_1DZ.gif\"></a>\n", i, i);
  }

  f << "<br>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"NClusters_ROC%i.gif\"><img width=\"150\" src=\"NClusters_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  f << "<br>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"NHitsPerCluster_ROC%i.gif\"><img width=\"150\" src=\"NHitsPerCluster_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;

  // PULSE HEIGHT
  f << "<hr />\n";
  f << "<h2>Pulse Height</h2>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeight_ROC%i.gif\"><img width=\"150\" src=\"PulseHeight_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeightLong_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightLong_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeightTime_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightTime_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeightAvg2D_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightAvg2D_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"OccupancyLowPH_ROC%i.gif\"><img width=\"150\" src=\"OccupancyLowPH_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"OccupancyHighPH_ROC%i.gif\"><img width=\"150\" src=\"OccupancyHighPH_ROC%i.gif\"></a>\n", i, i);
  }

  // TRACKING
  f << "<h2>Tracking</h2>\n";
  f << "<a href=\"TrackSlopeX.gif\"><img width=\"150\" src=\"TrackSlopeX.gif\"></a>\n";
  f << "<a href=\"TrackSlopeY.gif\"><img width=\"150\" src=\"TrackSlopeY.gif\"></a>\n";

  f << "<br>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"OccupancyTrack6_ROC%i.gif\"><img width=\"150\" src=\"OccupancyTrack6_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeightTrack6_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightTrack6_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeightAvg2DTrack6_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightAvg2DTrack6_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";

  // TRACK RESIDUALS
  f << "<h2>Track Residuals</h2>\n";

  f << "<br>" << std::endl;
  for (int i = 0; i != 6; i++)
    f << Form("<a href=\"Residual_ROC%i_X.gif\"><img width=\"150\" src=\"Residual_ROC%i_X.gif\"></a>\n", i, i);    
  f << "<br>\n";

  for (int i = 0; i != 6; i++)
    f << Form("<a href=\"Residual_ROC%i_Y.gif\"><img width=\"150\" src=\"Residual_ROC%i_Y.gif\"></a>\n", i, i);    
  f << "<br>\n";

  // EVENT DISPLAYS
  f << "<h2>Event Displays</h2>\n";

  f << "<br>" << std::endl;
  for (int irow = 0; irow != 4; irow++){
    for (int icol = 1; icol != 6; ++icol) {
      int i = irow*5+icol;
      f << Form("<a href=\"Tracks_Ev%i.gif\"><img width=\"150\" src=\"Tracks_Ev%i.gif\"></a>\n", i, i);
    }
    f << "<br>\n";
  }
  f << "<br>\n";

  // HOT PIXELS
  f << "<h2>Hot Pixels</h2>\n";

  f << "<br>" << std::endl;
  for (int i = 0; i != 6; i++)
    f << Form("<a href=\"PulseHeightHot_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightHot_ROC%i.gif\"></a>\n", i, i);    
  f << "<br>\n";


  for (int i = 0; i != 6; i++)
    f << Form("<a href=\"OccupancyHot_ROC%i.gif\"><img width=\"150\" src=\"OccupancyHot_ROC%i.gif\"></a>\n", i, i);    
  f << "<br>\n";

  f << "</body></html>";
  f.close();


  return;
}







int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [InFileName] [CalibrationList.txt] [doAlign]" << std::endl;
    std::cerr << "doAlign: 0 for reading alignment from file, 1 for producing alignment file" << std::endl;
    return 1;
  }

  /* There are now two useage modes: default and alignment

  default uses the Alignment_ETHTelescope.dat file and analyzes the given run
    producing Occupancy, PulseHeight and tracking plots.

  alignment starts with all alignment constants zero and does several iterations
    to minimize the residuals. All planes are shifted in x and y and rotated
    around the z-axis. Residual plots of the last iteration are saved.
    As output the file NewAlignment.dat is produced. To actually use it, do:
    mv NewAlignment.dat ALIGNMENT/Alignment_ETHTelescope
  */

  std::string const InFileName = argv[1];
  TString const FullRunName = InFileName;
  Int_t const Index = FullRunName.Index("bt05r",0);
  TString const RunNumber = FullRunName(Index+5,6);
  gSystem->mkdir("./plots/" + RunNumber);

  std::string CalibrationList = argv[2];


  int doAlign = atoi(argv[3]);

  if (doAlign)
    TestPSIBinaryFileReaderAlign(InFileName, CalibrationList, RunNumber);
  else
    TestPSIBinaryFileReader(InFileName, CalibrationList, RunNumber);


  return 0;
}
