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
#include "TH3F.h"
#include "TProfile2D.h"
#include "TParameter.h"


void WriteHTML (TString const, TString const);

int FindHotPixels (std::string const InFileName,
                   TFile * out_f,
                   std::string const CalibrationList,
                   TString const RunNumber,
                   std::vector< std::vector< std::vector<int> > > & hot_pixels
                   )
{
  // FindHotPixels
  // Loop over the event and add pixels with > 10xmean occupancy (per ROC)
  // to the hot_pixels matrix.
    std::cout << "Entering FindHotPixels" << std::endl;

  gStyle->SetOptStat(0);
  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";

  // Open Alignment
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope.dat");

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName, CalibrationList);
  BFR.SetTrackingAlignment(&Alignment);


  //FILE* f = fopen("MyGainCal.dat", "w");
  //BFR.GetGainCal()->PrintGainCal(f);
  //fclose(f);

  // Mask four extra rows on each boundary of the diamond sensors
  BFR.ReadPixelMask( "outerPixelMask_Telescope2.txt");

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

      // Find with an occupancy of more than 10 times the meanm
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
                          std::string const CalibrationList,
                          TString const RunNumber,
                          std::vector< std::vector< std::vector<int> > > & hot_pixels,
                          int plane_under_test)
{
  /* TestPlaneEfficiency

  o) Consider one plane to be the plane under test
  o) Require exactly one hit in all other planes
  o) This gives one track
  o) Then check if a hit was registered in the plane under test (within a given
      radius around the expected passing of the track)

  */

  // Track/Hit matching distance [cm]
  float max_dr = 0.03;


  gStyle->SetOptStat(0);
  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";

  // Open Alignment
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope.dat");

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName, CalibrationList);
  BFR.SetTrackingAlignment(&Alignment);
  BFR.SetPlaneUnderTest( plane_under_test );

  // Mask four extra rows on each boundary of the diamond sensors
  BFR.ReadPixelMask( "outerPixelMask_Telescope2.txt");

  // Add additional hot pixels (from FindHotPixels to mask)
  for (int iroc=0; iroc != 6; iroc++){
    for (int icolrow=0; icolrow != hot_pixels[iroc].size(); icolrow++){
      BFR.AddToPixelMask( 1, iroc, hot_pixels[iroc][icolrow][0], hot_pixels[iroc][icolrow][1]);
    }
  }

  BFR.CalculateLevels(10000, OutDir);

  // Prepare Occupancy histograms
  // Telescope coordinates
  TH2F hOccupancyNum   = TH2F(   Form("PlaneEfficiency_ROC%i",plane_under_test), "PlaneEfficiency",   52, 0, 52, 80, 0, 80);
  TH2F hOccupancyDenom = TH2F(  Form("TracksPassing_ROC%i",plane_under_test), Form("TracksPassing_ROC%i",plane_under_test), 52, 0, 52, 80, 0, 80);

  TH3F hCharge01       = TH3F( Form("Charge_ROC%i", plane_under_test),   "Mean Charge within #Delta R < 0.01 cm", 52,0,52, 80,0,80,50,0,50000);
  TH3F hCharge02       = TH3F( Form("Charge02_ROC%i", plane_under_test), "Mean Charge within #Delta R < 0.02 cm", 52,0,52, 80,0,80,50,0,50000);
  TH3F hCharge03       = TH3F( Form("Charge03_ROC%i", plane_under_test), "Mean Charge within #Delta R < 0.03 cm", 52,0,52, 80,0,80,50,0,50000);
  TH3F hCharge04       = TH3F( Form("Charge04_ROC%i", plane_under_test), "Mean Charge within #Delta R < 0.04 cm", 52,0,52, 80,0,80,50,0,50000);


  TH1F hdtx = TH1F( Form("SinglePlaneTestDX_ROC%i",plane_under_test),   "SinglePlaneTest_DX",   100, -0.2, 0.2 );
  TH1F hdty = TH1F( Form("SinglePlaneTestDY_ROC%i",plane_under_test),   "SinglePlaneTest_DY",   100, -0.2, 0.2 );
  TH1F hdtr = TH1F( Form("SinglePlaneTestDR_ROC%i",plane_under_test),   "SinglePlaneTest_DR",   100, 0, 0.4 );


  TH1F hChi2  = TH1F( Form("SinglePlaneTestChi2_ROC%i",plane_under_test),   "SinglePlaneTest_Chi2",    200, 0, 50 );
  TH1F hChi2X = TH1F( Form("SinglePlaneTestChi2X_ROC%i",plane_under_test),  "SinglePlaneTest_Chi2X",   100, 0, 20 );
  TH1F hChi2Y = TH1F( Form("SinglePlaneTestChi2Y_ROC%i",plane_under_test),  "SinglePlaneTest_Chi2Y",   100, 0, 20 );



  double tz = Alignment.GetTZ(1, plane_under_test);
  std::cout << "Got TZ: " << tz << std::endl;

  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

    // print progress
    if (ievent % 10000 == 0) {
      std::cout << "Processing event: " << ievent << std::endl;
    }

    // require exactly one track
    if (BFR.NTracks() == 1){

      // Look at the 90% quantile
      if (BFR.Track(0)->Chi2X() > 6.25)
        continue;
      if (BFR.Track(0)->Chi2Y() > 6.25)
        continue;


      hChi2.Fill( BFR.Track(0)->Chi2());
      hChi2X.Fill( BFR.Track(0)->Chi2X());
      hChi2Y.Fill( BFR.Track(0)->Chi2Y());

      // Get the intersection of track and plane under test and fill
      // denominator histogram
      double tx = BFR.Track(0)->TX( tz );
      double ty = BFR.Track(0)->TY( tz );

      double lx = Alignment.TtoLX( tx, ty, 1, plane_under_test);
      double ly = Alignment.TtoLY( tx, ty, 1, plane_under_test);

      int px = Alignment.PXfromLX( lx );
      int py = Alignment.PYfromLY( ly );

      hOccupancyDenom.Fill( px, py );

      // Now look for a close hit in the plane under test
      PLTPlane* Plane = BFR.Plane( plane_under_test );
      bool matched = false;

      float sum01 = 0;
      float sum02 = 0;
      float sum03 = 0;
      float sum04 = 0;

      // loop over all hits and check distance to intersection
      for (int ih = 0; ih != Plane->NHits(); ih++){
             float dtx = (tx - Plane->Hit(ih)->TX());
             float dty = (ty - Plane->Hit(ih)->TY());
             float dtr = sqrt( dtx*dtx + dty*dty );

             hdtx.Fill( dtx );
             hdty.Fill( dty );
             hdtr.Fill( dtr );

             if (sqrt( dtx*dtx + dty*dty ) < 0.01)
               sum01 += Plane->Hit(ih)->Charge();

             if (sqrt( dtx*dtx + dty*dty ) < 0.02)
               sum02 += Plane->Hit(ih)->Charge();

             if (sqrt( dtx*dtx + dty*dty ) < 0.03)
               sum03 += Plane->Hit(ih)->Charge();

             if (sqrt( dtx*dtx + dty*dty ) < 0.04)
               sum04 += Plane->Hit(ih)->Charge();

             if (sqrt( dtx*dtx + dty*dty ) < max_dr)
               matched=true;

       } // end of loop over hits

       hCharge01.Fill( px, py, sum01);
       hCharge02.Fill( px, py, sum02);
       hCharge03.Fill( px, py, sum03);
       hCharge04.Fill( px, py, sum04);

       // if there was at least one match: fill denominator
       if (matched)
         hOccupancyNum.Fill( px, py );

    } // end of having one track
  } // End of Event Loop


  // Remove masked areas from Occupancy Histograms
  const std::set<int> * pixelMask = BFR.GetPixelMask();

  // Loop over all masked pixels
  for (std::set<int>::const_iterator ipix = pixelMask->begin();
       ipix != pixelMask->end();
       ipix++){

         // Decode the integer
         int ch   = *ipix /  100000;
         int roc  = (*ipix % 100000) / 10000;
         int col  = (*ipix % 10000 ) / 100;
         int row  = (*ipix % 100);

         // Make sure this concerns the plane under test
         if (roc == plane_under_test){

             // Convert pixel row/column to local coordinates
             // deltaR(local) should be == deltaR(telescope) (within a plane)
             float masked_lx = Alignment.PXtoLX( col);
             float masked_ly = Alignment.PYtoLY( row);

             // Loop over the TH2
             for (int ibin_x = 1; ibin_x != hOccupancyNum.GetNbinsX()+2; ibin_x++){
               for (int ibin_y = 1; ibin_y != hOccupancyNum.GetNbinsY()+2; ibin_y++){

                 // Get the bin-centers
                 int px =  hOccupancyNum.GetXaxis()->GetBinCenter( ibin_x );
                 int py =  hOccupancyNum.GetYaxis()->GetBinCenter( ibin_y );

                 float lx = Alignment.PXtoLX( px);
                 float ly = Alignment.PYtoLY( py);

                 // And check if they are within matching-distance of a masked pixel
                 if (sqrt( (lx-masked_lx)*(lx-masked_lx)+(ly-masked_ly)*(ly-masked_ly) ) < max_dr){
                   // If yes: set numerator and denominator to zero
                   hOccupancyNum.SetBinContent( ibin_x, ibin_y, 0);
                   hOccupancyDenom.SetBinContent( ibin_x, ibin_y, 0);

                   for (int ibin_z = 1; ibin_z != hCharge01.GetNbinsZ()+2; ibin_z++){
                     hCharge01.SetBinContent( ibin_x, ibin_y, ibin_z, 0);
                     hCharge02.SetBinContent( ibin_x, ibin_y, ibin_z, 0);
                     hCharge03.SetBinContent( ibin_x, ibin_y, ibin_z, 0);
                     hCharge04.SetBinContent( ibin_x, ibin_y, ibin_z, 0);
                   }

                 }
               }
             }
        }
   } // end loop over pixels


  // Prepare drawing
  TCanvas Can;
  Can.cd();

  hOccupancyDenom.SetMinimum(0);
  hOccupancyNum.SetAxisRange(18,34,"X");
  hOccupancyNum.SetAxisRange(45,76,"Y");
  hOccupancyDenom.SetAxisRange(18,34,"X");
  hOccupancyDenom.SetAxisRange(45,76,"Y");

  hOccupancyDenom.Draw("colz");
  hOccupancyDenom.Write();
  Can.SaveAs( OutDir+TString(hOccupancyDenom.GetName()) + ".gif");
  Can.SaveAs( OutDir+TString(hOccupancyDenom.GetName()) + ".pdf");


  // Draw ratio of Occupancy histograms
  hOccupancyNum.Divide( &hOccupancyDenom );
  hOccupancyNum.SetMinimum(0);
  hOccupancyNum.SetMaximum(1.2);

  hOccupancyNum.Draw("colz");
  hOccupancyNum.Write();
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

  hChi2X.Draw("hist");


  TF1 fun_chi2_3dof("chi2_3dof", "exp(-x/2.)*sqrt(x)/(5*sqrt(2*3.1415))");
  fun_chi2_3dof.SetRange(0.,20.);
  fun_chi2_3dof.SetNpx(1000);
  fun_chi2_3dof.Draw("SAME");


  hChi2X.Write();
  Can.SaveAs( OutDir+ TString(hChi2X.GetName()) +".gif");
  Can.SaveAs( OutDir+ TString(hChi2X.GetName()) +".pdf");


  hChi2Y.Scale(1/hChi2Y.Integral());
  hChi2Y.Draw();
  fun_chi2_3dof.Draw("SAME");
  hChi2Y.Write();
  Can.SaveAs( OutDir+ TString(hChi2Y.GetName()) +".gif");
  Can.SaveAs( OutDir+ TString(hChi2Y.GetName()) +".pdf");


  TH1* h01 = hCharge01.Project3D("Z");
  TH1* h02 = hCharge02.Project3D("Z");
  TH1* h03 = hCharge03.Project3D("Z");
  TH1* h04 = hCharge04.Project3D("Z");

  float hmax = 1.1 * std::max( h01->GetMaximum(),
                 std::max( h02->GetMaximum(),
                   std::max( h03->GetMaximum(), h04->GetMaximum() )));

  h01->SetAxisRange(0,hmax,"Y");
  h02->SetAxisRange(0,hmax,"Y");
  h03->SetAxisRange(0,hmax,"Y");
  h04->SetAxisRange(0,hmax,"Y");

  h01->SetLineColor(1);
  h02->SetLineColor(2);
  h03->SetLineColor(3);
  h04->SetLineColor(4);

  h01->SetLineWidth(2);
  h02->SetLineWidth(2);
  h03->SetLineWidth(2);
  h04->SetLineWidth(2);

  h01->GetXaxis()->SetTitle("Charge (Electrons)");
  h01->GetYaxis()->SetTitle("Number of Hits");

  TLegend Leg(0.5, 0.5, 0.90, 0.88, "");
  Leg.SetFillColor(0);
  Leg.SetBorderSize(0);
  Leg.SetTextSize(0.05);
  Leg.AddEntry(h01, "#Delta R < 0.01 cm", "l");
  Leg.AddEntry(h02, "#Delta R < 0.02 cm", "l");
  Leg.AddEntry(h03, "#Delta R < 0.03 cm", "l");
  Leg.AddEntry(h04, "#Delta R < 0.04 cm", "l");

  h01->Draw();
  h02->Draw("SAME");
  h03->Draw("SAME");
  h04->Draw("SAME");
  Leg.Draw();

  h01->Write();
  h02->Write();
  h03->Write();
  h04->Write();

  Can.SaveAs( OutDir+ TString(hCharge01.GetName()) +".gif");
  Can.SaveAs( OutDir+ TString(hCharge01.GetName()) +".pdf");

  float maxz;
  if (plane_under_test==1)
    maxz = 30000;
  if (plane_under_test==2)
    maxz = 30000;
  if (plane_under_test==3)
    maxz = 30000;
  if (plane_under_test==4)
    maxz = 50000;


  TProfile2D * ph01 = hCharge01.Project3DProfile("yx");
  ph01->SetAxisRange(18,34,"X");
  ph01->SetAxisRange(45,76,"Y");
  ph01->SetMinimum(0);
  ph01->SetMaximum(maxz);
  ph01->Draw("COLZ");
  ph01->Write();
  Can.SaveAs( OutDir+ TString(hCharge01.GetName()) +"_profile.gif");
  Can.SaveAs( OutDir+ TString(hCharge01.GetName()) +"_profile.pdf");

  TProfile2D * ph02 = hCharge02.Project3DProfile("yx");
  ph02->SetAxisRange(18,34,"X");
  ph02->SetAxisRange(45,76,"Y");
  ph02->SetMinimum(0);
  ph02->SetMaximum(maxz);
  ph02->Draw("COLZ");
  ph02->Write();
  Can.SaveAs( OutDir+ TString(hCharge02.GetName()) +"_profile.gif");
  Can.SaveAs( OutDir+ TString(hCharge02.GetName()) +"_profile.pdf");

  TProfile2D * ph03 = hCharge03.Project3DProfile("yx");
  ph03->SetAxisRange(18,34,"X");
  ph03->SetAxisRange(45,76,"Y");
  ph03->SetMinimum(0);
  ph03->SetMaximum(maxz);
  ph03->Draw("COLZ");
  ph03->Write();
  Can.SaveAs( OutDir+ TString(hCharge03.GetName()) +"_profile.gif");
  Can.SaveAs( OutDir+ TString(hCharge03.GetName()) +"_profile.pdf");

  TProfile2D * ph04 = hCharge04.Project3DProfile("yx");
  ph04->SetAxisRange(18,34,"X");
  ph04->SetAxisRange(45,76,"Y");
  ph04->SetMinimum(0);
  ph04->SetMaximum(maxz);
  ph04->Draw("COLZ");
  ph04->Write();
  Can.SaveAs( OutDir+ TString(hCharge04.GetName()) +"_profile.gif");
  Can.SaveAs( OutDir+ TString(hCharge04.GetName()) +"_profile.pdf");


}



