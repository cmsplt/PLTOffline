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
#include "TString.h"
#include "TSystem.h"
#include "TGraphErrors.h"



void WriteHTML (TString const);

int TestPSIBinaryFileReader (std::string const InFileName, TString const RunNumber)
{

	TString const PlotsDir = "plots/";
	TString const OutDir = PlotsDir + RunNumber + "/";

	std::cout<<OutDir<<std::endl;
  /* TestPSIBinaryFileReaderAlign: Default run analysis.
  */

  gStyle->SetOptStat(0);

  // Open Alignment
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope.dat");

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName, "/Users/dhidas/PSITelescope_Cosmics/Telescope_test/phCalibrationFitTan_C5.dat");
  BFR.SetTrackingAlignment(&Alignment);
  FILE* f = fopen("MyGainCal.dat", "w");
  BFR.GetGainCal()->PrintGainCal(f);
  fclose(f);
  BFR.CalculateLevels(10000, OutDir);

  // Prepare Occupancy histograms
  std::vector< TH2F > hOccupancy;
  for (int iroc = 0; iroc != 6; ++iroc){
    hOccupancy.push_back( TH2F( Form("Occupancy_ROC%i",iroc),
                                Form("Occupancy_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
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

  int const HistColors[4] = { 1, 4, 28, 2 };
  for (int iroc = 0; iroc != 6; ++iroc) {
    for (int inpix = 0; inpix != 4; ++inpix) {
    hPulseHeight[iroc][inpix]->SetXTitle("Charge (electrons)");
    hPulseHeight[iroc][inpix]->SetYTitle("Number of Clusters");
    hPulseHeight[iroc][inpix]->SetLineColor(HistColors[inpix]);
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

  int const TimeWidth = 50000;
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
          NAvgPH[i][j] = 0;
          AvgPH[i][j] = 0;
        }
      }
      ++NGraphPoints;
    }

    // draw tracks
    static int ieventdraw = 0;
    if (ieventdraw < 20 && BFR.NClusters() >= 2) {
      BFR.DrawTracksAndHits( TString::Format(OutDir + "/Tracks_Ev%i.gif", ++ieventdraw).Data() );
    }

    for (size_t iplane = 0; iplane != BFR.NPlanes(); ++iplane) {
      PLTPlane* Plane = BFR.Plane(iplane);

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
          PLTU::AddToRunningAverage(AvgPH[iplane][0], NAvgPH[iplane][0], Cluster->Charge());
          if (Cluster->NHits() == 1) {
            hPulseHeight[iplane][1]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][1], NAvgPH[iplane][1], Cluster->Charge());
          } else if (Cluster->NHits() == 2) {
            hPulseHeight[iplane][2]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][2], NAvgPH[iplane][2], Cluster->Charge());
          } else if (Cluster->NHits() >= 3) {
            hPulseHeight[iplane][3]->Fill(Cluster->Charge());
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


    }


  } // End of Event Loop

  TCanvas Can;
  Can.cd();

  for (int iroc = 0; iroc != 6; ++iroc) {

    // Draw Occupancy histograms
    hOccupancy[iroc].SetMinimum(0);
    hOccupancy[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hOccupancy[iroc].GetName()) + ".gif");

    TH1F* hOccupancy1DZ = PLTU::HistFrom2D(&hOccupancy[iroc]);
    Can.cd();
    hOccupancy1DZ->Draw("hist");
    if (hOccupancy1DZ->GetEntries() > 0) {
      Can.SetLogy(1);
    }
    Can.SaveAs(OutDir+TString(hOccupancy1DZ->GetName()) + ".gif");
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

    // Draw the PulseHeights
    gStyle->SetOptStat(0);
    TLegend* Leg = new TLegend(0.75, 0.7, 0.90, 0.88, "");
    Leg->SetFillColor(0);
    Leg->SetBorderSize(0);
    Leg->AddEntry(hPulseHeight[iroc][0], "All", "l");
    Leg->AddEntry(hPulseHeight[iroc][1], "1 Pix", "l");
    Leg->AddEntry(hPulseHeight[iroc][2], "2 Pix", "l");
    Leg->AddEntry(hPulseHeight[iroc][3], "3+ Pix", "l");

    hPulseHeight[iroc][0]->SetTitle( TString::Format("Pulse Height ROC%i", iroc) );
    hPulseHeight[iroc][0]->Draw("hist");
    hPulseHeight[iroc][1]->Draw("samehist");
    hPulseHeight[iroc][2]->Draw("samehist");
    hPulseHeight[iroc][3]->Draw("samehist");
    Leg->Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeight_ROC%i.gif", iroc));

    gAvgPH[iroc][0].SetTitle( TString::Format("Average Pulse Height ROC%i", iroc) );
    gAvgPH[iroc][0].Draw("Ape");
    gAvgPH[iroc][1].Draw("samepe");
    gAvgPH[iroc][2].Draw("samepe");
    gAvgPH[iroc][3].Draw("samepe");
    Leg->Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeightTime_ROC%i.gif", iroc));

  }

  TCanvas Can2("CoincidenceMap", "CoincidenceMap", 1200, 400);
  Can2.cd();
  hCoincidenceMap.Draw("");
  Can2.SaveAs(OutDir+"Occupancy_Coincidence.gif");


  WriteHTML(PlotsDir + RunNumber);

  return 0;
}

