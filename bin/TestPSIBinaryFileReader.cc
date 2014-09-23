////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Apr  9 13:49:09 CEST 2014
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <utility>
#include <cmath>
#include <stdlib.h>
#include <algorithm>
#include <numeric>

#include "TLegend.h"
#include "TLegendEntry.h"
#include "TString.h"
#include "TSystem.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH3F.h"
#include "TProfile2D.h"
#include "TParameter.h"
#include "TTree.h"

#include "PSIBinaryFileReader.h"
#include "PLTPlane.h"
#include "PLTAlignment.h"

#define DEBUG false

template<typename T>
void FillIth(TH3F *h, int px, int py, std::vector<T> values, int i, bool fill_when_too_small = true){
/*
    Fill the i-th value of the vector into the histogram.
    px and py are the position to fill them at.

    If i is negative: count from the back, Python style!

    fill_when_too_small decides if we should fill with zero when not enough values are available,
*/
    // Count from the front
    // [0,1,2,3,...]
    if (i>=0){
        if (values.size() >= i+1)
            h->Fill(px, py, values[i]);
        else
            if (fill_when_too_small)
                h->Fill(px, py, 0);
    }
    // Count from the back
    // [...,-3, -2, -1]
    else{
      int abs_i = i*-1;

      if (values.size() >= abs_i)
        h->Fill(px, py, values[values.size()-abs_i]);
      else
        if (fill_when_too_small)
            h->Fill(px, py, 0);
    }

}

// Helper function for sorting a <int, float> pair according to the float
bool PairSort( std::pair<int, float> i, std::pair<int, float> j) { return (i.second < j.second); }

std::pair<int, float> FindILowestIndexAndValue( std::vector<float> values, int i=0){

    std::vector<std::pair<int, float> > index_and_values;

    if (DEBUG){
        std::cout << "FindILowestIndexAndValue, before sort: ";
        for (int iv = 0; iv != values.size(); iv++)
            std::cout << values[iv] << " ";
        std::cout << endl;
    }

    // SORT
    for (int iv = 0; iv != values.size(); iv++)
        index_and_values.push_back(std::make_pair(iv, values[iv]));


    std::sort(index_and_values.begin(), index_and_values.end(), PairSort);

   if (DEBUG){
        std::cout << "FindILowestIndexAndValue, after sort: ";
        for (int iv = 0; iv != index_and_values.size(); iv++)
            std::cout << index_and_values[iv].second << " ";
        std::cout << endl;
    }


    if ((i+1) <= index_and_values.size()){
        if (DEBUG)
            std::cout << "FindILowestIndexAndValue, i=" << i << " " << index_and_values[i].first << " : " << index_and_values[i].second << std::endl;
        return index_and_values[i];
    }
    else
        return std::make_pair(-1, TMath::QuietNaN());

}


std::string GetAlignmentFilename(int telescopeID, bool useInitial=0){

  // Get the correct Alignment for a given telescope
  // Initial Alignment (start values for finding alignment)
  if (useInitial){
    if ((telescopeID==1) || (telescopeID==2)){
      return "ALIGNMENT/Alignment_ETHTelescope_initial.dat";
    }
    else if (telescopeID==5){
      return "ALIGNMENT/Alignment_ETHTelescope_initial_4planes.dat";
    }
    else{
      std::cout << "ERROR: No Initial-Alignment file for telescopeID=" << telescopeID << std::endl;
      std::cout << "Exiting.." << std::endl;
      std::exit(0);
    }
  }
  // Real Alignment
  else{
    if (telescopeID==1)
      return "ALIGNMENT/Alignment_ETHTelescope_run316.dat";
    else if (telescopeID==2)
      return "ALIGNMENT/Alignment_ETHTelescope_run466.dat";
    else if (telescopeID==5){
      std::cout << "WARNING: USING INITIAL ALIGNMENT FOR 4-plane scope!" << std::endl;
      return "ALIGNMENT/Alignment_ETHTelescope_initial_4planes.dat";
    }
    else{
      std::cout << "ERROR: No Alignment file for telescopeID=" << telescopeID << std::endl;
      std::cout << "Exiting.." << std::endl;
      std::exit(0);
    }
  }
}


std::string GetMaskingFilename(int telescopeID){

  if (telescopeID == 1)
    return "outerPixelMask_Telescope1.txt";
  else if (telescopeID == 2)
    return "outerPixelMask_Telescope2.txt";
  else if (telescopeID == 5)
    return "outerPixelMask_Telescope5.txt";
  else{
    std::cout << "ERROR: No Masking file for telescopeID=" << telescopeID << std::endl;
    std::cout << "Exiting.." << std::endl;
    std::exit(0);
  }
}

std::string GetCalibrationFilename(int telescopeID){

  if (telescopeID == 1)
    return "GKCalibrationList.txt";
  else if (telescopeID == 2)
    return "GKCalibrationList_Telescope2.txt";
  else if (telescopeID == 5)
    return "GKCalibrationList_Telescope5.txt";
  else{
    std::cout << "ERROR: No Calibration file for telescopeID=" << telescopeID << std::endl;
    std::cout << "Exiting.." << std::endl;
    std::exit(0);
  }
}



int GetNumberOfROCS(int telescopeID){

  if ((telescopeID == 1) || (telescopeID == 2) ||  (telescopeID == 3))
    return 6;
  else if (telescopeID == 4)
    return 2;
  else if (telescopeID == 5)
    return 4;
  else{
    std::cout << "ERROR: Number of ROCs not defined for telescopeID=" << telescopeID << std::endl;
    std::cout << "Exiting.." << std::endl;
    std::exit(0);
  }
}


bool CheckEllipse(float dx, float dy, float max_dx, float max_dy){
  if ( (dx*dx/(max_dx*max_dx)) + (dy*dy/(max_dy*max_dy)) <= 1.01)
    return true;
  else
    return false;
}



void WriteHTML (TString const, TString const);

void Write2DCharge( TH3* h, TCanvas * Can, float maxz, TString OutDir){
  TProfile2D * ph = h->Project3DProfile("yx");
  ph->SetAxisRange(12,38,"X");
  ph->SetAxisRange(39,80,"Y");
  ph->SetMinimum(0);
  ph->SetMaximum(maxz);
  ph->Draw("COLZ");
  ph->Write();
  Can->SaveAs( OutDir+ TString(h->GetName()) +"_profile.gif");
  Can->SaveAs( OutDir+ TString(h->GetName()) +"_profile.pdf");
}


void WriteAngleHistograms( TH1 * h_before_chi2_x,
                          TH1 * h_before_chi2_y,
                          TH1 * h_after_chi2_x,
                          TH1 * h_after_chi2_y,
                          TCanvas * Can,
                          TString OutDir){

  h_before_chi2_x->SetLineColor( kRed );
  h_before_chi2_y->SetLineColor( kBlue );
  h_after_chi2_x->SetLineColor( kMagenta );
  h_after_chi2_y->SetLineColor( kBlack );

  h_before_chi2_x->SetLineStyle(1);
  h_before_chi2_y->SetLineStyle(2);
  h_after_chi2_x->SetLineStyle(3);
  h_after_chi2_y->SetLineStyle(4);

  h_before_chi2_x->SetLineWidth(2);
  h_before_chi2_y->SetLineWidth(2);
  h_after_chi2_x->SetLineWidth(2);
  h_after_chi2_y->SetLineWidth(2);

  float hmax = 1.1 * std::max( h_before_chi2_x->GetMaximum(),
                         std::max( h_before_chi2_y->GetMaximum(),
                             std::max( h_after_chi2_x->GetMaximum(),
                                h_after_chi2_y->GetMaximum())));


  h_before_chi2_x->SetAxisRange(0, hmax, "Y");
  h_before_chi2_y->SetAxisRange(0, hmax, "Y");
  h_after_chi2_x->SetAxisRange(0, hmax, "Y");
  h_after_chi2_y->SetAxisRange(0, hmax, "Y");

  h_before_chi2_x->GetXaxis()->SetTitle("Angle [rad]");
  h_before_chi2_x->GetYaxis()->SetTitle("Tracks");

  h_before_chi2_x->Draw();
  h_before_chi2_y->Draw("SAME");
  h_after_chi2_x->Draw("SAME");
  h_after_chi2_y->Draw("SAME");


  TLegend Leg(0.7, 0.5, 0.85, 0.88, "");
  Leg.SetFillColor(0);
  Leg.SetBorderSize(0);
  Leg.SetTextSize(0.04);
  Leg.AddEntry(h_before_chi2_x, "X Before Chi^{2}", "l");
  Leg.AddEntry(h_before_chi2_y, "Y Before Chi^{2}", "l");
  Leg.AddEntry(h_after_chi2_x, "X After Chi^{2}", "l");
  Leg.AddEntry(h_after_chi2_y, "Y After Chi^{2}", "l");

  Leg.Draw();

  Can->SaveAs( OutDir+ TString(h_before_chi2_x->GetName()) + ".gif");

}


float GetMaximumExceptBin(TH1* h, int ibin){

  if (h->GetMaximumBin() == ibin){
    return h->GetMaximum(h->GetMaximum());
  }
  else{
    return h->GetMaximum();
  }

}

void Write1DCharge( std::vector<TH3*> hs, TCanvas *Can, TString OutDir){

  if (hs.size()!=4){
    std::cerr << "Write1DCharge needs exactly four histograms!" << std::endl;
    return;
  }

  TH1* h15 = hs[0]->Project3D("Z");
  TH1* h30 = hs[1]->Project3D("Z");
  TH1* h45 = hs[2]->Project3D("Z");
  TH1* h60 = hs[3]->Project3D("Z");

  float hmax = 1.1 * std::max( GetMaximumExceptBin(h15, 1),
                       std::max( GetMaximumExceptBin(h30, 1),
                         std::max( GetMaximumExceptBin(h45, 1),
                            GetMaximumExceptBin(h60, 1))));

  h15->SetAxisRange(0,hmax,"Y");
  h30->SetAxisRange(0,hmax,"Y");
  h45->SetAxisRange(0,hmax,"Y");
  h60->SetAxisRange(0,hmax,"Y");


  h15->SetLineColor(1);
  h30->SetLineColor(2);
  h45->SetLineColor(3);
  h60->SetLineColor(4);

  h15->SetLineWidth(2);
  h30->SetLineWidth(2);
  h45->SetLineWidth(2);
  h60->SetLineWidth(2);

  h15->GetXaxis()->SetTitle("Charge (Electrons)");
  h15->GetYaxis()->SetTitle("Number of Hits");

  TLegend Leg(0.7, 0.5, 0.90, 0.88, "");
  Leg.SetFillColor(0);
  Leg.SetBorderSize(0);
  Leg.SetTextSize(0.05);
  Leg.AddEntry(h15, "R=1", "l");
  Leg.AddEntry(h30, "R=2", "l");
  Leg.AddEntry(h45, "R=3", "l");
  Leg.AddEntry(h60, "R=4", "l");

  h15->Draw();
  h30->Draw("SAME");
  h45->Draw("SAME");
  h60->Draw("SAME");
  Leg.Draw();

  h15->Write();
  h30->Write();
  h45->Write();
  h60->Write();

  Can->SaveAs( OutDir+ TString(hs[0]->GetName()) +".gif");
  Can->SaveAs( OutDir+ TString(hs[0]->GetName()) +".pdf");

}

void Write1DFraction(std::vector<TH1*> hs, TCanvas *Can, TString OutDir){

  if (hs.size()!=4){
    std::cerr << "Write1Dneeds exactly four histograms!" << std::endl;
    return;
  }


  float hmax = 1.1 * std::max( hs[0]->GetMaximum(),
                       std::max( hs[1]->GetMaximum(),
                         std::max( hs[2]->GetMaximum(),
                            hs[3]->GetMaximum())));

  hs[0]->SetAxisRange(0,hmax,"Y");
  hs[1]->SetAxisRange(0,hmax,"Y");
  hs[2]->SetAxisRange(0,hmax,"Y");
  hs[3]->SetAxisRange(0,hmax,"Y");


  hs[0]->SetLineColor(1);
  hs[1]->SetLineColor(2);
  hs[2]->SetLineColor(3);
  hs[3]->SetLineColor(4);

  hs[0]->SetLineWidth(2);
  hs[1]->SetLineWidth(2);
  hs[2]->SetLineWidth(2);
  hs[3]->SetLineWidth(2);

  hs[0]->GetXaxis()->SetTitle("Fraction of Hits in Cluster inside Radius");
  hs[0]->GetYaxis()->SetTitle("");

  TLegend Leg(0.7, 0.5, 0.90, 0.88, "");
  Leg.SetFillColor(0);
  Leg.SetBorderSize(0);
  Leg.SetTextSize(0.05);
  Leg.AddEntry(hs[0], "R=1", "l");
  Leg.AddEntry(hs[1], "R=2", "l");
  Leg.AddEntry(hs[2], "R=3", "l");
  Leg.AddEntry(hs[3], "R=4", "l");

  hs[0]->Draw();
  hs[1]->Draw("SAME");
  hs[2]->Draw("SAME");
  hs[3]->Draw("SAME");
  Leg.Draw();

  Can->SaveAs( OutDir+ TString(hs[0]->GetName()) +".gif");
  Can->SaveAs( OutDir+ TString(hs[0]->GetName()) +".pdf");

}

