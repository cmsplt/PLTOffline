////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri May  6 09:32:06 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include "TString.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFitResult.h"
#include "TCanvas.h"
#include "TKey.h"
#include "TClass.h"
#include "TDirectory.h"
#include "TAxis.h"
#include "TMath.h"

#include "PLTU_karen.h"
#include "PLTU.h"


TString Pack (int const mFec, int const mFecChannel, int const hubAddress, int const roc, int const col, int const row)
{
  // Just pack this into something easy to decode
  TString ret;
  char BUFF[10];
  sprintf(BUFF, "%1i%1i%02i%1i%2i%2i", mFec, mFecChannel, hubAddress, roc, col, row);
  ret = BUFF;
  return ret;
}

void UnPack (const char* in, int& mFec, int& mFecChannel, int& hubAddress, int& roc, int& col, int& row)
{
  // unpack the easy to decode string
  sscanf(in, "%1i%1i%2i%1i%2i%2i", &mFec, &mFecChannel, &hubAddress, &roc, &col, &row);
  return;
}


int GainCalFastFits (TString const InFileName)
{
  // Open the input file
  std::ifstream f(InFileName.Data());
  if (!f) {
    std::cerr << "ERROR; Cannot open file: " << InFileName << std::endl;
    throw;
  }


  // Set some basic style
  PLTU_karen::SetStyle();


  // Open the output root file
  TString const OutRootName = "GainCalFits.root";
  TFile fOutRoot(OutRootName, "recreate");
  if (!fOutRoot.IsOpen()) {
    std::cerr << "ERROR: Cannot open output root file: " << OutRootName << std::endl;
    throw;
  }

  // Make a directory for good and bad fits
  //  TDirectory* dGoodFits = fOutRoot.mkdir("Fits_Good");
  //  TDirectory* dBadFits = fOutRoot.mkdir("Fits_Bad");

  // Open root file outout
  TString const OutFitsName = "GainCalFits.dat";
  FILE* fOutFits = fopen(OutFitsName.Data(), "w");
  if (!fOutFits) {
    std::cerr << "ERROR: cannot open out data file: " << OutFitsName << std::endl;
    throw;
  }

  // Map of all all pixels
  std::map<TString, std::vector< std::pair<float, float> > > Map;

  // stringstream to be used for each line of input data file
  std::stringstream s;

  // veriales we care about
  int mFec, mFecChannel, hubAddress, roc, channel, col, row;
  float adc, vcal;

  // Keep track of which ROCs and what VCals we use
  char ROCId[10];
  std::set<TString> ROCNames;
  std::set<int>     VCals;


  // Loop over header linesin the input data file
  for (std::string line; std::getline(f, line); ) {
    fprintf(fOutFits, "%s\n", line.c_str());
    if (line == "\n") {
      break;
    }
  }

  // Loop over all lines in the input data file
  for (std::string line; std::getline(f, line); ) {
    s.clear();
    s.str(line);
    s >> mFec
      >> mFecChannel
      >> hubAddress
      >> col
      >> row
      >> roc
      >> adc
      >> vcal;


    // Get a simple string for the pixel and pair adc and vcal for this hit
    // which gets added to the map
    TString Id = Pack(mFec, mFecChannel, hubAddress, roc, col, row);
    Map[Id].push_back( std::make_pair(adc, vcal) );

    // Add ROC and VCal if not there already
    ROCNames.insert(TString::Format("%1i%1i%02i%1i", mFec, mFecChannel, hubAddress, roc));
    VCals.insert(vcal);
  }

  // Define the function we will fit for each pixel
  TF1 FitFunc("FitFunc", "[0]*x*x + [1]*x + [2] + TMath::Exp( (x-[3]) / [4]  )", 150, 400);

  // Define Chi2 plot for all pixels
  TH1F FitChi2("FitChi2", "FitChi2", 200, 0, 5000);

  // try to remember what the hell you were doing here dean.
  std::map< std::pair<TString, int>, TH2F*> RocVCalOccupancy;
  for (std::set<TString>::iterator ir = ROCNames.begin(); ir != ROCNames.end(); ++ir) {
    for (std::set<int>::iterator iv = VCals.begin(); iv != VCals.end(); ++iv) {
      RocVCalOccupancy[ std::make_pair<TString, int>(*ir, *iv) ] = new TH2F(TString::Format("%s_%04i", ir->Data(), *iv), TString::Format("%s_%04i", ir->Data(), *iv), 100, 0, 100, 100, 0, 100);
    }
  }

  // These are the 5 parameters from the fit we care about
  float Param[5];

  // Min and max for each TGraph below
  double adcMin, vcalMin, adcMax, vcalMax;

  std::map<TString, std::vector<float> > ROCSaturationValues;
  std::map<TString, std::vector<float> > ROCChi2;
  std::map<TString, std::vector<float> > ROCParam0;
  std::map<TString, std::vector<float> > ROCParam1;
  std::map<TString, std::vector<float> > ROCParam2;
  std::map<TString, std::vector<float> > ROCParam3;
  std::map<TString, std::vector<float> > ROCParam4;
  std::map<TString, std::vector<float> > ROCSaturationValues_bad;
  std::map<TString, std::vector<float> > ROCChi2_bad;
  std::map<TString, std::vector<float> > ROCParam0_bad;
  std::map<TString, std::vector<float> > ROCParam1_bad;
  std::map<TString, std::vector<float> > ROCParam2_bad;
  std::map<TString, std::vector<float> > ROCParam3_bad;
  std::map<TString, std::vector<float> > ROCParam4_bad;
  std::map<TString, TH2F*> hGoodFitMap;
  std::map<TString, TH2F*> hBadFitMap;
  std::map<TString, TH2F*> hAllFitMap;
  TDirectory* dGoodFits;
  TDirectory* dBadFits;

  // Loop over all entries in the map (which is by definiteion organized by pixel already, how nice
  for (std::map<TString, std::vector< std::pair<float, float> > >::iterator It = Map.begin(); It != Map.end(); ++It) {

    // Which pixel is this anyway.. get the info
    UnPack(It->first.Data(), mFec, mFecChannel, hubAddress, roc, col, row);

    // This is ROC
    sprintf(ROCId, "%1i%1i%02i%1i", mFec, mFecChannel, hubAddress, roc);
    //    printf("ROCId: %1i %1i %02i %1i   %s %s\n", mFec, mFecChannel, hubAddress, roc, ROCId, It->first.Data());

    //    Make a directory for good and bad fits

    TString gooddirname, baddirname;
    gooddirname.Form("Fits_Good_mFec%i_mFecChannel%i_hubAddress%02i_ROC%i", mFec, mFecChannel, hubAddress, roc);
    baddirname.Form("Fits_Bad_mFec%i_mFecChannel%i_hubAddress%02i_ROC%i", mFec, mFecChannel, hubAddress, roc);

    if (!fOutRoot.GetKey(gooddirname)){
      dGoodFits = fOutRoot.mkdir(gooddirname);
      dBadFits = fOutRoot.mkdir(baddirname);
    }

    // Hist for good bad and all fits
    //kga switched from GoodfitMap to AllFitMap...think that's right...
    if (!hAllFitMap.count(ROCId)) {
      TString MyName;
      MyName.Form("GoodFitMap_mFec%i_mFecChannel%i_hubAddress%02i_ROC%i", mFec, mFecChannel, hubAddress, roc);
      hGoodFitMap[ROCId] = new TH2F(MyName, MyName, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
      MyName.Form("BadFitMap_mFec%i_mFecChannel%i_hubAddress%02i_ROC%i", mFec, mFecChannel, hubAddress, roc);
      hBadFitMap[ROCId] = new TH2F(MyName, MyName, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
      MyName.Form("AllFitMap_mFec%i_mFecChannel%i_hubAddress%02i_ROC%i", mFec, mFecChannel, hubAddress, roc);
      hAllFitMap[ROCId] = new TH2F(MyName, MyName, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
    }

    // Input for TGraph, define size, and fill them
    float X[It->second.size()];
    float Y[It->second.size()];
    for (size_t i = 0; i != It->second.size(); ++i) {
      X[i] = It->second[i].first;
      Y[i] = It->second[i].second;
      RocVCalOccupancy[ std::make_pair<TString, int>(TString(ROCId), (int) Y[i]) ]->Fill(col, row);
    }

    // Actually make a TGraph
    TGraphErrors g(It->second.size(), X, Y);
    TString const Name = TString::Format("Fit_mF%1i_mFC%1i_hub%2i_ROC%1i_Col%2i_Row%2i", mFec, mFecChannel, hubAddress, roc, col, row);
    g.SetName(Name);
    g.SetTitle(Name);
    g.GetYaxis()->SetTitle("VCal");
    g.GetXaxis()->SetTitle("ADC");
   


    // Get the min and max point from the graph
    g.GetPoint(0, adcMin, vcalMin);
    g.GetPoint(g.GetN()-1, adcMax, vcalMax);

    // Set the range of the fit
    FitFunc.SetRange(adcMin, adcMax);

    // Some default parameters to start off with
    FitFunc.SetParameter(0, 0.1);
    FitFunc.SetParameter(1, -30);
    FitFunc.SetParameter(2, 2000);
    FitFunc.SetParameter(3, adcMax);
    FitFunc.SetParameter(4, 60);

    // Do the fit
    int FitResult = g.Fit("FitFunc", "Q","",adcMin,adcMax);

    // Grab the parameters from the fit
    Param[0] = FitFunc.GetParameter(0);
    Param[1] = FitFunc.GetParameter(1);
    Param[2] = FitFunc.GetParameter(2);
    Param[3] = FitFunc.GetParameter(3);
    Param[4] = FitFunc.GetParameter(4);

    if (FitResult == 0) {
      hGoodFitMap[ROCId]->Fill(col, row);
      dGoodFits->cd();
      g.Write();
      
      ROCParam0[ROCId].push_back(Param[0]);
      ROCParam1[ROCId].push_back(Param[1]);
      ROCParam2[ROCId].push_back(Param[2]);
      ROCParam3[ROCId].push_back(Param[3]);
      ROCParam4[ROCId].push_back(Param[4]);
      ROCSaturationValues[ROCId].push_back(Param[3]);
      ROCChi2[ROCId].push_back(FitFunc.GetChisquare()/FitFunc.GetNDF());
    } //end FitResult == 0

    else {
      hBadFitMap[ROCId]->Fill(col, row);
      dBadFits->cd();
      g.Write();
      printf("FitResult = %4i for %1i %1i %1i %2i %2i %2i\n", FitResult, mFec, mFecChannel, roc, hubAddress, col, row);

      ROCParam0_bad[ROCId].push_back(Param[0]);
      ROCParam1_bad[ROCId].push_back(Param[1]);
      ROCParam2_bad[ROCId].push_back(Param[2]);
      ROCParam3_bad[ROCId].push_back(Param[3]);
      ROCParam4_bad[ROCId].push_back(Param[4]);
      ROCSaturationValues_bad[ROCId].push_back(Param[3]);
      ROCChi2_bad[ROCId].push_back(FitFunc.GetChisquare()/FitFunc.GetNDF());
    } // end FitResult != 0

    hAllFitMap[ROCId]->Fill(col, row);

    // Polt the Chi2
    FitChi2.Fill(FitFunc.GetChisquare());

    //printf("%f   %f   %f   %f   %f\n", Param[0], Param[1],Param[2],Param[3],Param[4]);

    // Save the graph to output file
    fOutRoot.cd();

    // Print the fit parameters to the output params file
    fprintf(fOutFits, "%1i %1i %2i %1i %2i %2i %12E %12E %12E %12E %12E\n", 
	    mFec, mFecChannel, hubAddress, roc, col, row, Param[0], Param[1], Param[2], Param[3], Param[4]);

  }

  fOutRoot.cd();
  for (std::map<TString, TH2F*>::iterator It = hGoodFitMap.begin(); It != hGoodFitMap.end(); ++It) {
    It->second->Write();
    TCanvas Can;
    Can.cd();
    It->second->Draw("colz");
    Can.SaveAs(TString("plots/") + It->second->GetName() + ".gif");
  }
  for (std::map<TString, TH2F*>::iterator It = hBadFitMap.begin(); It != hBadFitMap.end(); ++It) {
    It->second->Write();
    TCanvas Can;
    Can.cd();
    It->second->Draw("colz");
    Can.SaveAs(TString("plots/") + It->second->GetName() + ".gif");
  }
  for (std::map<TString, TH2F*>::iterator It = hAllFitMap.begin(); It != hAllFitMap.end(); ++It) {
    It->second->Write();
    TCanvas Can;
    Can.cd();
    It->second->Draw("colz");
    Can.SaveAs(TString("plots/") + It->second->GetName() + ".gif");
  }

  // Plot the Good values
  for (std::map<TString, std::vector<float> >::iterator It = ROCChi2.begin(); It != ROCChi2.end(); ++It) {
    UnPack(It->first.Data(), mFec, mFecChannel, hubAddress, roc, col, row);

    //Fit Chi2/NDF
    TString Name = TString::Format("Chi2_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, roc);
    fOutRoot.cd();
    TH1F hC(Name, Name, 100, 0, 1500);
    hC.GetXaxis()->SetTitle("Chi2/NDF");
    hC.SetNdivisions(5);
    for (size_t i = 0; i !=  ROCChi2[It->first].size(); ++i) {
      hC.Fill(ROCChi2[It->first][i]);
    }
    hC.Write();

    TString Namebad = TString::Format("Chi2_mF%1i_mFC%1i_hub%02i_ROC%1i_bad", mFec, mFecChannel, hubAddress, roc);
    TH1F hCb(Namebad, Namebad, 100, 0, 1500);
    hCb.GetXaxis()->SetTitle("Chi2/NDF");
    hCb.SetNdivisions(5);
    for (size_t i = 0; i != ROCChi2_bad[It->first].size(); ++i) {
      hCb.Fill(ROCChi2_bad[It->first][i]);
    }
    hCb.Write();


    //Fit Saturation
    Name = TString::Format("Saturation_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, roc);
    TH1F hS(Name, Name, 100, 200, 300);
    hS.GetXaxis()->SetTitle("Fit Saturation Values");
    hS.SetNdivisions(5);
    for (size_t i = 0; i != ROCSaturationValues[It->first].size(); ++i) {
      hS.Fill(ROCSaturationValues[It->first][i]);
    }
    hS.Write();

    Namebad = TString::Format("Saturation_mF%1i_mFC%1i_hub%02i_ROC%1i_bad", mFec, mFecChannel, hubAddress, roc);
    TH1F hSb(Namebad, Namebad, 100, 200, 300);
    hSb.GetXaxis()->SetTitle("Fit Saturation Values");
    hSb.SetNdivisions(5);
    for (size_t i = 0; i != ROCSaturationValues_bad[It->first].size(); ++i) {
      hSb.Fill(ROCSaturationValues_bad[It->first][i]);
    }
    hSb.Write();


    //Fit Parameter 0
    Name = TString::Format("Param0_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, roc);
    TH1F h0(Name, Name, 100, -1, 1);
    h0.GetXaxis()->SetTitle("Fit Parameter 0");
    h0.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam0[It->first].size(); ++i) {
      h0.Fill(ROCParam0[It->first][i]);
    }
    h0.Write();

    Namebad = TString::Format("Param0_mF%1i_mFC%1i_hub%02i_ROC%1i_bad", mFec, mFecChannel, hubAddress, roc);
    TH1F h0b(Namebad, Namebad, 100, -1, 1);
    h0b.GetXaxis()->SetTitle("Fit Parameter 0");
    h0b.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam0_bad[It->first].size(); ++i) {
      h0b.Fill(ROCParam0_bad[It->first][i]);
    }
    h0b.Write();

    //Fit Parameter 1
    Name = TString::Format("Param1_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, roc);
    TH1F h1(Name, Name, 100, -150, 150);
    h1.GetXaxis()->SetTitle("Fit Parameter 1");
    h1.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam1[It->first].size(); ++i) {
      h1.Fill(ROCParam1[It->first][i]);
    }
    h1.Write();
    
    Namebad = TString::Format("Param1_mF%1i_mFC%1i_hub%02i_ROC%1i_bad", mFec, mFecChannel, hubAddress, roc);
    TH1F h1b(Namebad, Namebad, 100, -150, 150);
    h1b.GetXaxis()->SetTitle("Fit Parameter 1");
    h1b.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam1_bad[It->first].size(); ++i) {
      h1b.Fill(ROCParam1_bad[It->first][i]);
    }
    h1b.Write();


    //Fit Parameter 2
    Name = TString::Format("Param2_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, roc);
    TH1F h2(Name, Name, 100, -10000, 10000);
    h2.SetNdivisions(5);
    h2.GetXaxis()->SetTitle("Fit Parameter 2");
    for (size_t i = 0; i != ROCParam2[It->first].size(); ++i) {
      h2.Fill(ROCParam2[It->first][i]);
    }
    h2.Write();

    Namebad = TString::Format("Param2_mF%1i_mFC%1i_hub%02i_ROC%1i_bad", mFec, mFecChannel, hubAddress, roc);
    TH1F h2b(Namebad, Namebad, 100, -10000, 10000);
    h2b.SetNdivisions(5);
    h2b.GetXaxis()->SetTitle("Fit Parameter 2");
    for (size_t i = 0; i != ROCParam2_bad[It->first].size(); ++i) {
      h2b.Fill(ROCParam2_bad[It->first][i]);
    }
    h2b.Write();


    //Fit Parameter 3
    Name = TString::Format("Param3_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, roc);
    TH1F h3(Name, Name, 100, -400, 225);//100, 225);
    h3.GetXaxis()->SetTitle("Fit Parameter 3");
    h3.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam3[It->first].size(); ++i) {
      h3.Fill(ROCParam3[It->first][i]);
    }
    h3.Write();

    Namebad = TString::Format("Param3_mF%1i_mFC%1i_hub%02i_ROC%1i_bad", mFec, mFecChannel, hubAddress, roc);
    TH1F h3b(Namebad, Namebad, 100, -600, 225);//100, 225);
    h3b.GetXaxis()->SetTitle("Fit Parameter 3");
    h3b.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam3_bad[It->first].size(); ++i) {
      h3b.Fill(ROCParam3_bad[It->first][i]);
    }
    h3b.Write();

    //Fit Parameter 4
    Name = TString::Format("Param4_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, roc);
    TH1F h4(Name, Name, 100, -10,200);//-2,20);
    h4.GetXaxis()->SetTitle("Fit Parameter 4");
    h4.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam4[It->first].size(); ++i) {
      h4.Fill(ROCParam4[It->first][i]);
    }
    h4.Write();

    Namebad = TString::Format("Param4_mF%1i_mFC%1i_hub%02i_ROC%1i_bad", mFec, mFecChannel, hubAddress, roc);
    TH1F h4b(Namebad, Namebad, 100,-10,200); //-2,20);
    h4b.GetXaxis()->SetTitle("Fit Parameter 4");
    h4b.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam4_bad[It->first].size(); ++i) {
      h4b.Fill(ROCParam4_bad[It->first][i]);
    }
    h4b.Write();


    //Make a canvas for each roc with all fit params and chi2/ndf 
    TCanvas can;
    can.SetWindowSize(990,660);
    can.SetCanvasSize(990,660);
    can.Divide(3,2);

    can.cd(1);
    hCb.SetLineColor(kRed);
    hC.GetMaximum() > hCb.GetMaximum() ? hCb.SetMaximum(hC.GetMaximum()+2) : hC.SetMaximum(hCb.GetMaximum()+2);
    hC.Draw();
    hCb.Draw("sames");

    can.cd(2);
    h0b.SetLineColor(kRed);
    h0.GetMaximum() > h0b.GetMaximum() ? h0b.SetMaximum(h0.GetMaximum()+2) : h0.SetMaximum(h0b.GetMaximum()+2);
    h0.Draw();
    h0b.Draw("sames");

    can.cd(3);
    h1b.SetLineColor(kRed);
    h1.GetMaximum() > h1b.GetMaximum() ? h1b.SetMaximum(h1.GetMaximum()+2) : h1.SetMaximum(h1b.GetMaximum()+2);
    h1.Draw();
    h1b.Draw("sames");

    can.cd(4);
    h2b.SetLineColor(kRed);
    h2.GetMaximum() > h2b.GetMaximum() ? h2b.SetMaximum(h2.GetMaximum()+2) : h2.SetMaximum(h2b.GetMaximum()+2);
    h2.Draw();
    h2b.Draw("sames");

    can.cd(5);
    h3b.SetLineColor(kRed);
    h3.GetMaximum() > h3b.GetMaximum() ? h3b.SetMaximum(h3.GetMaximum()+2) : h3.SetMaximum(h3b.GetMaximum()+2);
    h3.Draw();
    h3b.Draw("sames");

    can.cd(6);
    h4b.SetLineColor(kRed);
    h4.GetMaximum() > h4b.GetMaximum() ? h4b.SetMaximum(h4.GetMaximum()+2) : h4.SetMaximum(h4b.GetMaximum()+2);
    h4.Draw();
    h4b.Draw("sames");

    can.Print(Form("plots/GainCalFitParams_mF%1i_mFC%1i_hub%02i_ROC%1i.png", mFec, mFecChannel, hubAddress, roc));
    can.Print(Form("plots/GainCalFitParams_mF%1i_mFC%1i_hub%02i_ROC%1i.pdf", mFec, mFecChannel, hubAddress, roc));

    TString gooddirname, baddirname;
    gooddirname.Form("Fits_Good_mFec%i_mFecChannel%i_hubAddress%02i_ROC%i", mFec, mFecChannel, hubAddress, roc);
    baddirname.Form("Fits_Bad_mFec%i_mFecChannel%i_hubAddress%02i_ROC%i", mFec, mFecChannel, hubAddress, roc);
    dGoodFits=fOutRoot.GetDirectory(gooddirname);
    dBadFits=fOutRoot.GetDirectory(baddirname);


    TCanvas can_good;
    can_good.SetWindowSize(990,990);
    can_good.SetCanvasSize(990,990);
    can_good.Divide(4,3);

    TCanvas can_bad;
    can_bad.SetWindowSize(990,990);
    can_bad.SetCanvasSize(990,990);
    can_bad.Divide(4,3);
    
    //  //make empty histo to define plot ranges.
    //  histo = new TH2D("histo","",1000,150,255,1000,0,2000);
    //  histo->SetStats(false);
    
    TKey *key;
    TObjArray histograms_good;
    TObjArray histograms_bad;
    char name[100000];
    
    int last_niter = -1;
    int niter = 0;
    TIter next_good(dGoodFits->GetListOfKeys());
    
    while ((key = (TKey*)next_good()) && niter < (dGoodFits->GetNkeys())) {
      niter++;
      if ((niter/(dGoodFits->GetNkeys() / 12)) == last_niter) continue;
      last_niter = (niter/(dGoodFits->GetNkeys() / 12));
      //cout << "last_niter = "<<last_niter<< endl;
      TClass *cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TGraphErrors")) continue;
      
      TGraphErrors *g = (TGraphErrors*)key->ReadObj();
      g->SetMarkerStyle(20);
      g->SetMarkerColor(kRed);
      //      g->GetYaxis()->SetTitle("VCal");
      //      g->GetXaxis()->SetTitle("ADC");

      sprintf(name,"goodhist_%d",last_niter);
      double xmax = TMath::MaxElement(g->GetN(),g->GetX()) + 5.;
      double xmin = TMath::MinElement(g->GetN(),g->GetX()) - 5.;
      double ymax = TMath::MaxElement(g->GetN(),g->GetY()) + 100.;
      double ymin = TMath::MinElement(g->GetN(),g->GetY()) - 100.;
      
      //      histograms_good.Add(new TH2F(name,"",1000,100,300,1000,0,1500));
      histograms_good.Add(new TH2F(name,"",1000,xmin,xmax,1000,ymin,ymax));
      TString title = g->GetTitle();
      ((TH2F*)(histograms_good[last_niter]))->SetTitle(title);
      ((TH2F*)(histograms_good[last_niter]))->GetYaxis()->SetTitle("VCal");
      ((TH2F*)(histograms_good[last_niter]))->GetXaxis()->SetTitle("ADC");
      can_good.cd(last_niter+1);
      ((TH2F*)(histograms_good[last_niter]))->Draw();
      g->Draw("psame");
      niter++;
    }
    
    niter = 0;
    last_niter = -1;
    TIter next_bad(dBadFits->GetListOfKeys());
    
    while ((key = (TKey*)next_bad()) && niter < (dBadFits->GetNkeys())) {
      niter++;
      if ((niter/(dBadFits->GetNkeys() / 12)) == last_niter) continue;
      last_niter = (niter/(dBadFits->GetNkeys() / 12));
      //cout << "last_niter = "<<last_niter<< endl;
      TClass *cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TGraphErrors")) continue;
      
      TGraphErrors *g = (TGraphErrors*)key->ReadObj();
      g->SetMarkerStyle(20);
      g->SetMarkerColor(kRed);
      g->GetYaxis()->SetTitle("VCal");
      g->GetXaxis()->SetTitle("ADC");

      sprintf(name,"badhist_%d",last_niter);

      double xmax = TMath::MaxElement(g->GetN(),g->GetX()) + 5;
      double xmin = TMath::MinElement(g->GetN(),g->GetX()) - 5;
      double ymax = TMath::MaxElement(g->GetN(),g->GetY()) + 100;
      double ymin = TMath::MinElement(g->GetN(),g->GetY()) - 100;

      histograms_bad.Add(new TH2F(name,"",1000,xmin,xmax,1000,ymin,ymax));
      
      TString title = g->GetTitle();
      ((TH2F*)(histograms_bad[last_niter]))->SetTitle(title);
      ((TH2F*)(histograms_bad[last_niter]))->GetYaxis()->SetTitle("VCal");
      ((TH2F*)(histograms_bad[last_niter]))->GetXaxis()->SetTitle("ADC");
      
      can_bad.cd(last_niter+1);
      ((TH2F*)(histograms_bad[last_niter]))->Draw();
      g->Draw("psame");
      niter++;
    }
    
    can_good.Print(Form("plots/GainCalFits_Good_mF%1i_mFC%1i_hub%02i_ROC%1i.png", mFec, mFecChannel, hubAddress, roc));
    can_good.Print(Form("plots/GainCalFits_Good_mF%1i_mFC%1i_hub%02i_ROC%1i.pdf", mFec, mFecChannel, hubAddress, roc));
    can_bad.Print(Form("plots/GainCalFits_Bad_mF%1i_mFC%1i_hub%02i_ROC%1i.png", mFec, mFecChannel, hubAddress, roc));
    can_bad.Print(Form("plots/GainCalFits_Bad_mF%1i_mFC%1i_hub%02i_ROC%1i.pdf", mFec, mFecChannel, hubAddress, roc));

    //clean up    
    for (int i = 0; i <= 12; i++){
      gDirectory->Delete(Form("badhist_%d",i));
      gDirectory->Delete(Form("goodhist_%d",i));
    }
 
 }



  // Write Chi2 plot
  fOutRoot.cd();
  FitChi2.Write();

  // Close output files
  fOutRoot.Close();
  fclose(fOutFits);

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  TString const InFileName = argv[1];
  GainCalFastFits(InFileName);

  




  return 0;
}