int TestPSIBinaryFileReaderAlign (std::string const InFileName,TString const RunNumber)
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

    PSIBinaryFileReader BFR(InFileName, "/Users/dhidas/PSITelescope_Cosmics/Telescope_test/phCalibrationFitTan_C5.dat");
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
                                  Form("Residual_ROC%i",iroc), 100, -.5, .5, 100, -.5, .5));
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
    // Dont move first and last plane
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
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + ".gif");

    // Residual X-Projection
    gStyle->SetOptStat(1111);
    hResidual[iroc].ProjectionX()->Draw();
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + "_X.gif");

    // Residual Y-Projection
    hResidual[iroc].ProjectionY()->Draw();
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + "_Y.gif");

    // 2D Residuals X/dY
    hResidualXdY[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hResidualXdY[iroc].GetName()) + ".gif");

    // 2D Residuals Y/dX
    hResidualYdX[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hResidualYdX[iroc].GetName()) + ".gif");

  } // end loop over ROCs
  Alignment.WriteAlignmentFile("NewAlignment.dat");

  return 0;
}







void WriteHTML (TString const OutDir)
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
  f << "<h1>Run Summary: </h1>\n";
  //f << "DataFileName: " << DataFileName << "<br />\n";
  //f << "GainCalFileName: " << GainCalFileName << "<br />\n";
  //f << "AlignmentFileName: " << AlignmentFileName << "<br />\n";
  //f << "Number of events: " << ie << "<br />\n";
  //f << "<br />\n<a href=\"" << OutFileName << "\">" << OutFileName << "</a><br />\n";

  f << "<hr />\n";
  f << "<h2>Levels</h2>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"Levels_ROC%i.gif\"><img width=\"150\" src=\"Levels_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;

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


  f << "<hr />\n";
  f << "<h2>Pulse Height</h2>" << std::endl;
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeight_ROC%i.gif\"><img width=\"150\" src=\"PulseHeight_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeightTime_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightTime_ROC%i.gif\"></a>\n", i, i);
  }


  f << "</body></html>";
  f.close();
  return;
}







int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFileName] [doAlign]" << std::endl;
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


  int doAlign = atoi(argv[2]);

  if (doAlign)
    TestPSIBinaryFileReaderAlign(InFileName,RunNumber);
  else
    TestPSIBinaryFileReader(InFileName,RunNumber);


  return 0;
}