void TestPlaneEfficiencySilicon (std::string const InFileName,
                                 TFile * out_f,
                                 std::string const CalibrationList,
                                 TString const RunNumber,
                                 std::vector< std::vector< std::vector<int> > > & hot_pixels)
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
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope.dat");

  // Initialize Reader
  PSIBinaryFileReader BFR(InFileName, CalibrationList);
  BFR.SetTrackingAlignment(&Alignment);

  // Mask four extra rows on each boundary of the diamond sensors
  BFR.ReadPixelMask( "outerPixelMask_Telescope2.txt");

  // Add additional hot pixels (from FindHotPixels to mask)
  for (int iroc=0; iroc != 6; iroc++){
    for (int icolrow=0; icolrow != hot_pixels[iroc].size(); icolrow++){
      BFR.AddToPixelMask( 1, iroc, hot_pixels[iroc][icolrow][0], hot_pixels[iroc][icolrow][1]);
    }
  }

  //BFR.AddToPixelMask(1,0,0,0);

  BFR.CalculateLevels(10000, OutDir);

  // numerators and denominators for efficiency calculation
  std::vector<int> nums(6);
  std::vector<int> denoms(6);
  for (int i = 0; i != 6; i++){
    nums[i]   = 0;
    denoms[i] = 0;
  }


  // Event Loop
  for (int ievent = 0; BFR.GetNextEvent() >= 0; ++ievent) {

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

}



