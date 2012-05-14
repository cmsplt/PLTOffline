////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue May 24 09:54:58 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>
#include <numeric>

#include "PLTEvent.h"
#include "PLTU.h"


#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE

TH1F* FidHistFrom2D (TH2F* hIN, TString const NewName, int const NBins, PLTPlane::FiducialRegion FidRegion)
{
  // This function returns a TH1F* and YOU are then the owner of
  // that memory.  please delete it when you are done!!!

  int const NBinsX = hIN->GetNbinsX();
  int const NBinsY = hIN->GetNbinsY();
  float const ZMin = hIN->GetMinimum();
  float const ZMax = hIN->GetMaximum() * (1.0 + 1.0 / (float) NBins);
  int const MyNBins = NBins + 1;

  TString const hNAME = NewName == "" ? TString(hIN->GetName()) + "_1DZFid" : NewName;

  TH1F* h;
  h = new TH1F(hNAME, hNAME, MyNBins, ZMin, ZMax);
  h->SetXTitle("Number of Hits");
  h->SetYTitle("Number of Pixels");
  h->GetXaxis()->CenterTitle();
  h->GetYaxis()->CenterTitle();
  h->SetTitleOffset(1.4, "y");
  h->SetFillColor(40);

  for (int ix = 1; ix <= NBinsX; ++ix) {
    for (int iy = 1; iy <= NBinsY; ++iy) {
      int const px = hIN->GetXaxis()->GetBinLowEdge(ix);
      int const py = hIN->GetYaxis()->GetBinLowEdge(iy);
      if (PLTPlane::IsFiducial(FidRegion, px, py)) {
        if (hIN->GetBinContent(ix, iy) > ZMax) {
          h->Fill(ZMax - hIN->GetMaximum() / (float) NBins);
        } else {
          h->Fill( hIN->GetBinContent(ix, iy) );
        }
      }
    }
  }

  return h;
}


TH2F* Get3x3EfficiencyHist (TH2F& HistIn, int const FirstCol, int const LastCol, int const FirstRow, int const LastRow)
{
  TH2F* HistEff = (TH2F*) HistIn.Clone(HistIn.GetName() + TString("_3x3Efficiency"));
  HistEff->Reset();

  for (int icol = 1; icol <= HistIn.GetNbinsX(); ++icol) {

    // What pixel column is this?  If it's outside the range skip it
    int const PixCol = HistEff->GetXaxis()->GetBinLowEdge(icol);
    if (PixCol < FirstCol || PixCol > LastCol) {
      continue;
    }

    for (int irow = 1; irow <= HistIn.GetNbinsY(); ++irow) {

      // What row is this?  If it's outside the range skip it
      int const PixRow = HistEff->GetYaxis()->GetBinLowEdge(irow);
      if (PixRow < FirstRow || PixRow > LastRow) {
        continue;
      }


      // For *this* pixel get the 3x3 surrounding values..  If a neighbor is outside of bounds we skip it
      std::vector<int> HitsNearThisPixel;
      for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
          if (PixCol + i >= FirstCol && PixCol + i <= LastCol && PixRow + j >= FirstRow && PixRow + j <= LastRow) {
            if ( (i != 0 || j != 0)) {
              //std::cout << "here " << HistIn.GetBinContent(icol + i, irow + j) << std::endl;
            }
            if ( (i != 0 || j != 0) && HistIn.GetBinContent(icol + i, irow + j) > 0) {
              HitsNearThisPixel.push_back( HistIn.GetBinContent(icol + i, irow + j));
            }
          }
        }
      }

      // Calcluate average neighbor occupancy
      float const NeighborsMean = std::accumulate(HitsNearThisPixel.begin(), HitsNearThisPixel.end(), 0) / (float) HitsNearThisPixel.size();

      // Set the content to itself divided by the average in neighbors
      if (NeighborsMean > 0) {
        HistEff->SetBinContent(icol, irow, HistIn.GetBinContent(icol, irow) / NeighborsMean);
        //printf("Efficiency: %2i %2i   %12.3f\n", PixCol, PixRow, HistIn.GetBinContent(icol, irow) / NeighborsMean);
      }

    }
  }

  return HistEff;
}




