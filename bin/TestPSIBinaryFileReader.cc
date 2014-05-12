////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Apr  9 13:49:09 CEST 2014
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PSIBinaryFileReader.h"
#include "PLTPlane.h"
#include "PLTAlignment.h"

#include "TLegend.h"
#include "TString.h"

int TestPSIBinaryFileReader (std::string const InFileName)
{
  //PLTU::SetStyle();
  gStyle->SetOptStat(0);

  PSIBinaryFileReader BFR(InFileName, "/Users/dhidas/PSITelescope_Cosmics/Telescope_test/phCalibrationFitTan_C5.dat");
  FILE* f = fopen("MyGainCal.dat", "w");
  BFR.GetGainCal()->PrintGainCal(f);
  fclose(f);
  BFR.CalculateLevels();
  //BFR.ReadAddressesFromFile("/Users/dhidas/PSITelescope_Cosmics/Telescope_test/addressParameters.dat");

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope.dat");


  int const HistColors[4] = { 1, 4, 28, 2 };

  // Prepare Occupancy and Residual histograms
  std::vector< TH2F > hResidual;
  std::vector< TH2F > hOccupancy;
  for (int iroc = 0; iroc != 6; ++iroc){
    hOccupancy.push_back( TH2F( Form("Occupancy_ROC%i",iroc),
                                Form("Occupancy_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
    hResidual.push_back( TH2F(  Form("Residual_ROC%i",iroc),
                                Form("Residual_ROC%i",iroc), 100, -.5, .5, 100, -.5, .5));
  }


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

  for (int iroc = 0; iroc != 6; ++iroc) {
    for (int inpix = 0; inpix != 4; ++inpix) {
    hPulseHeight[iroc][inpix]->SetXTitle("Charge (electrons)");
    hPulseHeight[iroc][inpix]->SetYTitle("Number of Clusters");
    hPulseHeight[iroc][inpix]->SetLineColor(HistColors[inpix]);
    }
  }

  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {
    if (ievent % 10000 == 0) {
      std::cout << "Processing event: " << ievent << std::endl;
    }

    static int ieventdraw = 0;
    if (ieventdraw < 20 && BFR.NClusters() >= 2) {
      BFR.DrawTracksAndHits( TString::Format("plots/Tracks_Ev%i.gif", ++ieventdraw).Data() );
    }

    // Fill Residual histograms
    for (int itrack = 0; itrack < BFR.NTracks(); itrack++){
      // Need at least three hits for the residual to make sense
      if (BFR.Track(itrack)->NClusters() > 2){
          // Loop over clusters
          for (int icluster = 0; icluster < BFR.Track(itrack)->NClusters(); icluster++){
          // Get the ROC in which this cluster was recorded and fill the
          // corresponding residual.
          int ROC = BFR.Track(itrack)->Cluster(icluster)->ROC();
          hResidual[ROC].Fill( BFR.Track(itrack)->LResidualX( ROC ),
                               BFR.Track(itrack)->LResidualY( ROC ));

        } // end of loop over clusters
      } // end >2 clusters
    } // end of loop over tracks


    for (size_t iplane = 0; iplane != BFR.NPlanes(); ++iplane) {
      PLTPlane* Plane = BFR.Plane(iplane);

      for (size_t icluster = 0; icluster != Plane->NClusters(); ++icluster) {
        PLTCluster* Cluster = Plane->Cluster(icluster);

        printf("Event %6i   ROC %i   NHits %3i   Charge %9.0f   Col %3i  Row %3i",
            ievent, iplane, Cluster->NHits(), Cluster->Charge(), Cluster->SeedHit()->Column(), Cluster->SeedHit()->Row());
        for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {
          printf(" %5i", Cluster->Hit(ihit)->ADC());
        }
        printf("\n");
        if (iplane < 6) {
          hPulseHeight[iplane][0]->Fill(Cluster->Charge());
          if (Cluster->NHits() == 1) {
            hPulseHeight[iplane][1]->Fill(Cluster->Charge());
          } else if (Cluster->NHits() == 2) {
            hPulseHeight[iplane][2]->Fill(Cluster->Charge());
          } else if (Cluster->NHits() >= 3) {
            hPulseHeight[iplane][3]->Fill(Cluster->Charge());
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
    if (BFR.NHits() > 0) {
      std::cout << std::endl;
    }


  } // End of Event Loop

  TCanvas Can;
  Can.cd();

  for (int iroc = 0; iroc != 6; ++iroc) {

    // Draw Occupancy histograms
    hOccupancy[iroc].Draw("colz");
    Can.SaveAs( TString(hOccupancy[iroc].GetName()) + ".gif");

    // Draw 2D Residuals
    hResidual[iroc].Draw("colz");
    Can.SaveAs( TString(hResidual[iroc].GetName()) + ".gif");

    // Draw Residual X-Projection
    gStyle->SetOptStat(1111);
    hResidual[iroc].ProjectionX()->Draw();
    Can.SaveAs( TString(hResidual[iroc].GetName()) + "_X.gif");

    // Draw Residual Y-Projection
    hResidual[iroc].ProjectionY()->Draw();
    Can.SaveAs( TString(hResidual[iroc].GetName()) + "_Y.gif");


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
    Can.SaveAs(TString::Format("PulseHeight_ROC%i.gif", iroc));
  }


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  std::string const InFileName = argv[1];

  TestPSIBinaryFileReader(InFileName);

  return 0;
}