int TestPSIBinaryFileReader (std::string const InFileName, TFile * out_f, std::string const CalibrationList, TString const RunNumber)
{
  // Run default analysis

  // Mask hot pixels in offline analysis
  // pixels are considered hot if they have > 10 times the number of mean hits of
  // other (filled) pixels in the ROC
  // Call the hot finder repeatedly until no new hot pixels can be found
  std::vector< std::vector< std::vector<int> > > hot_pixels;
  for (int iroc = 0; iroc != 6; ++iroc) {
    std::vector< std::vector<int> > tmp;
    hot_pixels.push_back( tmp );
  }

  // Look for hot pixels
  FindHotPixels(InFileName, out_f, CalibrationList, RunNumber, hot_pixels);

  TestPlaneEfficiencySilicon(InFileName, out_f, CalibrationList, RunNumber, hot_pixels);


  // Study single planes
  for (int iplane=1; iplane!=5;iplane++)
    TestPlaneEfficiency(InFileName, out_f, CalibrationList, RunNumber, hot_pixels,iplane);

  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";

  std::cout<<OutDir<<std::endl;

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

  // Mask additional outer four layer on all diamonds
  BFR.ReadPixelMask( "outerPixelMask_Telescope2.txt");

  //Add hot pixels we found to mask
  for (int iroc=0; iroc != 6; iroc++){
   for (int icolrow=0; icolrow != hot_pixels[iroc].size(); icolrow++){
     // std::cout << "Masking HOT: " << iroc << " " << hot_pixels[iroc][icolrow][0] << " " << hot_pixels[iroc][icolrow][1] << std::endl;
     BFR.AddToPixelMask( 1, iroc, hot_pixels[iroc][icolrow][0], hot_pixels[iroc][icolrow][1]);
   }
  }

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
  // Standard
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
  // For Tracks
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
  // Long Histogram (see the Protons)
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
  // For Tracks, using additional selections
  TH1F* hPulseHeightOffline[6][4];
  for (int iroc = 0; iroc != 6; ++iroc) {
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
    hPulseHeightOffline[iroc][inpix]->SetXTitle("Charge (electrons)");
    hPulseHeightOffline[iroc][inpix]->SetYTitle("Number of Clusters");
    hPulseHeightOffline[iroc][inpix]->SetLineColor(HistColors[inpix]);
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

  for (int iroc = 0; iroc != 6; ++iroc){
    hResidual.push_back( TH2F(  Form("Residual_ROC%i",iroc),
        Form("Residual_ROC%i",iroc), 100, -.15, .15, 100, -.15, .15));
    hResidualXdY.push_back( TH2F(  Form("ResidualXdY_ROC%i",iroc),
           Form("ResidualXdY_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
    hResidualYdX.push_back( TH2F(  Form("ResidualYdX_ROC%i",iroc),
           Form("ResidualYdX_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
  }

	int onepc = 0;
	int twopc = 0;
	int threepc = 0;

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
						onepc++;
            hPulseHeightLong[iplane][1]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][1], NAvgPH[iplane][1], Cluster->Charge());
          } else if (Cluster->NHits() == 2) {
            hPulseHeight[iplane][2]->Fill(Cluster->Charge());
            twopc++;
						hPulseHeightLong[iplane][2]->Fill(Cluster->Charge());
            PLTU::AddToRunningAverage(AvgPH[iplane][2], NAvgPH[iplane][2], Cluster->Charge());
          } else if (Cluster->NHits() >= 3) {
            hPulseHeight[iplane][3]->Fill(Cluster->Charge());
            threepc++;
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

          if (Track->IsFiducial(1, 5, Alignment, PLTPlane::kFiducialRegion_Diamond_m2_m2)) {
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


		double oneovertwo,oneoverthree,twooverthree;

		oneovertwo = onepc/twopc;
		oneoverthree = onepc/threepc;
		twooverthree = twopc/threepc;

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
    lPulseHeight.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeight[iroc][1]->GetMean())+" from"+TString::Format("%5.0i", onepc, "")+" clust.", "")->SetTextColor(HistColors[1]);
    lPulseHeight.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeight[iroc][2]->GetMean())+" from"+TString::Format("%5.0i", twopc, "")+" clust.", "")->SetTextColor(HistColors[2]);
    lPulseHeight.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeight[iroc][3]->GetMean())+" from"+TString::Format("%5.0i", threepc, "")+" clust.", "")->SetTextColor(HistColors[3]);
//    TLegend lRatio(0.75, 0.1, 0.90, 0.4, "Ratio:");
//    lRatio.SetTextAlign(11);
//    lRatio.SetFillStyle(0);
//    lRatio.SetBorderSize(0);
//    lRatio.AddEntry( "oneovertwo", TString::Format("%8.0f", oneovertwo, "")+" 1 over 2");
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
  hCoincidenceMap.Draw("");
  Can2.SaveAs(OutDir+"Occupancy_Coincidence.gif");

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


  WriteHTML(PlotsDir + RunNumber, CalibrationList);

  return 0;
}


