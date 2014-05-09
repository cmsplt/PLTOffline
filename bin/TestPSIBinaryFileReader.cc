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

  TH2F* hOccupancy[6];
  hOccupancy[0] = new TH2F("Occupancy_ROC0", "Occupancy_ROC0", 52, 0, 52, 80, 0, 80);
  hOccupancy[1] = new TH2F("Occupancy_ROC1", "Occupancy_ROC1", 52, 0, 52, 80, 0, 80);
  hOccupancy[2] = new TH2F("Occupancy_ROC2", "Occupancy_ROC2", 52, 0, 52, 80, 0, 80);
  hOccupancy[3] = new TH2F("Occupancy_ROC3", "Occupancy_ROC3", 52, 0, 52, 80, 0, 80);
  hOccupancy[4] = new TH2F("Occupancy_ROC4", "Occupancy_ROC4", 52, 0, 52, 80, 0, 80);
  hOccupancy[5] = new TH2F("Occupancy_ROC5", "Occupancy_ROC5", 52, 0, 52, 80, 0, 80);

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


  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {
    if (ievent % 10000 == 0) {
      std::cout << "Processing event: " << ievent << std::endl;
    }

    static int ieventdraw = 0;
    if (ieventdraw < 20 && BFR.NClusters() >= 2) { 
      BFR.DrawTracksAndHits( TString::Format("plots/Tracks_Ev%i.gif", ++ieventdraw).Data() );
    }




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
          hOccupancy[Hit->ROC()]->Fill(Hit->Column(), Hit->Row());
        } else {
          std::cerr << "Oops, ROC >= 6?" << std::endl;
        }
      }


    }
    if (BFR.NHits() > 0) {
      std::cout << std::endl;
    }


  }

  TCanvas Can;
  Can.cd();
  for (int iroc = 0; iroc != 6; ++iroc) {
    hOccupancy[iroc]->Draw("colz");
    Can.SaveAs( TString(hOccupancy[iroc]->GetName()) + ".gif");
    delete hOccupancy[iroc];

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
    //delete hPulseHeight[iroc];
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
