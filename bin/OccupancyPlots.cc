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
const Double_t quantile = .97;
const int colCutL = 1;	// Number of columns to skip at the lower edge (currently only used for quantile calculation)
const int colCutU = 1;	// Number of columns to skip at the upper edge
const int rowCutL = 3;	// Number of rows to skip at the lower edge
const int rowCutU = 1;	// Number of rows to skip at the upper edge

// CODE BELOW

int OccupancyPlots (std::string const DataFileName)
{
  // Set some basic style
  PLTU::SetStyle();
  TH2F h1("OccupancyR1", "OccupancyR1", 50, 0, 50, 60, 30, 90);
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);

  // Map for all ROC hists and canvas
  std::map<int, TH2F*> hMap;
  std::map<int, TH2F*> mhMap;
  std::map<int, TH2F*> qhMap;	
  std::map<int, TH2F*> eff3Map;
  std::map<int, TH1F*> eff31dMap;
  std::map<int, TH1F*> phitsMap;

  std::map<int, TCanvas*> cMap;
  std::map<int, TCanvas*> cpMap;
  std::map<int, TCanvas*> oMap;
  std::map<int, TCanvas*> oProjMap;
  std::map<int, TCanvas*> cphitsMap;
  std::map<int, TCanvas*> cmhMap;

  // char buffer for writing names
  char BUFF[200];
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    
    //    std::cout << "Here!" << std::endl;
    
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
        if (hMap.count(id) == 0) {

          // Create new hist with the given name
          sprintf(BUFF, "Occupancy Ch%02i,ROC%1i", Plane->Channel(), Plane->ROC());
          std::cout << "Creating New Hist: " << BUFF << std::endl;
          hMap[id] = new TH2F(BUFF, BUFF, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
          hMap[id]->SetXTitle("Column");
          hMap[id]->SetYTitle("Row");
          hMap[id]->SetZTitle("Number of Hits");
          hMap[id]->GetXaxis()->CenterTitle();
          hMap[id]->GetYaxis()->CenterTitle();
          hMap[id]->GetZaxis()->CenterTitle();
          hMap[id]->SetTitleOffset(1.2, "y");
          hMap[id]->SetTitleOffset(1.4, "z");
          hMap[id]->SetFillColor(40); // We need this for projections later
          hMap[id]->SetStats(false);

          // 2 D Occupancy efficiency plots (mean normalized)
          sprintf(BUFF, "Occupancy Normalized by Mean Ch%02i,ROC%1i", Plane->Channel(), Plane->ROC());
          std::cout << "Creating New Hist: " << BUFF << std::endl;
          mhMap[id] = new TH2F(BUFF, BUFF, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NCOL-1, PLTU::FIRSTROW, PLTU::LASTROW);
          mhMap[id]->SetXTitle("Column");
          mhMap[id]->SetYTitle("Row");
          mhMap[id]->SetZTitle("Relative number of hits");
          mhMap[id]->GetXaxis()->CenterTitle();
          mhMap[id]->GetYaxis()->CenterTitle();
          mhMap[id]->GetZaxis()->CenterTitle();
          mhMap[id]->SetTitleOffset(1.2, "z");
          mhMap[id]->SetStats(false);

          // 2D Occupancy efficiency plots wrt 3x3
          sprintf(BUFF, "3x3 Occupancy Efficiency Ch%02i,ROC%1i", Plane->Channel(), Plane->ROC());
          std::cout << "Creating New Hist: " << BUFF << std::endl;
          eff3Map[id] = new TH2F(BUFF, BUFF, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NCOL-1, PLTU::FIRSTROW, PLTU::LASTROW);
          eff3Map[id]->SetXTitle("Column");
          eff3Map[id]->SetYTitle("Row");
          eff3Map[id]->SetZTitle("Efficiency (relative to neighbors)");
          eff3Map[id]->GetXaxis()->CenterTitle();
          eff3Map[id]->GetYaxis()->CenterTitle();
          eff3Map[id]->GetZaxis()->CenterTitle();
          eff3Map[id]->SetTitleOffset(1.2, "z");
          eff3Map[id]->GetZaxis()->SetRangeUser(0.,3.0);
          eff3Map[id]->SetStats(false);

          sprintf(BUFF, "3x3 Efficiency Distribution Ch%02i,ROC%1i", Plane->Channel(), Plane->ROC());
          eff31dMap[id] = new TH1F(BUFF, BUFF, 50, 0, 3.0);
          eff31dMap[id]->SetXTitle("3x3 relative efficiency");
          eff31dMap[id]->SetYTitle("Number of pixels with this efficiency");
          eff31dMap[id]->GetXaxis()->CenterTitle();
          eff31dMap[id]->GetYaxis()->CenterTitle();
          eff31dMap[id]->SetFillColor(40);

          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cMap.count(Plane->Channel())) {

            // Create canvas with given name
            sprintf(BUFF, "Normalized Occupancy Ch%02i", Plane->Channel());
            std::cout << "Creating New Canvas: " << BUFF << std::endl;
            cMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 800);
            cMap[Plane->Channel()]->Divide(3,2);

            sprintf(BUFF, "Occupancy Efficiency Ch%02i", Plane->Channel());
            std::cout << "Creating New Canvas: " << BUFF << std::endl;
            cmhMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 800);
            cmhMap[Plane->Channel()]->Divide(3,2);

            sprintf(BUFF, "Occupancy Projection Ch%02i", Plane->Channel());
            std::cout << "Creating New Canvas: " << BUFF << std::endl;
            cpMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
            cpMap[Plane->Channel()]->Divide(3,3);

            sprintf(BUFF, "Occupancy Maps Ch%02i", Plane->Channel());
            std::cout << "Creating New Canvas: " << BUFF << std::endl;
            oMap[Plane->Channel()]= new TCanvas(BUFF, BUFF, 1200, 1200);
            oMap[Plane->Channel()]->Divide(3,3);

            sprintf(BUFF, "Full Occupancy Maps Ch%02i", Plane->Channel());
            oProjMap[Plane->Channel()]=new TCanvas(BUFF, BUFF, 1200, 800);
            oProjMap[Plane->Channel()]->Divide(3,2);

            sprintf(BUFF, "Planes hit in Ch%02i", Plane->Channel());
            std::cout << "Creating New Canvas: " << BUFF << std::endl;
            cphitsMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 400, 400);
            phitsMap[Plane->Channel()] = new TH1F(BUFF, BUFF, 7, 0, 7);
          }
        }

        // Fill this histogram with the given id
        hMap[id]->Fill(Hit->Column(), Hit->Row());
      }
    }

    // Loop over telescopes
    for (size_t ip = 0; ip != Event.NTelescopes(); ++ip) {

      // Grab telescope
      PLTTelescope* Tele = Event.Telescope(ip);

      // binary number for planes hit
      int phit = Tele->HitPlaneBits();

      // here we fill the Plot of Planes in Coincidence
      if(phit==0x1) phitsMap[Tele->Channel()]->Fill(0); //only first plane hit
      if(phit==0x2) phitsMap[Tele->Channel()]->Fill(1); //only 2nd plane hit
      if(phit==0x4) phitsMap[Tele->Channel()]->Fill(2); //only 3rd plane hit

      if(phit==0x3) phitsMap[Tele->Channel()]->Fill(3); //Plane 0 and 1 in coincidence
      if(phit==0x6) phitsMap[Tele->Channel()]->Fill(4); //Plane 1 and 2 in coincidence
      if(phit==0x5) phitsMap[Tele->Channel()]->Fill(5); //Plane 0 and 2 in coincidence
      if(phit==0x7) phitsMap[Tele->Channel()]->Fill(6); //All planes in coincidence
    }
  }

  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, TH2F*>::iterator it = hMap.begin(); it != hMap.end(); ++it) {
    //    std::cout << "Here again!" << std::endl;
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    printf("Drawing hist for Channel %2i ROC %i\n", Channel, ROC);


    TH1F* hOccZ = PLTU::HistFrom2D(it->second, "", 50);

    int nq = 1;      // Number of quantiles to compute
    Double_t xq[nq]; // Quantile positions in [0, 1]
    Double_t yq[nq]; // Quantile values
    xq[0] = quantile;

    hOccZ->GetQuantiles(nq, yq, xq);

    cMap[Channel]->cd(ROC+3+1)->SetLogy(1);
    hOccZ->SetMinimum(0.5);
    hOccZ->Draw("hist");

    TLine* lQ = new TLine(yq[0], hOccZ->GetMaximum(), yq[0], .5);
    lQ->SetLineColor(2);
    lQ->SetLineWidth(2);
    lQ->Draw("SAME");

    oProjMap[Channel]->cd(ROC+3+1);
    hOccZ->Draw("hist");
    lQ->Draw("SAME");
    qhMap[it->first] = (TH2F*) it->second->Clone();
    sprintf(BUFF, "Occupancy Normalized by Quantiles Ch%02i,ROC%1i", Channel, ROC );
    qhMap[it->first]->SetTitle(BUFF);
    cMap[Channel]->cd(ROC+1);
    if(yq[0] > 1 && it->second->GetMaximum() > yq[0]) {
      std::cerr << "WARNING: Z axis of Ch" << Channel << ",ROC=" << ROC << " Occupancy plot trimmed to " << yq[0] << ". Maximum was: " << it->second->GetMaximum() << std::endl;
      qhMap[it->first]->SetMaximum(yq[0]);
    }

    qhMap[it->first]->Draw("colz");

    // Draw on projection canvas
    cpMap[Channel]->cd(ROC+3+1);
    TH1D* hpX = it->second->ProjectionX();
    hpX->SetYTitle("Number of Hits");
    hpX->GetYaxis()->CenterTitle();
    hpX->SetTitleOffset(2, "Y");
    hpX->Draw("hist");

    cpMap[Channel]->cd(ROC+6+1);
    TH1D* hpY = it->second->ProjectionY();
    hpY->SetYTitle("Number of Hits");
    hpY->GetYaxis()->CenterTitle();
    hpY->SetTitleOffset(2, "Y");
    hpY->Draw("hist");

    cpMap[Channel]->cd(ROC+1);
    gStyle->SetPalette(1);
    it->second->Draw("colz");

    oProjMap[Channel]->cd(ROC+1);
    gStyle->SetPalette(1);
    it->second->Draw("colz"); 

    // loop over Histograms and calculate Efficiency:
    int count = 0;
    float sum = 0;
    int cnt = 0;
    for(int ic = 1; ic <= it->second->GetNbinsX(); ++ic) {
      for(int ir = 1; ir <= it->second->GetNbinsY(); ++ir) {
        if (it->second->GetBinContent(ic, ir) == 0.0) {
          continue;
        }
        if(it->second->ProjectionX()->GetBinLowEdge(ic) == PLTU::FIRSTCOL || it->second->ProjectionX()->GetBinLowEdge(ic) == PLTU::LASTCOL) {
          continue;
        }
        if(it->second->ProjectionY()->GetBinLowEdge(ir) == PLTU::FIRSTROW || it->second->ProjectionY()->GetBinLowEdge(ir) == PLTU::LASTROW) {
          continue;
        }
        ++count;
        sum += it->second->GetBinContent(ic, ir);
      }
    }

    float norm = sum / count;
   
    for(int ic = 1; ic <= it->second->GetNbinsX(); ic++) {
      for(int ir = 1; ir <= it->second->GetNbinsY(); ir++) {
        int _ic = it->second->ProjectionX()->GetBinLowEdge(ic);
        int _ir = it->second->ProjectionY()->GetBinLowEdge(ir);
        //std::cout<<"column, row "<<_ic<<", "<<_ir<<" Occupancy "<<it->second->GetBinContent(ic, ir)<<std::endl;
        // These numbers may be replaced in terms of PLTU::{cols,rows}{Min,Max} and {col,row}Cut{L,U} (see quantile calculation)
        if ((_ic > PLTU::FIRSTCOL + 1 && _ic < PLTU::LASTCOL - 1) && (_ir < PLTU::LASTROW - 1  && _ir > PLTU::FIRSTROW + 1)) {
          cnt = 0;
          float neighbours[8];
          for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
              if (i == 0 && j == 0) {
                continue;
              }
              neighbours[cnt] = it->second->GetBinContent(ic + i, ir + j);
              ++cnt;
            }
          }
          float pixnorm = TMath::Mean(8, neighbours);
          float iEff = it->second->GetBinContent(ic,ir) / norm;
          //std::cout<<"Norm "<<norm<<" Pix Norm "<<pixnorm<<" Eff "<<iEff<<std::endl;
          //if ((_ic > PLTU::FIRSTCOL+1 && _ic < PLTU::LASTCOL-1) && (_ir < PLTU::LASTROW-1  && _ir > PLTU::FIRSTROW+1))
          mhMap[it->first]->SetBinContent(ic,ir, iEff);
          //std::cout<<"Col, Row "<< mhMap[it->first]->ProjectionX()->GetBinLowEdge(ic)<<", "<<mhMap[it->first]->ProjectionY()->GetBinLowEdge(ir)<<std::endl;
          if (pixnorm > 0) {
            iEff = it->second->GetBinContent(ic,ir) / pixnorm;
          } else {
            iEff = 0;
          }
          // These numbers may be replaced in terms of PLTU::{cols,rows}{Min,Max} and {col,row}Cut{L,U} (see quantile calculation)
          //if((_ic > PLTU::FIRSTCOL+1 && _ic < PLTU::LASTCOL-1) && (_ir < PLTU::LASTROW-1  && _ir > PLTU::FIRSTROW+1)) 
          eff3Map[it->first]->SetBinContent(ic, ir, iEff);           
          //  if((_ic > PLTU::FIRSTCOL+1 && _ic < PLTU::LASTCOL-1) && (_ir < PLTU::LASTROW-1  && _ir > PLTU::FIRSTROW+1)) 
          eff31dMap[it->first]->Fill(iEff);
        }
      }
    }

    cmhMap[Channel]->cd(ROC+1);
    eff3Map[it->first]->Draw("colz");
    cmhMap[Channel]->cd(ROC+3+1);
    eff31dMap[it->first]->Draw("");

    oMap[Channel]->cd(ROC+1);
    it->second->Draw("colz");
    oMap[Channel]->cd(ROC+3+1);
    qhMap[it->first]->Draw("colz");
    oMap[Channel]->cd(ROC+6+1);
    mhMap[it->first]->Draw("colz");
  }


  for (std::map<int, TH1F*>::iterator it = phitsMap.begin(); it != phitsMap.end(); ++it) {
    cphitsMap[it->first]->cd();

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
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    sprintf(BUFF, "plots/Occupancy_Ch%02i.gif", it->first);
    it->second->SaveAs(BUFF);
    delete it->second;      
    sprintf(BUFF, "plots/Occupancy_Projection_Ch%02i.gif", it->first);
    cpMap[it->first]->SaveAs(BUFF);
  }

  // Loop over all canvas, save them, and delete them
  for (std::map<int, TCanvas*>::iterator it = cmhMap.begin(); it != cmhMap.end(); ++it) {
    sprintf(BUFF, "plots/Occupancy_Efficiency_Ch%02i.gif", it->first);
    it->second->SaveAs(BUFF);
    delete it->second;
    sprintf(BUFF, "plots/Occupancy_all_Ch%02i.gif", it->first);
    oMap[it->first]->SaveAs(BUFF);
    sprintf(BUFF, "plots/Occupancy_allwQuantiles_Ch%02i.gif", it->first);
    oProjMap[it->first]->SaveAs(BUFF);
  }

  for (std::map<int, TCanvas*>::iterator it = cphitsMap.begin(); it != cphitsMap.end(); ++it) {
    sprintf(BUFF, "plots/Occupancy_Coincidence_Ch%02i.gif", it->first);
    it->second->SaveAs(BUFF);
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