int TestPSIBinaryFileReaderAlign (std::string const InFileName, TFile * out_f, std::string const CalibrationList, TString const RunNumber)
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


  for (int ialign=0; ialign!=2;ialign++){


  // Start with initial Alignment (X,Y offsets and rotations set to zero)
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope_initial.dat");

  for (int iroc=0;iroc!=6;iroc++){
    Alignment.AddToLX( 1, iroc, x_align[iroc] );
    Alignment.AddToLY( 1, iroc, y_align[iroc] );
    //Alignment.AddToLR( 1, iroc, r_align[iroc] );
  }


  for (int iroc_align = 0; iroc_align != 6; ++iroc_align) {

    std::cout << "GOING TO ALIGN: " << iroc_align << std::endl;

    float best_RMS = 99999;

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

    PSIBinaryFileReader BFR(InFileName, CalibrationList);
    BFR.SetTrackingAlignment(&Alignment);
    FILE* f = fopen("MyGainCal.dat", "w");
    BFR.GetGainCal()->PrintGainCal(f);
    fclose(f);
    BFR.ReadPixelMask( "outerPixelMask_Telescope2.txt");
    BFR.CalculateLevels(10000 ,OutDir);
    BFR.SetPlaneUnderTest( iroc_align );


    // Reset residual histograms
    hResidual.clear();
    hResidualXdY.clear();
    hResidualYdX.clear();
    for (int iroc = 0; iroc != 6; ++iroc){
      hResidual.push_back( TH2F(  Form("Residual_ROC%i",iroc),
                                  Form("Residual_ROC%i",iroc), 400, -.8, .8, 400, -.8, .8));
      hResidualXdY.push_back( TH2F(  Form("ResidualXdY_ROC%i",iroc),
                                     Form("ResidualXdY_ROC%i",iroc), 133, -1, 0.995, 100, -.5, .5));
      hResidualYdX.push_back( TH2F(  Form("ResidualYdX_ROC%i",iroc),
                                     Form("ResidualYdX_ROC%i",iroc), 201, -1, 1, 100, -.5, .5));
    }

    sumResSquareCurrent = 0;


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

      float track_LX = Alignment.TtoLX( track_TX, track_TY, 1, iroc_align);
      float track_LY = Alignment.TtoLY( track_TX, track_TY, 1, iroc_align);

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




  std::cout << "Before: " << Alignment.LX(1,iroc_align) << std::endl;

  float angle = atan(hResidualXdY[iroc_align].GetCorrelationFactor()) ;

  x_align[iroc_align] +=  hResidual[iroc_align].GetMean(1);
  y_align[iroc_align] +=  hResidual[iroc_align].GetMean(2);
  r_align[iroc_align] +=  hResidualXdY[iroc_align].GetCorrelationFactor();


  std::cout << "After: " << Alignment.LX(1,iroc_align) << std::endl;


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



  float new_RMS = sqrt(hResidual[iroc_align].GetRMS(1)*hResidual[iroc_align].GetRMS(1)+
                       hResidual[iroc_align].GetRMS(2)*hResidual[iroc_align].GetRMS(2));

  //std::cout << "New RMS: " << iroc_align << " " << lz << " " << new_RMS << std::endl;




  for (int i=0; i!=6;i++){

    std::cout << i << " " << x_align[i] << " " << y_align[i] << " " << z_align[i] << " " << r_align[i] <<std::endl;
  }

} // end loop over rocs
Alignment.WriteAlignmentFile("NewAlignment.dat");

} // end alignment loop