// CODE BELOW

int OccupancyPlots (std::string const DataFileName)
{
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  //Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering);
  PLTPlane::FiducialRegion MyFiducialRegion = PLTPlane::kFiducialRegion_m1_m1;
  //  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_m1_m1);
  Event.SetPlaneFiducialRegion(MyFiducialRegion);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  // Map for all ROC hists and canvas
  std::map<int, TH2F*>    hAllMap;
  std::map<int, TCanvas*> cAllMap;
  std::map<int, TH2F*>    hOccupancyMap;
  std::map<int, TCanvas*> cOccupancyMap;
  std::map<int, TH2F*>    hQuantileMap;
  std::map<int, TCanvas*> cQuantileMap;
  std::map<int, TCanvas*> cProjectionMap;
  std::map<int, TCanvas*> cEfficiencyMap;
  std::map<int, TH2F*>    hEfficiencyMap;
  std::map<int, TH1F*>    hEfficiency1DMap;
  std::map<int, TCanvas*> cCoincidenceMap;
  std::map<int, TH1F*>    hCoincidenceMap;
  std::map<int, TH2F*>    hMeanMap;
  std::map<int, TCanvas*> cMeanMap;




  // char buffer for writing names
  char BUFF[200];
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    // Loop over all planes with hits in event
    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {

      // THIS plane is
      PLTPlane* Plane = Event.Plane(ip);
      if (Plane->ROC() > 2) {
        std::cerr << "WARNING: ROC > 2 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->ROC() < 0) {
        std::cerr << "WARNING: ROC < 0 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->Channel() > 99) {
        std::cerr << "WARNING: Channel > 99 found: " << Plane->Channel() << std::endl;
        continue;
      }


      // Loop over all hits on this plane
      for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {

        // THIS hit is
        PLTHit* Hit = Plane->Hit(ihit);
        //printf("Channel ROC Row Col ADC: %2i %1i %2i %2i %4i %12i\n", Hit->Channel(), Hit->ROC(), Hit->Row(), Hit->Column(), Hit->ADC(), Event.EventNumber());

        // ID the plane and roc by 3 digit number
        int const id = 10 * Plane->Channel() + Plane->ROC();

        // If the hist doesn't exist yet we have to make it
        if (hOccupancyMap.count(id) == 0) {

          // Create new hist with the given name
          sprintf(BUFF, "Occupancy Ch%02i ROC%1i", Plane->Channel(), Plane->ROC());
          hOccupancyMap[id] = new TH2F(BUFF, BUFF, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL+1, PLTU::NROW,PLTU::FIRSTROW, PLTU::LASTROW+1);
          hOccupancyMap[id]->SetXTitle("Column");
          hOccupancyMap[id]->SetYTitle("Row");
          hOccupancyMap[id]->SetZTitle("Number of Hits");
          hOccupancyMap[id]->GetXaxis()->CenterTitle();
          hOccupancyMap[id]->GetYaxis()->CenterTitle();
          hOccupancyMap[id]->GetZaxis()->CenterTitle();
          hOccupancyMap[id]->SetTitleOffset(1.2, "y");
          hOccupancyMap[id]->SetTitleOffset(1.4, "z");
          hOccupancyMap[id]->SetFillColor(40); // We need this for projections later
          hOccupancyMap[id]->SetStats(false);




          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cOccupancyMap.count(Plane->Channel())) {

            // Create canvas with given name
            sprintf(BUFF, "Occupancy All Ch%02i", Plane->Channel());
            cAllMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
            cAllMap[Plane->Channel()]->Divide(3,3);

            sprintf(BUFF, "Occupancy Ch%02i", Plane->Channel());
            cOccupancyMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
            cOccupancyMap[Plane->Channel()]->Divide(3,3);

            sprintf(BUFF, "Occupancy w/ QuantilesCh%02i", Plane->Channel());
            cQuantileMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 800);
            cQuantileMap[Plane->Channel()]->Divide(3,2);

            sprintf(BUFF, "Occupancy Projection Ch%02i", Plane->Channel());
            cProjectionMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
            cProjectionMap[Plane->Channel()]->Divide(3,3);

            sprintf(BUFF, "Occupancy Efficiency Ch%02i", Plane->Channel());
            cEfficiencyMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
            cEfficiencyMap[Plane->Channel()]->Divide(3,3);

            sprintf(BUFF, "Planes hit in Ch%02i", Plane->Channel());
            cCoincidenceMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 400, 400);
            hCoincidenceMap[Plane->Channel()] = new TH1F(BUFF, BUFF, 7, 0, 7);

            sprintf(BUFF, "Occupancy by Mean Ch%02i", Plane->Channel());
            cMeanMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 800);
            cMeanMap[Plane->Channel()]->Divide(3,2);
          }
        }

        // Fill this histogram with the given id
        hOccupancyMap[id]->Fill(Hit->Column(), Hit->Row());
      }
    }

    // Loop over telescopes
    for (size_t ip = 0; ip != Event.NTelescopes(); ++ip) {

      // Grab telescope
      PLTTelescope* Tele = Event.Telescope(ip);

      // binary number for planes hit
      int phit = Tele->HitPlaneBits();

      // here we fill the Plot of Planes in Coincidence
      if(phit==0x1) hCoincidenceMap[Tele->Channel()]->Fill(0); //only first plane hit
      if(phit==0x2) hCoincidenceMap[Tele->Channel()]->Fill(1); //only 2nd plane hit
      if(phit==0x4) hCoincidenceMap[Tele->Channel()]->Fill(2); //only 3rd plane hit

      if(phit==0x3) hCoincidenceMap[Tele->Channel()]->Fill(3); //Plane 0 and 1 in coincidence
      if(phit==0x6) hCoincidenceMap[Tele->Channel()]->Fill(4); //Plane 1 and 2 in coincidence
      if(phit==0x5) hCoincidenceMap[Tele->Channel()]->Fill(5); //Plane 0 and 2 in coincidence
      if(phit==0x7) hCoincidenceMap[Tele->Channel()]->Fill(6); //All planes in coincidence
    }
  }

  std::cout << "Done reading events.  Will make some plots now" << std::endl;

  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, TH2F*>::iterator it = hOccupancyMap.begin(); it != hOccupancyMap.end(); ++it) {
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    printf("Drawing hist for Channel %2i ROC %i\n", Channel, ROC);

    // Grab a 1-D hist from the 2D occupancy
    TH1F* hOccupancy1D = PLTU::HistFrom2D(it->second, "", 50);

    // Draw the 2D and 1D distribution on occupancy canvas
    cOccupancyMap[Channel]->cd(ROC+1);
    hOccupancyMap[id]->Draw("colz");
    cOccupancyMap[Channel]->cd(ROC+3+1)->SetLogy(1);
    hOccupancy1D->SetMinimum(0.5);
    hOccupancy1D->Clone()->Draw("hist");
    cOccupancyMap[Channel]->cd(ROC+6+1);
    hOccupancy1D->SetMinimum(0);
    hOccupancy1D->Draw("hist");



    // Grab the quantile you're interested in here
    Double_t QProbability[1] = { 0.95 }; // Quantile positions in [0, 1]
    Double_t QValue[1];                  // Quantile values
    hOccupancy1D->GetQuantiles(1, QValue, QProbability);



    // Plot the occupancy z-scale determined by quantile
    cQuantileMap[Channel]->cd(ROC+1);
    hQuantileMap[it->first] = (TH2F*) it->second->Clone();
    hQuantileMap[it->first]->SetTitle(TString::Format("Occupancy by Quantiles Ch%02i ROC%1i", Channel, ROC));
    hQuantileMap[it->first]->Draw("colz");
    if(QValue[0] > 1 && it->second->GetMaximum() > QValue[0]) {
      hQuantileMap[it->first]->SetMaximum(QValue[0]);
    }
    cQuantileMap[Channel]->cd(ROC+3+1)->SetLogy(1);
    hOccupancy1D->Draw("hist");

    // Grab a line and draw it on the plot
    TLine* LineQuantile = new TLine(QValue[0], hOccupancy1D->GetMaximum(), QValue[0], .5);
    LineQuantile->SetLineColor(2);
    LineQuantile->SetLineWidth(2);
    LineQuantile->Draw("same");



    // Draw on projection canvas
    cProjectionMap[Channel]->cd(ROC+1);
    it->second->Draw("colz");

    // Column projection
    cProjectionMap[Channel]->cd(ROC+3+1);
    TH1D* hProjectionX = it->second->ProjectionX();
    hProjectionX->SetYTitle("Number of Hits");
    hProjectionX->GetYaxis()->CenterTitle();
    hProjectionX->SetTitleOffset(2, "Y");
    hProjectionX->Draw("hist");

    // Row projection
    cProjectionMap[Channel]->cd(ROC+6+1);
    TH1D* hProjectionY = it->second->ProjectionY();
    hProjectionY->SetYTitle("Number of Hits");
    hProjectionY->GetYaxis()->CenterTitle();
    hProjectionY->SetTitleOffset(2, "Y");
    hProjectionY->Draw("hist");



   

    // 2D Occupancy efficiency plots wrt 3x3
    hEfficiencyMap[it->first] = Get3x3EfficiencyHist(*hQuantileMap[it->first], PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::FIRSTROW, PLTU::LASTROW);
    hEfficiencyMap[it->first]->SetTitle( TString::Format("Occupancy 3x3Eff Ch%i ROC%i", Channel, ROC));
    hEfficiencyMap[it->first]->SetXTitle("Column");
    hEfficiencyMap[it->first]->SetYTitle("Row");
    hEfficiencyMap[it->first]->SetZTitle("Efficiency (relative to neighbors)");
    hEfficiencyMap[it->first]->GetXaxis()->CenterTitle();
    hEfficiencyMap[it->first]->GetYaxis()->CenterTitle();
    hEfficiencyMap[it->first]->GetZaxis()->CenterTitle();
    hEfficiencyMap[it->first]->SetTitleOffset(1.2, "z");
    hEfficiencyMap[it->first]->GetZaxis()->SetRangeUser(0, 3);
    hEfficiencyMap[it->first]->SetStats(false);

    // Make 1d plot
    hEfficiency1DMap[id] = FidHistFrom2D(hEfficiencyMap[it->first], "", 50, MyFiducialRegion);
    hEfficiency1DMap[id]->SetXTitle("3x3 relative efficiency");
    hEfficiency1DMap[id]->SetYTitle("Number of pixels");
    hEfficiency1DMap[id]->GetXaxis()->CenterTitle();
    hEfficiency1DMap[id]->GetYaxis()->CenterTitle();
    hEfficiency1DMap[id]->SetFillColor(40);


    cEfficiencyMap[Channel]->cd(ROC+1);
    hEfficiencyMap[it->first]->Draw("colz");

    cEfficiencyMap[Channel]->cd(ROC+3+1)->SetLogy(1);
    hEfficiency1DMap[it->first]->Clone()->Draw("");

    cEfficiencyMap[Channel]->cd(ROC+6+1);
    hEfficiency1DMap[it->first]->Draw("");


    // Occupancy normalized by mean
    sprintf(BUFF, "Occupancy by Mean Ch%02i ROC%1i", Channel, ROC);
    hMeanMap[id] = (TH2F*) it->second->Clone(BUFF);
    hMeanMap[id]->SetTitle(BUFF);
    TH1F* hMean = PLTU::HistFrom2D(hMeanMap[it->first], "", 50);
    hMeanMap[id]->Scale(1.0/hMean->GetMean());
    delete hMean;
    hMean = PLTU::HistFrom2D(hMeanMap[it->first], "", 50);
    hMeanMap[id]->SetZTitle("Relative number of hits");
    hMeanMap[id]->GetXaxis()->CenterTitle();
    hMeanMap[id]->GetYaxis()->CenterTitle();
    hMeanMap[id]->GetZaxis()->CenterTitle();
    hMeanMap[id]->SetTitleOffset(1.2, "z");
    hMeanMap[id]->SetStats(false);
    cMeanMap[Channel]->cd(ROC+1);
    hMeanMap[it->first]->Draw("colz");
    cMeanMap[Channel]->cd(ROC+3+1);
    hMean->Draw("hist");


    // Summary canvas of your three favorite plots
    cAllMap[Channel]->cd(ROC+1);
    it->second->Draw("colz");
    cAllMap[Channel]->cd(ROC+3+1);
    hQuantileMap[id]->Draw("colz");
    cAllMap[Channel]->cd(ROC+6+1);
    hEfficiencyMap[id]->Draw("colz");

  }


  for (std::map<int, TH1F*>::iterator it = hCoincidenceMap.begin(); it != hCoincidenceMap.end(); ++it) {
    cCoincidenceMap[it->first]->cd();

    // Naming for coincidence map
    char *bin[7] = { (char*)"ROC0"
      , (char*)"ROC1"
        , (char*)"ROC2"
        , (char*)"ROC0&1"
        , (char*)"ROC1&2"
        , (char*)"ROC0&2"
        , (char*)"All ROC Hit"
    };

    it->second->SetBit(TH1::kCanRebin);
    for (int r = 0; r <= 6; r++) {
      it->second->Fill(bin[r], 0);
    }
    it->second->LabelsDeflate();
    it->second->SetFillColor(40);
    it->second->SetYTitle("Number of Hits");
    it->second->GetYaxis()->SetTitleOffset(1.9);
    it->second->GetYaxis()->CenterTitle();
    it->second->Draw("");
  }

  // Loop over all canvas, save them, and delete them
  for (std::map<int, TCanvas*>::iterator it = cOccupancyMap.begin(); it != cOccupancyMap.end(); ++it) {
    it->second->SaveAs(TString::Format("plots/Occupancy_Ch%02i.png", it->first));
    delete it->second;
  }
  for (std::map<int, TCanvas*>::iterator it = cQuantileMap.begin(); it != cQuantileMap.end(); ++it) {
    it->second->SaveAs(TString::Format("plots/Occupancy_Quantile_Ch%02i.png", it->first));
    delete it->second;
  }
  for (std::map<int, TCanvas*>::iterator it = cProjectionMap.begin(); it != cProjectionMap.end(); ++it) {
    it->second->SaveAs(TString::Format("plots/Occupancy_Projection_Ch%02i.png", it->first));
    delete it->second;
  }
  for (std::map<int, TCanvas*>::iterator it = cEfficiencyMap.begin(); it != cEfficiencyMap.end(); ++it) {
    it->second->SaveAs(TString::Format("plots/Occupancy_Efficiency_Ch%02i.png", it->first));
    delete it->second;
  }
  for (std::map<int, TCanvas*>::iterator it = cCoincidenceMap.begin(); it != cCoincidenceMap.end(); ++it) {
    it->second->SaveAs(TString::Format("plots/Occupancy_Coincidence_Ch%02i.png", it->first));
  }
  for (std::map<int, TCanvas*>::iterator it = cMeanMap.begin(); it != cMeanMap.end(); ++it) {
    it->second->SaveAs(TString::Format("plots/Occupancy_Mean_Ch%02i.png", it->first));
  }
  for (std::map<int, TCanvas*>::iterator it = cAllMap.begin(); it != cAllMap.end(); ++it) {
    it->second->SaveAs(TString::Format("plots/Occupancy_All_Ch%02i.png", it->first));
    delete it->second;
  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  OccupancyPlots(DataFileName);

  return 0;
}