int FindHotPixels (std::string const InFileName,
                   TFile * out_f,
                   TString const RunNumber,
                   std::vector< std::vector< std::vector<int> > > & hot_pixels,
                   int telescopeID
                   )
{
  // FindHotPixels
  // Loop over the event and add pixels with > 10xmean occupancy (per ROC)
  // to the hot_pixels matrix.
    std::cout << "Entering FindHotPixels" << std::endl;

  gStyle->SetOptStat(0);
  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName,
                          GetCalibrationFilename(telescopeID),
                          GetAlignmentFilename(telescopeID), 4);
  BFR.GetAlignment()->SetErrors(telescopeID);

  // Apply Masking
  BFR.ReadPixelMask(GetMaskingFilename(telescopeID));

  std::cout << "Read pixel mask" << std::endl;

  // Add hot pixels we are given to mask
  // Since we now do multiple iterations in the histograms in one FindHotPixels call
  // instead of multiple FindHotPixels calls this should not be necessary anymore.
  for (int iroc=0; iroc != 6; iroc++){
    for (int icolrow=0; icolrow != hot_pixels[iroc].size(); icolrow++){
      std::cout << "HOT: " << iroc << " " << hot_pixels[iroc][icolrow][0] << " " << hot_pixels[iroc][icolrow][1] << std::endl;
      BFR.AddToPixelMask( 1, iroc, hot_pixels[iroc][icolrow][0], hot_pixels[iroc][icolrow][1]);
    }
  }

  BFR.CalculateLevels(10000, OutDir);

  std::cout << "calculated levels" << std::endl;

  // Prepare Occupancy histograms
  // x == columns
  // y == rows
  std::vector< TH2F > hOccupancy;
  for (int iroc = 0; iroc != 6; ++iroc){
    hOccupancy.push_back( TH2F( Form("Occupancy_ROC%i",iroc),
                                Form("Occupancy_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
  }

  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

    // print progress
    if (ievent % 10000 == 0) {
      std::cout << "Processing event: " << ievent << std::endl;
    }

    // loop over planes and fill occupancy histograms
    for (size_t iplane = 0; iplane != BFR.NPlanes(); ++iplane) {

      PLTPlane* Plane = BFR.Plane(iplane);

      for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {
        PLTHit* Hit = Plane->Hit(ihit);

        if (Hit->ROC() < 6) {
          hOccupancy[Hit->ROC()].Fill(Hit->Column(), Hit->Row());
        }
        else {
          std::cerr << "Oops, ROC >= 6?" << std::endl;
        }
      } // End of loop over hits
    } // End of loop over planes
  } // End of Event Loop


  int total_hot_pixels=0;

  while (1){

    int new_hot_pixels = 0;

    // Look for new hot pixels
    for (int iroc = 0; iroc != 6; ++iroc) {

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

      std::cout << "FindHotPixels, ROC: " << iroc << " Mean Occupancy: " << mean_occupancy << std::endl;

      // Find with an occupancy of more than 10 times the mean
      for (int icol=1; icol != hOccupancy[iroc].GetNbinsX()+1; icol++){
        for (int irow=1; irow != hOccupancy[iroc].GetNbinsY()+1; irow++){

          if (hOccupancy[iroc].GetBinContent(icol, irow) > 10*mean_occupancy){

            new_hot_pixels++;
            std::vector<int> colrow;
            // Store column and row
            // decrement by one (histogram bins vs real location)
            colrow.push_back( icol-1 );
            colrow.push_back( irow-1 );
            hOccupancy[iroc].SetBinContent( icol, irow, 0);
            hot_pixels[iroc].push_back( colrow );
            std::cout << "Masking ROC COL ROW: " << iroc << " " << icol-1 << " " << irow-1 << std::endl;
          }
        }
      }
    } // end of loop over ROCs

    total_hot_pixels += new_hot_pixels;

    if (new_hot_pixels==0)
      break;

  }

  std::cout << "Leaving FindHotPixels, found: " << total_hot_pixels << std::endl;
  return 0;
}



void TestPlaneEfficiency (std::string const InFileName,
                          TFile * out_f,
                          TString const RunNumber,
                          std::vector< std::vector< std::vector<int> > > & hot_pixels,
                          int plane_under_test,
                          int n_events,
                          int telescopeID)
{
  /* TestPlaneEfficiency

  o) Consider one plane to be the plane under test
  o) Require exactly one hit in all other planes
  o) This gives one track
  o) Then check if a hit was registered in the plane under test (within a given
      radius around the expected passing of the track)

  */

  // Track/Hit matching distance [cm]
  float max_dr_x = 0.03;
  float max_dr_y = 0.02;

  gStyle->SetOptStat(0);
  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName,
                          GetCalibrationFilename(telescopeID),
                          GetAlignmentFilename(telescopeID), 4);
  BFR.GetAlignment()->SetErrors(telescopeID);
  BFR.SetPlaneUnderTest(plane_under_test);

  // Apply Masking
  BFR.ReadPixelMask(GetMaskingFilename(telescopeID));

  // Add additional hot pixels (from FindHotPixels to mask)
  for (int iroc=0; iroc != 6; iroc++){
    for (int icolrow=0; icolrow != hot_pixels[iroc].size(); icolrow++){
      BFR.AddToPixelMask( 1, iroc, hot_pixels[iroc][icolrow][0], hot_pixels[iroc][icolrow][1]);
    }
  }

  BFR.CalculateLevels(10000, OutDir);

  // Prepare Occupancy histograms
  // Telescope coordinates
  TH2F hOccupancyNum   = TH2F(Form("PlaneEfficiency_ROC%i", plane_under_test), "PlaneEfficiency",   52, 0, 52, 80, 0, 80);
  TH2F hOccupancyDenom = TH2F(Form("TracksPassing_ROC%i", plane_under_test), Form("TracksPassing_ROC%i",plane_under_test), 52, 0, 52, 80, 0, 80);

  // Also have a second set - sliced according to event number
  int n_slices = 5;
  int slice_size = n_events/n_slices;
  std::vector<TH2F> hOccupancyNum_eventSlices;
  std::vector<TH2F> hOccupancyDenom_eventSlices;
  for (int i=0; i != n_slices; i++){

    TH2F h_n = TH2F(   Form("Numerator_ROC%i_slice%i",plane_under_test,i), "",   52, 0, 52, 80, 0, 80);
    TH2F h_d = TH2F(   Form("Denominator_ROC%i_slice%i",plane_under_test,i), "",   52, 0, 52, 80, 0, 80);

    hOccupancyNum_eventSlices.push_back( h_n );
    hOccupancyDenom_eventSlices.push_back( h_d );
  }

  TH3F hSumCharge1 = TH3F( Form("SumCharge_ROC%i", plane_under_test),  "Sum Charge within 1-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F hSumCharge2 = TH3F( Form("SumCharge2_ROC%i", plane_under_test), "Sum Charge within 2-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F hSumCharge3 = TH3F( Form("SumCharge3_ROC%i", plane_under_test), "Sum Charge within 3-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F hSumCharge4 = TH3F( Form("SumCharge4_ROC%i", plane_under_test), "Sum Charge within 4-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);

  TH3F h1stCharge1 = TH3F( Form("1stCharge_ROC%i", plane_under_test),  "1st Charge within 1-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F h1stCharge2 = TH3F( Form("1stCharge2_ROC%i", plane_under_test), "1st Charge within 2-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F h1stCharge3 = TH3F( Form("1stCharge3_ROC%i", plane_under_test), "1st Charge within 3-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F h1stCharge4 = TH3F( Form("1stCharge4_ROC%i", plane_under_test), "1st Charge within 4-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);

  TH3F h1stCharge1ADC = TH3F( Form("1stCharge_ADC_ROC%i", plane_under_test),  "1st Charge within 1-Pixel Ellipse", 52,0,52, 80,0,80, 50,-700, -200);
  TH3F h1stCharge2ADC = TH3F( Form("1stCharge2_ADC_ROC%i", plane_under_test), "1st Charge within 2-Pixel Ellipse", 52,0,52, 80,0,80, 50,-700, -200);
  TH3F h1stCharge3ADC = TH3F( Form("1stCharge3_ADC_ROC%i", plane_under_test), "1st Charge within 3-Pixel Ellipse", 52,0,52, 80,0,80, 50,-700, -200);
  TH3F h1stCharge4ADC = TH3F( Form("1stCharge4_ADC_ROC%i", plane_under_test), "1st Charge within 4-Pixel Ellipse", 52,0,52, 80,0,80, 50,-700, -200);

  TH3F h2ndCharge1 = TH3F( Form("2ndCharge_ROC%i", plane_under_test),  "2nd Charge within 1-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F h2ndCharge2 = TH3F( Form("2ndCharge2_ROC%i", plane_under_test), "2nd Charge within 2-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F h2ndCharge3 = TH3F( Form("2ndCharge3_ROC%i", plane_under_test), "2nd Charge within 3-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);
  TH3F h2ndCharge4 = TH3F( Form("2ndCharge4_ROC%i", plane_under_test), "2nd Charge within 4-Pixel Ellipse", 52,0,52, 80,0,80,50,0,50000);

  TH3F h2ndCharge1ADC = TH3F( Form("2ndCharge_ADC_ROC%i", plane_under_test),  "2nd Charge within 1-Pixel Ellipse", 52,0,52, 80,0,80, 50, -700, -200);
  TH3F h2ndCharge2ADC = TH3F( Form("2ndCharge2_ADC_ROC%i", plane_under_test), "2nd Charge within 2-Pixel Ellipse", 52,0,52, 80,0,80, 50, -700, -200);
  TH3F h2ndCharge3ADC = TH3F( Form("2ndCharge3_ADC_ROC%i", plane_under_test), "2nd Charge within 3-Pixel Ellipse", 52,0,52, 80,0,80, 50, -700, -200);
  TH3F h2ndCharge4ADC = TH3F( Form("2ndCharge4_ADC_ROC%i", plane_under_test), "2nd Charge within 4-Pixel Ellipse", 52,0,52, 80,0,80, 50, -700, -200);

  TH1F hFractionContainted1 = TH1F(Form("FractionContained1_ROC%i", plane_under_test), "Fraction Contained", 50, 0, 1.1);
  TH1F hFractionContainted2 = TH1F(Form("FractionContained2_ROC%i", plane_under_test), "Fraction Contained", 50, 0, 1.1);
  TH1F hFractionContainted3 = TH1F(Form("FractionContained3_ROC%i", plane_under_test), "Fraction Contained", 50, 0, 1.1);
  TH1F hFractionContainted4 = TH1F(Form("FractionContained4_ROC%i", plane_under_test), "Fraction Contained", 50, 0, 1.1);

  TH3F hClusterSize       = TH3F( Form("ClusterSize_ROC%i", plane_under_test), "Cluster Size", 52,0,52, 80,0,80,11,-0.5,10.5);

  TH1F hdtx = TH1F( Form("SinglePlaneTestDX_ROC%i",plane_under_test),   "SinglePlaneTest_DX",   100, -0.2, 0.2 );
  TH1F hdty = TH1F( Form("SinglePlaneTestDY_ROC%i",plane_under_test),   "SinglePlaneTest_DY",   100, -0.2, 0.2 );
  TH1F hdtr = TH1F( Form("SinglePlaneTestDR_ROC%i",plane_under_test),   "SinglePlaneTest_DR",   100, 0, 0.4 );

  TH1F hDrSecondCluster = TH1F(Form("DeltaRSecondCluster_ROC%i", plane_under_test), "#Delta R Second Cluster", 50, -2., 20);

  TH1F hChi2  = TH1F( Form("SinglePlaneTestChi2_ROC%i",plane_under_test),   "SinglePlaneTest_Chi2",    200, 0, 50 );
  TH1F hChi2X = TH1F( Form("SinglePlaneTestChi2X_ROC%i",plane_under_test),  "SinglePlaneTest_Chi2X",   100, 0, 20 );
  TH1F hChi2Y = TH1F( Form("SinglePlaneTestChi2Y_ROC%i",plane_under_test),  "SinglePlaneTest_Chi2Y",   100, 0, 20 );

  TH1F hAngleBeforeChi2X = TH1F( Form("SinglePlaneAngleBeforeChi2CutX_ROC%i",plane_under_test), "SinglePlaneAngleBeforeChi2CutX", 100, -0.04, 0.04 );
  TH1F hAngleBeforeChi2Y = TH1F( Form("SinglePlaneAngleBeforeChi2CutY_ROC%i",plane_under_test), "SinglePlaneAngleBeforeChi2CutX", 100, -0.04, 0.04 );

  TH1F hAngleAfterChi2X = TH1F( Form("SinglePlaneAngleAfterChi2CutX_ROC%i",plane_under_test), "SinglePlaneAngleAfterChi2CutX", 100, -0.04, 0.04 );
  TH1F hAngleAfterChi2Y = TH1F( Form("SinglePlaneAngleAfterChi2CutY_ROC%i",plane_under_test), "SinglePlaneAngleAfterChi2CutX", 100, -0.04, 0.04 );


  double tz = BFR.GetAlignment()->GetTZ(1, plane_under_test);
  std::cout << "Got TZ: " << tz << std::endl;

  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

    // print progress
    if (ievent % 10000 == 0) {
      std::cout << "Processing event: " << ievent << std::endl;
    }

    int i_slice = ievent/slice_size;
    if (i_slice==n_slices)
        i_slice--;

    // require exactly one track
    if (BFR.NTracks() == 1){

      // Calculate the Angle of the tracks
      double slopeX = BFR.Track(0)->fTVX / BFR.Track(0)->fTVZ;
      double slopeY = BFR.Track(0)->fTVY / BFR.Track(0)->fTVZ;

      double angleX = atan(slopeX);
      double angleY = atan(slopeY);

      hAngleBeforeChi2X.Fill(angleX);
      hAngleBeforeChi2Y.Fill(angleY);

      // Look at the 90% quantile
      if (BFR.Track(0)->Chi2X() > 6.25)
        continue;
      if (BFR.Track(0)->Chi2Y() > 6.25)
        continue;
    
      hAngleAfterChi2X.Fill(angleX);
      hAngleAfterChi2Y.Fill(angleY);
  
      // Only accept reasonably central events
      if ((fabs(angleX) > 0.02) || (fabs(angleY) > 0.02))
        continue;

      hChi2.Fill( BFR.Track(0)->Chi2());
      hChi2X.Fill( BFR.Track(0)->Chi2X());
      hChi2Y.Fill( BFR.Track(0)->Chi2Y());

      // Get the intersection of track and plane under test and fill
      // denominator histogram
      double tx = BFR.Track(0)->TX( tz );
      double ty = BFR.Track(0)->TY( tz );

      double lx = BFR.GetAlignment()->TtoLX( tx, ty, 1, plane_under_test);
      double ly = BFR.GetAlignment()->TtoLY( tx, ty, 1, plane_under_test);

      int px = BFR.GetAlignment()->PXfromLX( lx );
      int py = BFR.GetAlignment()->PYfromLY( ly );

      hOccupancyDenom.Fill( px, py );
      hOccupancyDenom_eventSlices[i_slice].Fill(px, py);

      PLTPlane* Plane = BFR.Plane( plane_under_test );

      std::vector<float> delta_rs;

      for (int icl = 0; icl != Plane->NClusters(); icl++){

        float cl_px = Plane->Cluster(icl)->PX();
        float cl_py = Plane->Cluster(icl)->PY();

        float delta_px = px - cl_px;
        float delta_py = py - cl_py;

        delta_rs.push_back(sqrt(delta_px*delta_px + delta_py*delta_py));
      }

      if (DEBUG)
        std::cout << "TestPlaneEfficiency. Before FindILowestIndexAndValue." << std::endl;

      int closest_cluster_index = FindILowestIndexAndValue(delta_rs).first;

      if (DEBUG)
        std::cout << "TestPlaneEfficiency. After FindILowestIndexAndValue. closest_cluster_index = " << closest_cluster_index << std::endl;

      if (delta_rs.size() == 1)
        hDrSecondCluster.Fill(-1.);
      else if (delta_rs.size() >= 2)
        hDrSecondCluster.Fill(FindILowestIndexAndValue(delta_rs, 1).second);

      // Now look for a close hit in the plane under test

      int matched = 0;

      std::vector<float> charges_in_ell_1;
      std::vector<float> charges_in_ell_2;
      std::vector<float> charges_in_ell_3;
      std::vector<float> charges_in_ell_4;

      std::vector<int> adcs_in_ell_1;
      std::vector<int> adcs_in_ell_2;
      std::vector<int> adcs_in_ell_3;
      std::vector<int> adcs_in_ell_4;

      // Make sure there is at least one cluster
      if (closest_cluster_index != -1){

          // Determine here if the closest cluster is actually close enouigh
          // and fill cluster size
          float cluster_dtx = (tx - Plane->Cluster(closest_cluster_index)->TX());
          float cluster_dty = (ty - Plane->Cluster(closest_cluster_index)->TY());
          if (CheckEllipse(cluster_dtx, cluster_dty, max_dr_x, max_dr_y)){
            hClusterSize.Fill(px, py, Plane->Cluster(closest_cluster_index)->NHits());
          }

          if (DEBUG)
            std::cout << "TestPlaneEfficiency. Before Loop over hits" << std::endl;

          // loop over all hits in the cluster and check distance to intersection
          for (int ih = 0; ih != Plane->Cluster(closest_cluster_index)->NHits(); ih++){

                 if (DEBUG)
                   std::cout << "TestPlaneEfficiency. ih = " << ih << std::endl;

                 float dtx = (tx - Plane->Cluster(closest_cluster_index)->Hit(ih)->TX());
                 float dty = (ty - Plane->Cluster(closest_cluster_index)->Hit(ih)->TY());
                 float dtr = sqrt( dtx*dtx + dty*dty );

                 hdtx.Fill( dtx );
                 hdty.Fill( dty );
                 hdtr.Fill( dtr );

                 int adc = Plane->Cluster(closest_cluster_index)->Hit(ih)->ADC();
                 float charge = Plane->Cluster(closest_cluster_index)->Hit(ih)->Charge();

                 if (CheckEllipse(dtx, dty, max_dr_x, max_dr_y))
                    matched++;

                 // 1 Pixel Ellipse
                 if (CheckEllipse(dtx, dty, 0.015, 0.01)){
                   adcs_in_ell_1.push_back(adc);
                   charges_in_ell_1.push_back(charge);
                 }

                 // 2 Pixel Ellipse
                 if (CheckEllipse(dtx, dty, 0.03, 0.02)){
                   adcs_in_ell_2.push_back(adc);
                   charges_in_ell_2.push_back(charge);
                 }

                 // 3 Pixel Ellipse
                 if (CheckEllipse(dtx, dty, 0.045, 0.03)){
                   adcs_in_ell_3.push_back(adc);
                   charges_in_ell_3.push_back(charge);
                 }

                 // 4 Pixel Ellipse
                 if (CheckEllipse(dtx, dty, 0.06, 0.04)){
                   adcs_in_ell_4.push_back(adc);
                   charges_in_ell_4.push_back(charge);
                 }

          } // end of loop over hits

          hFractionContainted1.Fill(1. * charges_in_ell_1.size() / Plane->Cluster(closest_cluster_index)->NHits());
          hFractionContainted2.Fill(1. * charges_in_ell_2.size() / Plane->Cluster(closest_cluster_index)->NHits());
          hFractionContainted3.Fill(1. * charges_in_ell_3.size() / Plane->Cluster(closest_cluster_index)->NHits());
          hFractionContainted4.Fill(1. * charges_in_ell_4.size() / Plane->Cluster(closest_cluster_index)->NHits());

      } // End of having at least one valid cluster

      if (DEBUG)
        std::cout << "TestPlaneEfficiency. After Loop over hits" << std::endl;

      // if there was at least one match: fill denominator
      if (matched > 0){
         hOccupancyNum.Fill( px, py );
         hOccupancyNum_eventSlices[i_slice].Fill(px, py, 1);
      }

      // Sort Charge Vectors
      std::sort(charges_in_ell_1.begin(), charges_in_ell_1.end());
      std::sort(charges_in_ell_2.begin(), charges_in_ell_2.end());
      std::sort(charges_in_ell_3.begin(), charges_in_ell_3.end());
      std::sort(charges_in_ell_4.begin(), charges_in_ell_4.end());

      // Sort ADC Vectors
      std::sort(adcs_in_ell_1.begin(), adcs_in_ell_1.end());
      std::sort(adcs_in_ell_2.begin(), adcs_in_ell_2.end());
      std::sort(adcs_in_ell_3.begin(), adcs_in_ell_3.end());
      std::sort(adcs_in_ell_4.begin(), adcs_in_ell_4.end());

      // Fill Sum of Charges
      hSumCharge1.Fill(px, py, std::accumulate(charges_in_ell_1.begin(), charges_in_ell_1.end(), 0));
      hSumCharge2.Fill(px, py, std::accumulate(charges_in_ell_2.begin(), charges_in_ell_2.end(), 0));
      hSumCharge3.Fill(px, py, std::accumulate(charges_in_ell_3.begin(), charges_in_ell_3.end(), 0));
      hSumCharge4.Fill(px, py, std::accumulate(charges_in_ell_4.begin(), charges_in_ell_4.end(), 0));

      // Fill Highest Charge
      FillIth(&h1stCharge1, px, py, charges_in_ell_1, -1);
      FillIth(&h1stCharge2, px, py, charges_in_ell_2, -1);
      FillIth(&h1stCharge3, px, py, charges_in_ell_3, -1);
      FillIth(&h1stCharge4, px, py, charges_in_ell_4, -1);

      // Fill Highest ADC
      FillIth(&h1stCharge1ADC, px, py, adcs_in_ell_1, -1);
      FillIth(&h1stCharge2ADC, px, py, adcs_in_ell_2, -1);
      FillIth(&h1stCharge3ADC, px, py, adcs_in_ell_3, -1);
      FillIth(&h1stCharge4ADC, px, py, adcs_in_ell_4, -1);

      // Fill Second Highest Charge
      // do NOT fill if not available!
      FillIth(&h2ndCharge1, px, py, charges_in_ell_1, -2, false);
      FillIth(&h2ndCharge2, px, py, charges_in_ell_2, -2, false);
      FillIth(&h2ndCharge3, px, py, charges_in_ell_3, -2, false);
      FillIth(&h2ndCharge4, px, py, charges_in_ell_4, -2, false);

      // Fill Second Highest ADC
      // do NOT fill if not available!
      FillIth(&h2ndCharge1ADC, px, py, adcs_in_ell_1, -2, false);
      FillIth(&h2ndCharge2ADC, px, py, adcs_in_ell_2, -2, false);
      FillIth(&h2ndCharge3ADC, px, py, adcs_in_ell_3, -2, false);
      FillIth(&h2ndCharge4ADC, px, py, adcs_in_ell_4, -2, false);

    } // end of having one track
  } // End of Event Loop



  // Remove masked areas from Occupancy Histograms
  const std::set<int> * pixelMask = BFR.GetPixelMask();

  std::cout << "Got PixelMask: "<<pixelMask->size() <<std::endl;

  // Loop over all masked pixels
  for (std::set<int>::const_iterator ipix = pixelMask->begin();
       ipix != pixelMask->end();
       ipix++){

         // Decode the integer
         int roc  = (*ipix % 100000) / 10000;
         int col  = (*ipix % 10000 ) / 100;
         int row  = (*ipix % 100);

         // Make sure this concerns the plane under test
         if (roc == plane_under_test){

             // Convert pixel row/column to local coordinates
             // deltaR(local) should be == deltaR(telescope) (within a plane)
             float masked_lx = BFR.GetAlignment()->PXtoLX( col);
             float masked_ly = BFR.GetAlignment()->PYtoLY( row);

             //std::cout << col << " " << row << " " << masked_lx << " " << masked_ly << std::endl;

             // Loop over the TH2
             for (int ibin_x = 1; ibin_x != hOccupancyNum.GetNbinsX()+2; ibin_x++){
               for (int ibin_y = 1; ibin_y != hOccupancyNum.GetNbinsY()+2; ibin_y++){

                 // Get the bin-centers
                 int px =  hOccupancyNum.GetXaxis()->GetBinCenter( ibin_x );
                 int py =  hOccupancyNum.GetYaxis()->GetBinCenter( ibin_y );

                 float lx = BFR.GetAlignment()->PXtoLX( px);
                 float ly = BFR.GetAlignment()->PYtoLY( py);

                 //std::cout << px << " " << py << " " << lx << " " << ly;

                 // And check if they are within matching-distance of a masked pixel
                 float dtx = lx-masked_lx;
                 float dty = ly-masked_ly;

                 if (CheckEllipse(dtx, dty, max_dr_x, max_dr_y)){
                   // If yes: set numerator and denominator to zero
                   hOccupancyNum.SetBinContent( ibin_x, ibin_y, 0);
                   hOccupancyDenom.SetBinContent( ibin_x, ibin_y, 0);

                   for (int ibin_z = 1; ibin_z != hSumCharge1.GetNbinsZ()+2; ibin_z++){

                    hSumCharge1.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    hSumCharge2.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    hSumCharge3.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    hSumCharge4.SetBinContent(ibin_x, ibin_y, ibin_z, 0);

                    h1stCharge1.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h1stCharge2.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h1stCharge3.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h1stCharge4.SetBinContent(ibin_x, ibin_y, ibin_z, 0);

                    h1stCharge1ADC.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h1stCharge2ADC.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h1stCharge3ADC.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h1stCharge4ADC.SetBinContent(ibin_x, ibin_y, ibin_z, 0);

                    h2ndCharge1.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h2ndCharge2.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h2ndCharge3.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h2ndCharge4.SetBinContent(ibin_x, ibin_y, ibin_z, 0);

                    h2ndCharge1ADC.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h2ndCharge2ADC.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h2ndCharge3ADC.SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    h2ndCharge4ADC.SetBinContent(ibin_x, ibin_y, ibin_z, 0);

                    hClusterSize.SetBinContent( ibin_x, ibin_y, ibin_z, 0);
                    for (int i=0; i != n_slices; i++){
                        hOccupancyNum_eventSlices[i].SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                        hOccupancyDenom_eventSlices[i].SetBinContent(ibin_x, ibin_y, ibin_z, 0);
                    }

                   }

                 }

               }
             }
        }
   } // end loop over pixels


  // Prepare drawing
  TCanvas Can;
  Can.cd();

  for (int i=0; i!= n_slices; i++){
    hOccupancyNum_eventSlices[i].Write();
    hOccupancyDenom_eventSlices[i].Write();
  }

  hOccupancyNum.SetMinimum(0);
  hOccupancyNum.SetAxisRange(12,38,"X");
  hOccupancyNum.SetAxisRange(39,80,"Y");
  hOccupancyNum.Draw("colz");
  hOccupancyNum.Write();


  hOccupancyDenom.Draw("colz");
  hOccupancyDenom.Write();
  Can.SaveAs( OutDir+TString(hOccupancyDenom.GetName()) + ".gif");
  Can.SaveAs( OutDir+TString(hOccupancyDenom.GetName()) + ".pdf");

  hOccupancyDenom.SetMinimum(0);
  hOccupancyNum.SetAxisRange(12,38,"X");
  hOccupancyNum.SetAxisRange(39,80,"Y");
  hOccupancyDenom.SetAxisRange(12,38,"X");
  hOccupancyDenom.SetAxisRange(39,80,"Y");

  hOccupancyDenom.Draw("colz");
  hOccupancyDenom.Write();
  Can.SaveAs( OutDir+TString(hOccupancyDenom.GetName()) + ".gif");
  Can.SaveAs( OutDir+TString(hOccupancyDenom.GetName()) + ".pdf");


  // Draw ratio of Occupancy histograms
  hOccupancyNum.Divide( &hOccupancyDenom );
  hOccupancyNum.SetMinimum(0);
  hOccupancyNum.SetMaximum(1.2);

  hOccupancyNum.Draw("colz");
  // Do not write the numerator-histo after division to the file
  Can.SaveAs( OutDir+TString(hOccupancyNum.GetName()) + ".gif");
  Can.SaveAs( OutDir+TString(hOccupancyNum.GetName()) + ".pdf");

  hdtx.Draw();
  hdtx.Write();
  Can.SaveAs( OutDir+ TString(hdtx.GetName()) +".gif");
  Can.SaveAs( OutDir+ TString(hdtx.GetName()) +".pdf");


  hdty.Draw();
  hdty.Write();
  Can.SaveAs(OutDir+ TString(hdty.GetName()) +".gif");
  Can.SaveAs(OutDir+ TString(hdty.GetName()) +".pdf");

  hdtr.Draw();
  hdtr.Write();
  Can.SaveAs( OutDir+ TString(hdtr.GetName()) +".gif");
  Can.SaveAs( OutDir+ TString(hdtr.GetName()) +".pdf");


  TF1 fun_chi2_6dof("chi2_6dof", "exp(-x/2.)*x*x/(4*16)");
  fun_chi2_6dof.SetRange(0.,50.);
  fun_chi2_6dof.SetNpx(1000);
  fun_chi2_6dof.Draw("SAME");

  hChi2.Scale(1/hChi2.Integral());
  hChi2.Draw();
  fun_chi2_6dof.Draw("SAME");
  hChi2.Write();
  Can.SaveAs( OutDir+ TString(hChi2.GetName()) +".gif");
  Can.SaveAs( OutDir+ TString(hChi2.GetName()) +".pdf");

  hChi2X.Scale( 1/ hChi2X.Integral());
  hChi2X.SetAxisRange(0, 0.07,"Y");
  hChi2X.Draw("hist");


  TF1 fun_chi2_3dof("chi2_3dof", "exp(-x/2.)*sqrt(x)/(5*sqrt(2*3.1415))");
  fun_chi2_3dof.SetRange(0.,20.);
  fun_chi2_3dof.SetNpx(1000);
  fun_chi2_3dof.Draw("SAME");


  hChi2X.Write();
  Can.SaveAs( OutDir+ TString(hChi2X.GetName()) +".gif");
  Can.SaveAs( OutDir+ TString(hChi2X.GetName()) +".pdf");


  hChi2Y.Scale(1/hChi2Y.Integral());
  hChi2Y.SetAxisRange(0, 0.08,"Y");
  hChi2Y.Draw();
  fun_chi2_3dof.Draw("SAME");
  hChi2Y.Write();
  Can.SaveAs( OutDir+ TString(hChi2Y.GetName()) +".gif");
  Can.SaveAs( OutDir+ TString(hChi2Y.GetName()) +".pdf");


  std::vector <TH3*> hs_mean_sum_charge;
  hs_mean_sum_charge.push_back( &hSumCharge1 );
  hs_mean_sum_charge.push_back( &hSumCharge2 );
  hs_mean_sum_charge.push_back( &hSumCharge3 );
  hs_mean_sum_charge.push_back( &hSumCharge4 );
  Write1DCharge(hs_mean_sum_charge, &Can, OutDir);

  std::vector <TH3*> hs_mean_1st_charge;
  hs_mean_1st_charge.push_back( &h1stCharge1 );
  hs_mean_1st_charge.push_back( &h1stCharge2 );
  hs_mean_1st_charge.push_back( &h1stCharge3 );
  hs_mean_1st_charge.push_back( &h1stCharge4 );
  Write1DCharge(hs_mean_1st_charge, &Can, OutDir);

  std::vector <TH3*> hs_mean_2nd_charge;
  hs_mean_2nd_charge.push_back( &h2ndCharge1 );
  hs_mean_2nd_charge.push_back( &h2ndCharge2 );
  hs_mean_2nd_charge.push_back( &h2ndCharge3 );
  hs_mean_2nd_charge.push_back( &h2ndCharge4 );
  Write1DCharge(hs_mean_2nd_charge, &Can, OutDir);

  std::vector <TH3*> hs_mean_1st_charge_adc;
  hs_mean_1st_charge_adc.push_back( &h1stCharge1ADC );
  hs_mean_1st_charge_adc.push_back( &h1stCharge2ADC );
  hs_mean_1st_charge_adc.push_back( &h1stCharge3ADC );
  hs_mean_1st_charge_adc.push_back( &h1stCharge4ADC );
  Write1DCharge(hs_mean_1st_charge_adc, &Can, OutDir);

  std::vector <TH3*> hs_mean_2nd_charge_adc;
  hs_mean_2nd_charge_adc.push_back( &h2ndCharge1ADC );
  hs_mean_2nd_charge_adc.push_back( &h2ndCharge2ADC );
  hs_mean_2nd_charge_adc.push_back( &h2ndCharge3ADC );
  hs_mean_2nd_charge_adc.push_back( &h2ndCharge4ADC );
  Write1DCharge(hs_mean_2nd_charge_adc, &Can, OutDir);


  float maxz;
  if (plane_under_test==1)
    maxz = 30000;
  if (plane_under_test==2)
    maxz = 30000;
  if (plane_under_test==3)
    maxz = 30000;
  if (plane_under_test==4)
    maxz = 50000;

  Write2DCharge( &hSumCharge1, &Can, maxz, OutDir);
  Write2DCharge( &hSumCharge2, &Can, maxz, OutDir);
  Write2DCharge( &hSumCharge3, &Can, maxz, OutDir);
  Write2DCharge( &hSumCharge4, &Can, maxz, OutDir);

  hSumCharge2.Write();
  hSumCharge4.Write();

  Write2DCharge( &h1stCharge1, &Can, maxz, OutDir);
  Write2DCharge( &h1stCharge2, &Can, maxz, OutDir);
  Write2DCharge( &h1stCharge3, &Can, maxz, OutDir);
  Write2DCharge( &h1stCharge4, &Can, maxz, OutDir);

  h1stCharge2.Write();
  h1stCharge4.Write();

  Write2DCharge( &h2ndCharge1, &Can, maxz, OutDir);
  Write2DCharge( &h2ndCharge2, &Can, maxz, OutDir);
  Write2DCharge( &h2ndCharge3, &Can, maxz, OutDir);
  Write2DCharge( &h2ndCharge4, &Can, maxz, OutDir);

  h2ndCharge2.Write();
  h2ndCharge4.Write();

  Write2DCharge( &hClusterSize, &Can, 7, OutDir);
  hClusterSize.Write();

  Can.SetLogy(1);
  hDrSecondCluster.Draw();
  Can.SaveAs( OutDir+ TString(hDrSecondCluster.GetName()) +".gif");
  Can.SetLogy(0);

  std::vector<TH1*> hs_fraction_contained;
  hs_fraction_contained.push_back(&hFractionContainted1);
  hs_fraction_contained.push_back(&hFractionContainted2);
  hs_fraction_contained.push_back(&hFractionContainted3);
  hs_fraction_contained.push_back(&hFractionContainted4);
  Write1DFraction(hs_fraction_contained, &Can, OutDir);

  Can.SaveAs( OutDir+ TString(hFractionContainted1.GetName()) +".gif");

  WriteAngleHistograms(&hAngleBeforeChi2X,
                       &hAngleBeforeChi2Y,
                       &hAngleAfterChi2X,
                       &hAngleAfterChi2Y,
                       &Can,
                       OutDir);

}

int TestPlaneEfficiencySilicon (std::string const InFileName,
                                 TFile * out_f,
                                 TString const RunNumber,
                                 std::vector< std::vector< std::vector<int> > > & hot_pixels,
                                 int telescopeID)
{
  /* TestPlaneEfficiencySilicon

  o) Consider one plane to be the plane under test
  o) Require at least one hit in both silicon planes
  o) Check if we have a hit in the other planes
  */

  gStyle->SetOptStat(0);
  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";

  // Open Alignment
  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName,
                          GetCalibrationFilename(telescopeID),
                          GetAlignmentFilename(telescopeID), 4);
  BFR.GetAlignment()->SetErrors(telescopeID);

  // Apply Masking
  BFR.ReadPixelMask("outerPixelMask_forSiEff.txt");

  // Add additional hot pixels (from FindHotPixels to mask)
  for (int iroc=0; iroc != 6; iroc++){
    for (int icolrow=0; icolrow != hot_pixels[iroc].size(); icolrow++){
      BFR.AddToPixelMask( 1, iroc, hot_pixels[iroc][icolrow][0], hot_pixels[iroc][icolrow][1]);
    }
  }

  BFR.CalculateLevels(10000, OutDir);

  // numerators and denominators for efficiency calculation
  std::vector<int> nums(6);
  std::vector<int> denoms(6);
  for (int i = 0; i != 6; i++){
    nums[i]   = 0;
    denoms[i] = 0;
  }

  int n_events = 0;
  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

    n_events++;

    // print progress
    if (ievent % 10000 == 0) {
      std::cout << "Processing event: " << ievent << std::endl;
    }

    // Initializes all planes as un-hit
    std::vector<bool> plane_is_hit;
    for (int i=0;i!=6;i++)
      plane_is_hit.push_back(false);

    // then loop and see where we have a hit
    for (int ihit = 0; ihit != BFR.NHits(); ++ihit)
      plane_is_hit[ BFR.Hit(ihit)->ROC() ] = true;

    // Check for Coincidence of silicon hits
    if (plane_is_hit[0] && plane_is_hit[5]){

      // Increment denominators
      for (int i = 0; i != 6; i++)
        denoms[i]++;

      // Increment numerators
      for (int i = 0; i != 6; i++)
        if (plane_is_hit[i])
          nums[i]++;
    }

  } // End of Event Loop

  // Store the numerators and denominators to the file
  for (int i=0; i!=6; i++){

    // Numerator
    TParameter<int> n;
    n.SetVal( nums[i]);
    n.Write( Form("SiliconEfficiencyNumeratorROC%i",i));

    // denominator
    TParameter<int> d;
    d.SetVal( denoms[i]);
    d.Write( Form("SiliconEfficiencyDenominatorROC%i",i));

  }

  return n_events;
}



int TestPSIBinaryFileReader (std::string const InFileName,
                             TFile * out_f,
                             TString const RunNumber,
                             int telescopeID)
{
  // Run default analysis
  const int NROC = GetNumberOfROCS(telescopeID);

  // Initialize hot-pixel array
  std::vector< std::vector< std::vector<int> > > hot_pixels;
  for (int iroc = 0; iroc != NROC; ++iroc) {
    std::vector< std::vector<int> > tmp;
    hot_pixels.push_back( tmp );
  }

  // Look for hot pixels
  //FindHotPixels(InFileName,
  //              out_f,
  //              RunNumber,
  //              hot_pixels,
  //              telescopeID);


  // For Telescopes from May testbeam:
  //   Do single plane studies
  if ((telescopeID == 1) || (telescopeID == 2)){

    int n_events = TestPlaneEfficiencySilicon(InFileName,
					      out_f,
					      RunNumber,
					      hot_pixels,
					      telescopeID);
    
    for (int iplane=1; iplane != 5; iplane++){
      std::cout << "Going to call TestPlaneEfficiency " << iplane << std::endl;

      TestPlaneEfficiency(InFileName,
                          out_f,
                          RunNumber,
                          hot_pixels,
                          iplane,
                          n_events,
                          telescopeID);
    }
  }

  // Setup Output Directory and gStyle
  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";
  std::cout<<OutDir<<std::endl;
  gStyle->SetOptStat(0);

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName,
                          GetCalibrationFilename(telescopeID),
                          GetAlignmentFilename(telescopeID), 4);
  BFR.GetAlignment()->SetErrors(telescopeID);
  FILE* f = fopen("MyGainCal.dat", "w");
  BFR.GetGainCal()->PrintGainCal(f);
  fclose(f);

  // Apply Masking
  BFR.ReadPixelMask(GetMaskingFilename(telescopeID));

  //Add hot pixels we found to mask
  for (int iroc=0; iroc != NROC; iroc++){
    for (int icolrow=0; icolrow != hot_pixels[iroc].size(); icolrow++){
      BFR.AddToPixelMask( 1, iroc, hot_pixels[iroc][icolrow][0], hot_pixels[iroc][icolrow][1]);
    }
  }

  BFR.CalculateLevels(10000, OutDir);


  // Prepare Occupancy histograms
  // x == columns
  // y == rows
  std::vector< TH2F > hOccupancy;
  for (int iroc = 0; iroc != NROC; ++iroc){
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
  for (int iroc = 0; iroc != NROC; ++iroc){
    hOccupancyLowPH.push_back( TH2F( Form("OccupancyLowPH_ROC%i",iroc),
                                Form("OccupancyLowPH_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
  }
  std::vector<TH2F> hOccupancyHighPH;
  for (int iroc = 0; iroc != NROC; ++iroc){
    hOccupancyHighPH.push_back( TH2F( Form("OccupancyHighPH_ROC%i",iroc),
                                Form("OccupancyHighPH_ROC%i",iroc), 52, 0, 52, 80, 0, 80));
  }

  std::vector<TH1F> hNHitsPerCluster;
  for (int iroc = 0; iroc != NROC; ++iroc){
    hNHitsPerCluster.push_back( TH1F( Form("NHitsPerCluster_ROC%i",iroc),
                                Form("NHitsPerCluster_ROC%i",iroc), 10, 0, 10));
  }

  std::vector<TH1F> hNClusters;
  for (int iroc = 0; iroc != NROC; ++iroc){
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
  TH1F* hPulseHeight[NROC][4];
  int const phMin = 0;
  int const phMax = 50000;
  int const phNBins = 50;
  // Standard
  for (int iroc = 0; iroc != NROC; ++iroc) {
    TString Name = TString::Format("PulseHeight_ROC%i_All", iroc);
    hPulseHeight[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeight_ROC%i_NPix1", iroc);
    hPulseHeight[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeight_ROC%i_NPix2", iroc);
    hPulseHeight[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeight_ROC%i_NPix3Plus", iroc);
    hPulseHeight[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phMax);
  }
  // For Tracks
  TH1F* hPulseHeightTrack6[NROC][4];
  for (int iroc = 0; iroc != NROC; ++iroc) {
    TString Name = TString::Format("PulseHeightTrack6_ROC%i_All", iroc);
    hPulseHeightTrack6[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightTrack6_ROC%i_NPix1", iroc);
    hPulseHeightTrack6[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightTrack6_ROC%i_NPix2", iroc);
    hPulseHeightTrack6[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightTrack6_ROC%i_NPix3Plus", iroc);
    hPulseHeightTrack6[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phMax);
  }
  // Long Histogram (see the Protons)
  TH1F* hPulseHeightLong[NROC][4];
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
  // For Tracks, using additional selections
  TH1F* hPulseHeightOffline[NROC][4];
  for (int iroc = 0; iroc != NROC; ++iroc) {
    TString Name = TString::Format("PulseHeightOffline_ROC%i_All", iroc);
    hPulseHeightOffline[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightOffline_ROC%i_NPix1", iroc);
    hPulseHeightOffline[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightOffline_ROC%i_NPix2", iroc);
    hPulseHeightOffline[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phMax);
    Name = TString::Format("PulseHeightOffline_ROC%i_NPix3Plus", iroc);
    hPulseHeightOffline[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phMax);
  }



  int const HistColors[4] = { 1, 4, 28, 2 };
  for (int iroc = 0; iroc != NROC; ++iroc) {
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
    hPulseHeightOffline[iroc][inpix]->SetXTitle("Charge (electrons)");
    hPulseHeightOffline[iroc][inpix]->SetYTitle("Number of Clusters");
    hPulseHeightOffline[iroc][inpix]->SetLineColor(HistColors[inpix]);
    }
  }

  // 2D Pulse Height maps for All and Track6
  double AvgPH2D[NROC][PLTU::NCOL][PLTU::NROW];
  int NAvgPH2D[NROC][PLTU::NCOL][PLTU::NROW];
  double AvgPH2DTrack6[NROC][PLTU::NCOL][PLTU::NROW];
  int NAvgPH2DTrack6[NROC][PLTU::NCOL][PLTU::NROW];
  for (int i = 0; i != NROC; ++i) {
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
  int NAvgPH[NROC][4];
  double AvgPH[NROC][4];
  std::vector< std::vector< TGraphErrors > > gAvgPH;

  for (int i = 0; i != NROC; ++i) {

    std::vector< TGraphErrors > tmp_gr_vector;

    for (int j = 0; j != 4; ++j) {
      NAvgPH[i][j] = 0;
      AvgPH[i][j] = 0;
      TGraphErrors gr;
      gr.SetName( Form("PulseHeightTime_ROC%i_NPix%i", i, j) );
      gr.SetTitle( Form("Average Pulse Height ROC %i NPix %i", i, j) );
      gr.GetXaxis()->SetTitle("Event Number");
      gr.GetYaxis()->SetTitle("Average Pulse Height (electrons)");
      gr.SetLineColor(HistColors[j]);
      gr.SetMarkerColor(HistColors[j]);
      gr.SetMinimum(0);
      gr.SetMaximum(60000);
      gr.GetXaxis()->SetTitle("Event Number");
      gr.GetYaxis()->SetTitle("Average Pulse Height (electrons)");
      tmp_gr_vector.push_back(gr);
    }
    gAvgPH.push_back(tmp_gr_vector);
  }

  // Track Chi2 Distribution
  TH1F hChi2("Chi2", "Chi2", 240, 0., 60.);

  // Track Chi2 Distribution
  TH1F hChi2X("Chi2X", "Chi2X", 240, 0., 60.);

  // Track Chi2 Distribution
  TH1F hChi2Y("Chi2Y", "Chi2Y", 240, 0., 60.);



  // Prepare Residual histograms
  // hResidual:    x=dX / y=dY
  // hResidualXdY: x=X  / y=dY
  // hResidualYdX: x=Y  / y=dX
  std::vector< TH2F > hResidual;
  std::vector< TH2F > hResidualXdY;
  std::vector< TH2F > hResidualYdX;

  for (int iroc = 0; iroc != NROC; ++iroc){
    hResidual.push_back( TH2F(  Form("Residual_ROC%i",iroc),
        Form("Residual_ROC%i",iroc), 100, -.15, .15, 100, -.15, .15));
    hResidualXdY.push_back( TH2F(  Form("ResidualXdY_ROC%i",iroc),
           Form("ResidualXdY_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
    hResidualYdX.push_back( TH2F(  Form("ResidualYdX_ROC%i",iroc),
           Form("ResidualYdX_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
  }

	float_t  onepc[NROC];
	float_t  twopc[NROC];
	float_t threepc[NROC];

  int const TimeWidth = 20000;
  int NGraphPoints = 0;

  // "times" for counting
  int const StartTime = 0;
  int ThisTime;

  // tree for timing information
  TTree *time_tree = new TTree("time_tree", "time_tree");
  
  long long br_time;
  int br_ievent;
  int br_hit_plane_bits;

  time_tree->Branch("time", &br_time);
  time_tree->Branch("ievent", &br_ievent);
  time_tree->Branch("hit_plane_bits", &br_hit_plane_bits);
  
  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

    ThisTime = ievent;

    // print progress
    if (ievent % 10000 == 0) {
      std::cout << "Processing event: " << ievent << std::endl;
    }

    // Write out information for PAD studies
    br_time = BFR.GetTime();
    br_ievent = ievent;
    br_hit_plane_bits = BFR.HitPlaneBits();
    time_tree->Fill();

    hCoincidenceMap.Fill(BFR.HitPlaneBits());

    if (ThisTime - (StartTime + NGraphPoints * TimeWidth) > TimeWidth) {
      for (int i = 0; i != NROC; ++i) {
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
    if (ieventdraw < 20 && BFR.NClusters() >= NROC) {
      BFR.DrawTracksAndHits( TString::Format(OutDir + "/Tracks_Ev%i.gif", ++ieventdraw).Data() );
    }

    for (size_t iplane = 0; iplane != BFR.NPlanes(); ++iplane) {
      PLTPlane* Plane = BFR.Plane(iplane);

      hNClusters[Plane->ROC()].Fill(Plane->NClusters());

      for (size_t icluster = 0; icluster != Plane->NClusters(); ++icluster) {
        PLTCluster* Cluster = Plane->Cluster(icluster);

        if (iplane < NROC) {
          hPulseHeight[iplane][0]->Fill(Cluster->Charge());
          hPulseHeightLong[iplane][0]->Fill(Cluster->Charge());

	  if (Cluster->Charge() > 300000) {
              continue;
          }
          PLTU::AddToRunningAverage(AvgPH2D[iplane][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], NAvgPH2D[iplane][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], Cluster->Charge());
          PLTU::AddToRunningAverage(AvgPH[iplane][0], NAvgPH[iplane][0], Cluster->Charge());
          if (Cluster->NHits() == 1) {
            hPulseHeight[iplane][1]->Fill(Cluster->Charge());
						onepc[iplane]++;
            hPulseHeightLong[iplane][1]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][1], NAvgPH[iplane][1], Cluster->Charge());
          } else if (Cluster->NHits() == 2) {
            hPulseHeight[iplane][2]->Fill(Cluster->Charge());
            twopc[iplane]++;
						hPulseHeightLong[iplane][2]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][2], NAvgPH[iplane][2], Cluster->Charge());
          } else if (Cluster->NHits() >= 3) {
            hPulseHeight[iplane][3]->Fill(Cluster->Charge());
            threepc[iplane]++;
						hPulseHeightLong[iplane][3]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][3], NAvgPH[iplane][3], Cluster->Charge());
          }
        }
      }

      for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {
        PLTHit* Hit = Plane->Hit(ihit);


        if (Hit->ROC() < NROC) {
          hOccupancy[Hit->ROC()].Fill(Hit->Column(), Hit->Row());
        } else {
          std::cerr << "Oops, ROC >= NROC?" << std::endl;
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

    if (telescopeID != 5 &&
	BFR.NTracks() == 1 &&
        BFR.Track(0)->NClusters() == NROC &&
        BFR.Track(0)->Cluster(0)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(1)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(2)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(3)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(4)->Charge() < 300000 &&
        BFR.Track(0)->Cluster(5)->Charge() < 300000 ) {

        PLTTrack* Track = BFR.Track(0);
        double slopeX = Track->fTVX / Track->fTVZ;
        double slopeY = Track->fTVY / Track->fTVZ;

        hChi2X.Fill( Track->Chi2X() );
        hChi2Y.Fill( Track->Chi2Y() );
        hChi2.Fill( Track->Chi2() );

        hTrackSlopeX.Fill( slopeX);
        hTrackSlopeY.Fill( slopeY);

        // Fill Residuals
        // Loop over clusters
        for (size_t icluster = 0; icluster < Track->NClusters(); icluster++){

          // Get the ROC in which this cluster was recorded and fill the
          // corresponding residual.
          int ROC = Track->Cluster(icluster)->ROC();

          // dX vs dY
          hResidual[ROC].Fill( Track->LResidualX( ROC ),
                               Track->LResidualY( ROC ));
          // X vs dY
          hResidualXdY[ROC].Fill( Track->Cluster(icluster)->LX(),
                                  Track->LResidualY( ROC ));
          // Y vs dX
          hResidualYdX[ROC].Fill( Track->Cluster(icluster)->LY(),
                                  Track->LResidualX( ROC ));

        } // end of loop over clusters

        for (size_t icluster = 0; icluster != Track->NClusters(); ++icluster) {
          PLTCluster* Cluster = Track->Cluster(icluster);

          if (Cluster->Charge() > 300000) {
              //printf("High Charge: %13.3E\n", Cluster->Charge());
              continue;
          }
          PLTU::AddToRunningAverage(AvgPH2DTrack6[Cluster->ROC()][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], NAvgPH2DTrack6[Cluster->ROC()][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], Cluster->Charge());

          if (Track->IsFiducial(1, 5, *(BFR.GetAlignment()), PLTPlane::kFiducialRegion_Diamond_m2_m2)) {
            hOccupancyTrack6[Cluster->ROC()].Fill(Cluster->PX(), Cluster->PY());
          }

          // Fill the Track6 PulseHeights
          hPulseHeightTrack6[Cluster->ROC()][0]->Fill(Cluster->Charge());
          if (Cluster->NHits() == 1) {
            hPulseHeightTrack6[Cluster->ROC()][1]->Fill(Cluster->Charge());
          } else if (Cluster->NHits() == 2) {
            hPulseHeightTrack6[Cluster->ROC()][2]->Fill(Cluster->Charge());
          } else if (Cluster->NHits() >= 3) {
            hPulseHeightTrack6[Cluster->ROC()][3]->Fill(Cluster->Charge());
          }

          // Fill the Offline PulseHeights (Track6+|Slope| < 0.01 in x and y )
          if ( (fabs(slopeX)< 0.01) && (fabs(slopeY)<0.01) ){
            hPulseHeightOffline[Cluster->ROC()][0]->Fill(Cluster->Charge());
            if (Cluster->NHits() == 1) {
              hPulseHeightOffline[Cluster->ROC()][1]->Fill(Cluster->Charge());
            } else if (Cluster->NHits() == 2) {
              hPulseHeightOffline[Cluster->ROC()][2]->Fill(Cluster->Charge());
            } else if (Cluster->NHits() >= 3) {
              hPulseHeightOffline[Cluster->ROC()][3]->Fill(Cluster->Charge());
            }
          }

        }

    }




  } // End of Event Loop

  time_tree->Write();


  // Catch up on PH by time graph
    for (int i = 0; i != NROC; ++i) {
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


  for (int iroc = 0; iroc != NROC; ++iroc) {

    // Draw Occupancy histograms
    hOccupancy[iroc].SetMinimum(0);
    hOccupancy[iroc].SetAxisRange(12,38,"X");
    hOccupancy[iroc].SetAxisRange(39,80,"Y");
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


		float_t oneovertwo[iroc],oneoverthree[iroc],twooverthree[iroc];

		oneovertwo[iroc] = onepc[iroc]/twopc[iroc];
		oneoverthree[iroc] = onepc[iroc]/threepc[iroc];
		twooverthree[iroc] = twopc[iroc]/threepc[iroc];

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
    TLegend lPulseHeight(0.7, 0.4, 0.95, 0.75, "Mean:");
   // lPulseHeight.SetTextAlign(11);
    lPulseHeight.SetFillStyle(0);
    lPulseHeight.SetBorderSize(0);
    lPulseHeight.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeight[iroc][0]->GetMean()), "")->SetTextColor(HistColors[0]);
    lPulseHeight.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeight[iroc][1]->GetMean()), "")->SetTextColor(HistColors[1]);
    lPulseHeight.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeight[iroc][2]->GetMean()), "")->SetTextColor(HistColors[2]);
    lPulseHeight.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeight[iroc][3]->GetMean()), "")->SetTextColor(HistColors[3]);
//    TLegend lRatio(0.75, 0.1, 0.90, 0.4, "Ratio:");
//    lRatio.SetTextAlign(11);
//    lRatio.SetFillStyle(0);
//    lRatio.SetBorderSize(0);
		lPulseHeight.AddEntry( "oneovertwo", TString::Format("%8.0f", oneovertwo[iroc])+" 1pix/2pix");
//    lRatio.AddEntry( "oneoverthree", TString::Format("%8.0f", oneoverthree, "")+" 1 over 3");
//    lRatio.AddEntry( "twooverthree", TString::Format("%8.0f", twooverthree, "")+" 2 over 3");
//    lRatio.Draw("same");
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
    hPulseHeightOffline[iroc][0]->SetTitle( TString::Format("Pulse Height Offline ROC%i", iroc) );
    hPulseHeightOffline[iroc][0]->Draw("hist");
    hPulseHeightOffline[iroc][1]->Draw("samehist");
    hPulseHeightOffline[iroc][2]->Draw("samehist");
    hPulseHeightOffline[iroc][3]->Draw("samehist");
    TLegend lPulseHeightOffline(0.75, 0.4, 0.90, 0.7, "Mean:");
    lPulseHeightOffline.SetTextAlign(11);
    lPulseHeightOffline.SetFillStyle(0);
    lPulseHeightOffline.SetBorderSize(0);
    lPulseHeightOffline.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeightOffline[iroc][0]->GetMean()), "")->SetTextColor(HistColors[0]);
    lPulseHeightOffline.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeightOffline[iroc][1]->GetMean()), "")->SetTextColor(HistColors[1]);
    lPulseHeightOffline.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeightOffline[iroc][2]->GetMean()), "")->SetTextColor(HistColors[2]);
    lPulseHeightOffline.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeightOffline[iroc][3]->GetMean()), "")->SetTextColor(HistColors[3]);
    lPulseHeightOffline.Draw("same");
    Leg.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeightOffline_ROC%i.gif", iroc));

    hPulseHeightOffline[iroc][0]->Write();
    hPulseHeightOffline[iroc][1]->Write();
    hPulseHeightOffline[iroc][2]->Write();
    hPulseHeightOffline[iroc][3]->Write();


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


  } // end of loop over ROCs

  TCanvas Can2("CoincidenceMap", "CoincidenceMap", 1200, 400);
  Can2.cd();
  Can2.SetLogy(1);
  hCoincidenceMap.Draw("");
  Can2.SaveAs(OutDir+"Occupancy_Coincidence.gif");
  Can2.SetLogy(0);

  Can.cd();
  hTrackSlopeX.Draw("hist");
  Can.SaveAs(OutDir+"TrackSlopeX.gif");

  Can.cd();
  hTrackSlopeY.Draw("hist");
  Can.SaveAs(OutDir+"TrackSlopeY.gif");

  Can.cd();
  gStyle->SetOptStat(000001111);

  hChi2.Draw("hist");
  Can.SaveAs(OutDir+"Chi2.gif");
  gStyle->SetOptStat(0);
  hChi2X.Scale( 1/hChi2X.Integral());

  Can.SaveAs(OutDir+"Chi2X.gif");
  gStyle->SetOptStat(0);

  hChi2Y.Draw("hist");
  Can.SaveAs(OutDir+"Chi2Y.gif");
  gStyle->SetOptStat(0);

  WriteHTML(PlotsDir + RunNumber,
            GetCalibrationFilename(telescopeID));

  return 0;
}


int DoAlignment (std::string const InFileName,
                 TFile * out_f,
                 TString const RunNumber,
                 int telescopeID)
{
  /* DoAlignment: Produce alignment constants and save
  them to NewAlignment.dat
  */

	TString const PlotsDir = "plots/";
	TString const OutDir = PlotsDir + RunNumber;

  gStyle->SetOptStat(0);

  std::vector<float> x_align;
  std::vector<float> y_align;
  std::vector<float> z_align;
  std::vector<float> r_align;

  for (int i=0; i!=6;i++){
    x_align.push_back(0);
    y_align.push_back(0);
    z_align.push_back(0);
    r_align.push_back(0);
  }

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName,
                          GetCalibrationFilename(telescopeID),
                          GetAlignmentFilename(telescopeID, true), 4);
  BFR.GetAlignment()->SetErrors(telescopeID, true);

  // Apply Masking
  BFR.ReadPixelMask(GetMaskingFilename(telescopeID));

  BFR.CalculateLevels(10000, OutDir);


  for (int ialign=0; ialign!=2;ialign++){


  for (int iroc=1;iroc!=5;iroc++){
    BFR.GetAlignment()->AddToLX( 1, iroc, x_align[iroc] );
    BFR.GetAlignment()->AddToLY( 1, iroc, y_align[iroc] );
    //BFR.GetAlignment()->AddToLR( 1, iroc, r_align[iroc] );
  }


  for (int iroc_align = 1; iroc_align != 6; ++iroc_align) {

    std::cout << "GOING TO ALIGN: " << iroc_align << std::endl;

    BFR.ResetFile();
    BFR.SetPlaneUnderTest( iroc_align );

    // Prepare Residual histograms
    // hResidual:    x=dX / y=dY
    // hResidualXdY: x=X  / y=dY
    // hResidualYdX: x=Y  / y=dX
    std::vector< TH2F > hResidual;
    std::vector< TH2F > hResidualXdY;
    std::vector< TH2F > hResidualYdX;



    // Reset residual histograms
    hResidual.clear();
    hResidualXdY.clear();
    hResidualYdX.clear();
    for (int iroc = 0; iroc != 6; ++iroc){
      hResidual.push_back( TH2F(  Form("Residual_ROC%i",iroc),
                                  Form("Residual_ROC%i",iroc), 200, -.2, .2, 200, -.2, .2));
      hResidualXdY.push_back( TH2F(  Form("ResidualXdY_ROC%i",iroc),
                                     Form("ResidualXdY_ROC%i",iroc), 133, -1, 0.995, 100, -.5, .5));
      hResidualYdX.push_back( TH2F(  Form("ResidualYdX_ROC%i",iroc),
                                     Form("ResidualYdX_ROC%i",iroc), 201, -1, 1, 100, -.5, .5));
    }

    // Event Loop
    for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

      if (! (BFR.NTracks()==1))
        continue;

      PLTTrack * Track = BFR.Track(0);

      if (! BFR.Plane(iroc_align)->NClusters()==1)
        continue;

      float max_charge = -1;
      float h_LX = -9999;
      float h_LY = -9999;

      for (int i=0; i != BFR.Plane(iroc_align)->Cluster(0)->NHits(); ++i){

          PLTHit * Hit = BFR.Plane(iroc_align)->Cluster(0)->Hit(i);

          if (Hit->Charge() > max_charge){
            max_charge = Hit->Charge();
            h_LX = Hit->LX();
            h_LY = Hit->LY();
          }
      }

      // float h_LX    = BFR.Plane(iroc_align)->Cluster(0)->LX();
      // float h_LY    = BFR.Plane(iroc_align)->Cluster(0)->LY();

      float track_TX = Track->TX(iroc_align);
      float track_TY = Track->TY(iroc_align);

      float track_LX = BFR.GetAlignment()->TtoLX( track_TX, track_TY, 1, iroc_align);
      float track_LY = BFR.GetAlignment()->TtoLY( track_TX, track_TY, 1, iroc_align);

      float d_LX =  (track_LX - h_LX);
      float d_LY =  (track_LY - h_LY);

      if (!(fabs(d_LX)<2))
        continue;

      if (!(fabs(d_LY)<2))
        continue;


      // dX vs dY
      hResidual[iroc_align].Fill( d_LX, d_LY);

      // X vs dY
      hResidualXdY[iroc_align].Fill( h_LX, d_LY);

      // Y vs dX
      hResidualYdX[iroc_align].Fill( h_LY, d_LX);

    } // end event loop

    std::cout << "RESIDUALS: " << hResidual[iroc_align].GetMean(1) << " " << hResidual[iroc_align].GetMean(2) << std::endl;
    std::cout << "RESIDUALS RMS: " << hResidual[iroc_align].GetRMS(1) << " " << hResidual[iroc_align].GetRMS(2) <<std::endl;




  std::cout << "Before: " << BFR.GetAlignment()->LX(1,iroc_align) << std::endl;

  x_align[iroc_align] +=  hResidual[iroc_align].GetMean(1);
  y_align[iroc_align] +=  hResidual[iroc_align].GetMean(2);
  r_align[iroc_align] +=  hResidualXdY[iroc_align].GetCorrelationFactor();


  std::cout << "After: " << BFR.GetAlignment()->LX(1,iroc_align) << std::endl;


  TCanvas Can;
  Can.cd();

  // 2D Residuals
  hResidual[iroc_align].Draw("colz");
  Can.SaveAs( OutDir+"/"+TString(hResidual[iroc_align].GetName()) + ".gif");

  // Residual X-Projection
  gStyle->SetOptStat(1111);
  hResidual[iroc_align].ProjectionX()->Draw();
  Can.SaveAs( OutDir+"/"+TString(hResidual[iroc_align].GetName()) + "_X.gif");

  // Residual Y-Projection
  hResidual[iroc_align].ProjectionY()->Draw();
  Can.SaveAs( OutDir+"/"+TString(hResidual[iroc_align].GetName()) + "_Y.gif");

  // 2D Residuals X/dY
  hResidualXdY[iroc_align].Draw("colz");
  Can.SaveAs( OutDir+"/"+TString(hResidualXdY[iroc_align].GetName()) + ".gif");

  // 2D Residuals Y/dX
  hResidualYdX[iroc_align].Draw("colz");
  Can.SaveAs( OutDir+"/"+TString(hResidualYdX[iroc_align].GetName()) + ".gif");


  for (int i=1; i!=5;i++){

    std::cout << i << " " << x_align[i] << " " << y_align[i] << " " << z_align[i] << " " << r_align[i] <<std::endl;
  }





} // end loop over rocs
BFR.GetAlignment()->WriteAlignmentFile("NewAlignment.dat");

} // end alignment loop




std::cout << "PART TWO!!!!!" << std::endl;


for (int ialign=1; ialign!=15;ialign++){

  BFR.ResetFile();
  BFR.SetAllPlanes();

  // Prepare Residual histograms
  // hResidual:    x=dX / y=dY
  // hResidualXdY: x=X  / y=dY
  // hResidualYdX: x=Y  / y=dX
  std::vector< TH2F > hResidual;
  std::vector< TH2F > hResidualXdY;
  std::vector< TH2F > hResidualYdX;
  std::vector< TGraph > gResidualXdY;

  // Reset residual histograms
  hResidual.clear();
  hResidualXdY.clear();
  hResidualYdX.clear();
  for (int iroc = 0; iroc != 6; ++iroc){
    hResidual.push_back( TH2F(  Form("Residual_ROC%i",iroc),
                                Form("Residual_ROC%i",iroc), 400, -.8, .8, 400, -.8, .8));
    hResidualXdY.push_back( TH2F(  Form("ResidualXdY_ROC%i",iroc),
                                   Form("ResidualXdY_ROC%i",iroc), 35, -0.2, 0.2, 100, -.2, .2));
    hResidualYdX.push_back( TH2F(  Form("ResidualYdX_ROC%i",iroc),
                                   Form("ResidualYdX_ROC%i",iroc), 41, -.2, .2, 100, -.2, .2));
    gResidualXdY.push_back( TGraph() );
  }

  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

    if (! (BFR.NTracks()==1))
      continue;

    PLTTrack * Track = BFR.Track(0);

    //if (Track->Chi2()>12)
    //  continue;

    for (int iroc=0; iroc!=6; iroc++){

      float d_LX = Track->LResidualX(iroc);
      float d_LY = Track->LResidualY(iroc);

      float cl_LX = -999;
      float cl_LY = -999;

      for (int icl=0; icl != Track->NClusters(); icl++){
        if (Track->Cluster(icl)->ROC() == iroc){

          cl_LX = Track->Cluster(icl)->LX();
          cl_LY = Track->Cluster(icl)->LY();


        }
      }

      // Hits instead of Clusters for Alignment
      // float h_LX = -999;
      // float h_LY = -999;
      // float max_charge = 0;
      // for (int i=0; i != BFR.Plane(iroc)->Cluster(0)->NHits(); ++i){
      //
      //   PLTHit * Hit = BFR.Plane(iroc)->Cluster(0)->Hit(i);
      //
      //   if (Hit->Charge() > max_charge){
      //     max_charge = Hit->Charge();
      //     h_LX = Hit->LX();
      //     h_LY = Hit->LY();
      //   }
      // }
      //
      // if (fabs(h_LX)>10 || fabs(h_LY)>10)
      //     continue
      //
      // float track_TX = Track->TX(iroc);
      // float track_TY = Track->TY(iroc);
      //
      // float track_LX = Alignment.TtoLX( track_TX, track_TY, 1, iroc);
      // float track_LY = Alignment.TtoLY( track_TX, track_TY, 1, iroc);
      //
      // float d_LX =  (track_LX - h_LX);
      // float d_LY =  (track_LY - h_LY);


      // dX vs dY
      hResidual[iroc].Fill( d_LX, d_LY);

      if ((fabs(d_LX) < 1000) && (fabs(d_LY) < 1000)){
          // X vs dY
          hResidualXdY[iroc].Fill( cl_LX, d_LY);

          // Y vs dX
          hResidualYdX[iroc].Fill( cl_LY, d_LX);

          gResidualXdY[iroc].SetPoint(gResidualXdY[iroc].GetN(), cl_LX, d_LY );
        }



    }


  } // end event loop

  for (int iroc=1; iroc!=6; iroc++){
  std::cout << "RESIDUALS: " << hResidual[iroc].GetMean(1) << " " << hResidual[iroc].GetMean(2) << std::endl;
  std::cout << "RESIDUALS RMS: " << hResidual[iroc].GetRMS(1) << " " << hResidual[iroc].GetRMS(2) <<std::endl;

  BFR.GetAlignment()->AddToLX(1, iroc, hResidual[iroc].GetMean(1));
  BFR.GetAlignment()->AddToLY(1, iroc, hResidual[iroc].GetMean(2));

  float angle = atan(hResidualXdY[iroc].GetCorrelationFactor()) ;


  TF1 linear_fun = TF1("","[0]+[1]*x");
  gResidualXdY[iroc].Fit(&linear_fun);


  float other_angle = atan(linear_fun.GetParameter(1));

  BFR.GetAlignment()->AddToLR(1, iroc, other_angle/3.);

  std::cout << "ROC: " << iroc << " Angle: " << angle << " Other Angle:" << other_angle << std::endl;

  TCanvas Can;
  Can.cd();

  gResidualXdY[iroc].Draw("AP*");
  Can.SaveAs( OutDir+"/"+TString::Format("gRes%i",iroc) + ".gif");

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

  }



  } // end alignment loop

  BFR.GetAlignment()->WriteAlignmentFile("NewAlignment.dat");

  return 0;
}