std::cout << "PART TWO!!!!!" << std::endl;



// Start with initial Alignment (X,Y offsets and rotations set to zero)
PLTAlignment Alignment;
Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_ETHTelescope_initial.dat");

for (int iroc=0;iroc!=6;iroc++){
  Alignment.AddToLX( 1, iroc, x_align[iroc] );
  Alignment.AddToLY( 1, iroc, y_align[iroc] );
  //Alignment.AddToLR( 1, iroc, r_align[iroc] );
}


for (int ialign=0; ialign!=4;ialign++){


  float best_RMS = 99999;

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

  PSIBinaryFileReader BFR(InFileName, CalibrationList);
  BFR.SetTrackingAlignment(&Alignment);
  FILE* f = fopen("MyGainCal.dat", "w");
  BFR.GetGainCal()->PrintGainCal(f);
  fclose(f);
  BFR.ReadPixelMask( "outerPixelMask_Telescope2.txt");
  BFR.CalculateLevels(10000 ,OutDir);


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

  sumResSquareCurrent = 0;


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

      // dX vs dY
      hResidual[iroc].Fill( d_LX, d_LY);

      // X vs dY
      hResidualXdY[iroc].Fill( cl_LX, d_LY);

      // Y vs dX
      hResidualYdX[iroc].Fill( cl_LY, d_LX);


    }


  } // end event loop

  for (int iroc=0; iroc!=6; iroc++){
  std::cout << "RESIDUALS: " << hResidual[iroc].GetMean(1) << " " << hResidual[iroc].GetMean(2) << std::endl;
  std::cout << "RESIDUALS RMS: " << hResidual[iroc].GetRMS(1) << " " << hResidual[iroc].GetRMS(2) <<std::endl;

  Alignment.AddToLX(1, iroc, hResidual[iroc].GetMean(1));
  Alignment.AddToLY(1, iroc, hResidual[iroc].GetMean(2));

  float angle = atan(hResidualXdY[iroc].GetCorrelationFactor()) ;
  Alignment.AddToLR(1, iroc, angle/10.);


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

  }



  } // end alignment loop

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

  // OFFLINE
  f << "<h2>Straight Tracks</h2>\n";

  f << "<br>\n";
  for (int i = 0; i != 6; ++i) {
    f << Form("<a href=\"PulseHeightOffline_ROC%i.gif\"><img width=\"150\" src=\"PulseHeightOffline_ROC%i.gif\"></a>\n", i, i);
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
    f << Form("<a href=\"Charge_ROC%i.gif\"><img width=\"150\" src=\"Charge_ROC%i.gif\"></a>\n", i, i);
  f << "<br>\n";


  for (int i = 1; i != 5; i++)
    f << Form("<a href=\"Charge_ROC%i_profile.gif\"><img width=\"150\" src=\"Charge_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
   f << Form("<a href=\"Charge02_ROC%i_profile.gif\"><img width=\"150\" src=\"Charge02_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
   f << Form("<a href=\"Charge03_ROC%i_profile.gif\"><img width=\"150\" src=\"Charge03_ROC%i_profile.gif\"></a>\n", i, i);
  f << "<br>\n";

  for (int i = 1; i != 5; i++)
  f << Form("<a href=\"Charge04_ROC%i_profile.gif\"><img width=\"150\" src=\"Charge04_ROC%i_profile.gif\"></a>\n", i, i);
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
    for (int icol = 1; icol != 6; ++icol) {
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


  // Open a ROOT file to store histograms in
  // do it here and pass to all functions we call
  TString const PlotsDir = "plots/";
  TString const OutDir = PlotsDir + RunNumber + "/";
  TFile out_f( OutDir + "histos.root","recreate");

  int doAlign = atoi(argv[3]);

  if (doAlign)
    TestPSIBinaryFileReaderAlign(InFileName, &out_f, CalibrationList, RunNumber);
  else
    TestPSIBinaryFileReader(InFileName, &out_f, CalibrationList, RunNumber);


  return 0;
}