int FindResiduals(std::string const InFileName,
                  TFile * out_f,
                  TString const RunNumber,
                  int telescopeID){

  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber;

  gStyle->SetOptStat(0);

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName,
                          GetCalibrationFilename(telescopeID),
                          GetAlignmentFilename(telescopeID), 4);
  BFR.GetAlignment()->SetErrors(telescopeID, true);

  FILE* f = fopen("MyGainCal.dat", "w");
  BFR.GetGainCal()->PrintGainCal(f);
  fclose(f);
  BFR.ReadPixelMask(GetMaskingFilename(telescopeID));
  BFR.CalculateLevels(10000 ,OutDir);


  for (int ires=0; ires != 8; ires++){

    BFR.ResetFile();
    BFR.SetAllPlanes();

    TH1F hChi2_6_X( "", "", 100, 0, 10);
    TH1F hChi2_6_Y( "", "", 100, 0, 10);

    float chi2_6_x;
    float chi2_6_y;
    std::vector<float> chi2_5_x;
    std::vector<float> chi2_5_y;

    // Determine the 6-plane CHi2
    // Event Loop
    for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {


      if (! (BFR.NTracks()==1))
        continue;

      PLTTrack * Track = BFR.Track(0);
      hChi2_6_X.Fill( Track->Chi2X() );
      hChi2_6_Y.Fill( Track->Chi2Y() );

    } // end event loop



    for (int iplane=0; iplane!=6;iplane++){

      // Prepare Residual histograms
      TH1F hChi2_5_X( "", "", 100, 0, 10);
      TH1F hChi2_5_Y( "", "", 100, 0, 10);

      // Determine the 5-plane CHi2
      {
      // Initialize Reader
      BFR.ResetFile();
      BFR.SetPlaneUnderTest( iplane );

      // Event Loop
      for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

        if (! (BFR.NTracks()==1))
          continue;

        PLTTrack * Track = BFR.Track(0);
        hChi2_5_X.Fill( Track->Chi2X() );
        hChi2_5_Y.Fill( Track->Chi2Y() );


      } // end event loop
      } // end getting 5-plane Chi2



      TCanvas Can;
      Can.cd();

      hChi2_5_X.SetLineColor(3);
      hChi2_5_Y.SetLineColor(3);


      TF1 fun1("fun1","TMath::GammaDist(x, [0], 0, 2)/10.", 0, 10);
      TF1 fun2("fun2","TMath::GammaDist(x, [0], 0, 2)/10.", 0, 10);
      TF1 fun3("fun3","TMath::GammaDist(x, [0], 0, 2)/10.", 0, 10);
      TF1 fun4("fun4","TMath::GammaDist(x, [0], 0, 2)/10.", 0, 10);

      fun1.SetNpx(1000);
      fun2.SetNpx(1000);
      fun3.SetNpx(1000);
      fun4.SetNpx(1000);

      fun1.SetParameter(0, 2);
      fun2.SetParameter(0, 2);
      fun3.SetParameter(0, 2);
      fun4.SetParameter(0, 2);

      hChi2_5_X.Scale( 1./ hChi2_5_X.Integral());
      hChi2_5_Y.Scale( 1./ hChi2_5_Y.Integral());

      hChi2_6_X.Scale( 1./ hChi2_6_X.Integral());
      hChi2_6_Y.Scale( 1./ hChi2_6_Y.Integral());

      hChi2_5_X.Fit( &fun1 );
      hChi2_6_X.Fit( &fun2 );
      hChi2_5_Y.Fit( &fun3 );
      hChi2_6_Y.Fit( &fun4 );

      hChi2_5_X.Draw();
      hChi2_6_X.Draw("SAME");
      Can.SaveAs( OutDir+TString::Format("/FunWithChi2X_ROC%i",iplane) + ".gif");

      hChi2_5_Y.Draw();
      hChi2_6_Y.Draw("SAME");
      Can.SaveAs( OutDir+TString::Format("/FunWithChi2Y_ROC%i",iplane) + ".gif");

      chi2_5_x.push_back( fun1.GetParameter(0)*2 );
      chi2_5_y.push_back( fun3.GetParameter(0)*2 );

      chi2_6_x = fun2.GetParameter(0)*2 ;
      chi2_6_y = fun4.GetParameter(0)*2 ;

    } // end loop over planes


    if (true){

      float max_dchi2_x=-1;
      float max_dchi2_y=-1;
      int imax_x=-1;
      int imax_y=-1;

      std::cout << "SIX PLANES: " << chi2_6_x << " " << chi2_6_y << std::endl;

      for (int ic=0; ic!=6; ic++){

          std::cout << "X ROC: " << ic << " " << chi2_5_x[ic] <<std::endl;
          std::cout << "Y ROC: " << ic << " " << chi2_5_y[ic] <<std::endl;

          if (fabs(1-(chi2_6_x-chi2_5_x[ic])) > max_dchi2_y ){
            imax_x      = ic;
            max_dchi2_x = fabs(1-(chi2_6_x-chi2_5_x[ic]));
          }
          if (fabs(1-(chi2_6_y-chi2_5_y[ic])) > max_dchi2_y ){
            imax_y = ic;
            max_dchi2_y = fabs(1-(chi2_6_y-chi2_5_y[ic]));
          }


      }

      BFR.GetAlignment()->SetErrorX(imax_x, BFR.GetAlignment()->GetErrorX(imax_x)*(chi2_6_x-chi2_5_x[imax_x]));
      BFR.GetAlignment()->SetErrorY(imax_y, BFR.GetAlignment()->GetErrorY(imax_y)*(chi2_6_y-chi2_5_y[imax_y]));


      for (int ic=0; ic!=6; ic++){
        BFR.GetAlignment()->SetErrorX(ic, BFR.GetAlignment()->GetErrorX(ic)*(chi2_6_x/4.));
        BFR.GetAlignment()->SetErrorY(ic, BFR.GetAlignment()->GetErrorY(ic)*(chi2_6_y/4.));

        std::cout << "X ROC RES: " << ic << " " << BFR.GetAlignment()->GetErrorX(ic) <<std::endl;
        std::cout << "Y ROC RES: " << ic << " " << BFR.GetAlignment()->GetErrorY(ic) <<std::endl;
      }


    }

  } // end of residual finding loop

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

  int nplanes = 4;

  // LEVELS
  f << "<hr />\n";
  f << "<h2>Levels</h2>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"Levels_ROC%i.gif\"><img width=\"150\" src=\"Levels_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;

  // OCCUPANCY
  f << "<hr />\n";
  f << "<h2>Occupancy</h2>" << std::endl;
  f << "<a href=\"Occupancy_Coincidence.gif\"><img width=\"900\" src=\"Occupancy_Coincidence.gif\"></a>\n<br>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i.gif\"><img width=\"150\" src=\"Occupancy_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i_1DZ.gif\"><img width=\"150\" src=\"Occupancy_ROC%i_1DZ.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i_Quantile.gif\"><img width=\"150\" src=\"Occupancy_ROC%i_Quantile.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"Occupancy1DZ_ROC%i_Quantile.gif\"><img width=\"150\" src=\"Occupancy1DZ_ROC%i_Quantile.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;

  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i_3x3Efficiency.gif\"><img width=\"150\" src=\"Occupancy_ROC%i_3x3Efficiency.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"Occupancy_ROC%i_3x3Efficiency_1DZ.gif\"><img width=\"150\" src=\"Occupancy_ROC%i_3x3Efficiency_1DZ.gif\"></a>\n", i, i);
  }

  f << "<br>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"NClusters_ROC%i.gif\"><img width=\"150\" src=\"NClusters_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;
  f << "<br>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"NHitsPerCluster_ROC%i.gif\"><img width=\"150\" src=\"NHitsPerCluster_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>" << std::endl;

  // PULSE HEIGHT
  f << "<hr />\n";
  f << "<h2>Pulse Height</h2>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"PulseHeight_ROC%i.gif\"><img width=\"150\" src=\"PulseHeight_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"PulseHeightLong_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightLong_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"PulseHeightTime_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightTime_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"PulseHeightAvg2D_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightAvg2D_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"OccupancyLowPH_ROC%i.gif\"><img width=\"150\" src=\"OccupancyLowPH_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"OccupancyHighPH_ROC%i.gif\"><img width=\"150\" src=\"OccupancyHighPH_ROC%i.gif\"></a>\n", i, i);
  }

  // TRACKING
  f << "<h2>Tracking</h2>\n";
  f << "<a href=\"TrackSlopeX.gif\"><img width=\"150\" src=\"TrackSlopeX.gif\"></a>\n";
  f << "<a href=\"TrackSlopeY.gif\"><img width=\"150\" src=\"TrackSlopeY.gif\"></a>\n";

  f << "<br>" << std::endl;
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"OccupancyTrack6_ROC%i.gif\"><img width=\"150\" src=\"OccupancyTrack6_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"PulseHeightTrack6_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightTrack6_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"PulseHeightAvg2DTrack6_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightAvg2DTrack6_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";

  // OFFLINE
  f << "<h2>Straight Tracks</h2>\n";

  f << "<br>\n";
  for (int i = 0; i != nplanes; ++i) {
    f << Form("<a href=\"PulseHeightOffline_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightOffline_ROC%i.gif\"></a>\n", i, i);
  }
  f << "<br>\n";


  // TRACK RESIDUALS
  f << "<h2>Track Residuals</h2>\n";

  f << "<br>" << std::endl;
  for (int i = 0; i != nplanes; i++)
    f << Form("<a href=\"Residual_ROC%i_X.gif\"><img width=\"150\" src=\"Residual_ROC%i_X.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 0; i != nplanes; i++)
    f << Form("<a href=\"Residual_ROC%i_Y.gif\"><img width=\"150\" src=\"Residual_ROC%i_Y.gif\"></a>\n", i, i);
  f << "<br>\n";


  // Single Plane Studies
  f << "<h2>Single Plane Studies</h2>\n";
  f << "<br>" << std::endl;

  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"TracksPassing_ROC%i.gif\"><img width=\"150\" src=\"TracksPassing_ROC%i.gif\"></a>\n", i, i);
  f << "<br>\n";


  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"PlaneEfficiency_ROC%i.gif\"><img width=\"150\" src=\"PlaneEfficiency_ROC%i.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"ClusterSize_ROC%i_profile.gif\"><img width=\"150\" src=\"ClusterSize_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";


  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"SumCharge_ROC%i.gif\"><img width=\"150\" src=\"SumCharge_ROC%i.gif\"></a>\n", i, i);
    f << "<br>\n";

  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"1stCharge_ROC%i.gif\"><img width=\"150\" src=\"1stCharge_ROC%i.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"2ndCharge_ROC%i.gif\"><img width=\"150\" src=\"2ndCharge_ROC%i.gif\"></a>\n", i, i);
  f << "<br>\n";



  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"SumCharge_ROC%i_profile.gif\"><img width=\"150\" src=\"SumCharge_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
   f << Form("<a href=\"SumCharge2_ROC%i_profile.gif\"><img width=\"150\" src=\"SumCharge2_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
   f << Form("<a href=\"SumCharge3_ROC%i_profile.gif\"><img width=\"150\" src=\"SumCharge3_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
  f << Form("<a href=\"SumCharge3_ROC%i_profile.gif\"><img width=\"150\" src=\"SumCharge4_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";


  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"1stCharge_ROC%i_profile.gif\"><img width=\"150\" src=\"1stCharge_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
   f << Form("<a href=\"1stCharge2_ROC%i_profile.gif\"><img width=\"150\" src=\"1stCharge2_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
   f << Form("<a href=\"1stCharge3_ROC%i_profile.gif\"><img width=\"150\" src=\"1stCharge3_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
  f << Form("<a href=\"1stCharge3_ROC%i_profile.gif\"><img width=\"150\" src=\"1stCharge4_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"2ndCharge_ROC%i_profile.gif\"><img width=\"150\" src=\"2ndCharge_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
   f << Form("<a href=\"2ndCharge2_ROC%i_profile.gif\"><img width=\"150\" src=\"2ndCharge2_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
   f << Form("<a href=\"2ndCharge3_ROC%i_profile.gif\"><img width=\"150\" src=\"2ndCharge3_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
  f << Form("<a href=\"2ndCharge3_ROC%i_profile.gif\"><img width=\"150\" src=\"2ndCharge4_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";




  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"SinglePlaneTestChi2_ROC%i.gif\"><img width=\"150\" src=\"SinglePlaneTestChi2_ROC%i.gif\"></a>\n", i, i);
  f << "<br>\n";

for (int i = 1; i != 5; i++)
  f << Form("<a href=\"SinglePlaneTestChi2X_ROC%i.gif\"><img width=\"150\" src=\"SinglePlaneTestChi2X_ROC%i.gif\"></a>\n", i, i);
f << "<br>\n";

for (int i = 1; i != 5; i++)
  f << Form("<a href=\"SinglePlaneTestChi2Y_ROC%i.gif\"><img width=\"150\" src=\"SinglePlaneTestChi2Y_ROC%i.gif\"></a>\n", i, i);
f << "<br>\n";


  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"SinglePlaneTestDY_ROC%i.gif\"><img width=\"150\" src=\"SinglePlaneTestDY_ROC%i.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"SinglePlaneTestDR_ROC%i.gif\"><img width=\"150\" src=\"SinglePlaneTestDR_ROC%i.gif\"></a>\n", i, i);
  f << "<br>\n";


  // EVENT DISPLAYS
  f << "<h2>Event Displays</h2>\n";

  f << "<br>" << std::endl;
  for (int irow = 0; irow != 4; irow++){
    for (int icol = 1; icol != nplanes; ++icol) {
      int i = irow*5+icol;
      f << Form("<a href=\"Tracks_Ev%i.gif\"><img width=\"150\" src=\"Tracks_Ev%i.gif\"></a>\n", i, i);
    }
    f << "<br>\n";
  }
  f << "<br>\n";

  // HOT PIXELS
  // f << "<h2>Hot Pixels</h2>\n";
  //
  // f << "<br>" << std::endl;
  // for (int i = 0; i != 6; i++)
  //   f << Form("<a href=\"PulseHeightHot_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightHot_ROC%i.gif\"></a>\n", i, i);
  // f << "<br>\n";
  //
  //
  // for (int i = 0; i != 6; i++)
  //   f << Form("<a href=\"OccupancyHot_ROC%i.gif\"><img width=\"150\" src=\"OccupancyHot_ROC%i.gif\"></a>\n", i, i);
  // f << "<br>\n";

  f << "</body></html>";
  f.close();


  return;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " InFileName action telescopeUD" << std::endl;
    std::cerr << "action: 0 for analysis, 1 for producing alignment file, 2 for finding residuals" << std::endl;
    return 1;
  }

  /* There three useage modes: analysis, alignment and residuals

  analysis uses alignment and residuals for the given telescope to perform
    global and single plane studies

  alignment starts with all alignment constants zero and does several iterations
    to minimize the residuals. All planes are shifted in x and y and rotated
    around the z-axis. Residual plots of the last iteration are saved.
    As output the file NewAlignment.dat is produced.

  residuals tries to find the correct residuals for tracking
  */

  std::string const InFileName = argv[1];
  TString const FullRunName = InFileName;
  Int_t const Index = FullRunName.Index("bt2014_09r",0);
  TString const RunNumber = FullRunName(Index+10,6);
  gSystem->mkdir("./plots/" + RunNumber);

  // 0: Analysis
  // 1: Alignment
  // 2: Residuals
  int action = atoi(argv[2]);

  // Telescope IDs:
  // 1: First May-Testbeam Telescope (Si, PolyA, PolyD, S86,  S105, Si)
  // 2: Second May-Tesbeam Telescope (Si, PolyB, PolyD, S108, Si,   Si)
  // 3: First Silicon + 1 Diamond Telescope (July Testbeam)
  // 4: Two-Plane Silicon Telescope (July Testbeam)
  // 5: Four-Plane Silicon (?) Telescope (September Testbeam)
  int telescopeID = atoi(argv[3]);

  // Open a ROOT file to store histograms in
  // do it here and pass to all functions we call
  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";
  TFile out_f( OutDir + "histos.root", "recreate");

  // ALIGNMENT
  if (action==1)
    DoAlignment(InFileName,
                &out_f,
                RunNumber,
                telescopeID);

  // RESIDUAL CALCULATION
  else if (action==2)
    FindResiduals(InFileName,
                  &out_f,
                  RunNumber,
                  telescopeID);

  // ANALYSIS
  else
    TestPSIBinaryFileReader(InFileName,
                            &out_f,
                            RunNumber,
                            telescopeID);


  return 0;
}
