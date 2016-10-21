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

// What image type do you want?
TString const SUFFIX = ".gif";

int Pack (int const channel, int const roc, int const col, int const row)
{
  // Just pack this into something easy to decode
  return 100000*channel + 10000*roc + 100*col + row;
}

void UnPack (int const in, int& channel, int& roc, int& col, int& row)
{
  // unpack the easy to decode string
  channel = in / 100000;
  roc = (in / 10000) % 10;
  col = (in / 100) % 100;
  row = in % 100;

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

  std::cout<<"InFileName: " << InFileName <<std::endl;

  // Set some basic style
  PLTU_karen::SetStyle();

  // Assumes InFileName is in dir1/dir2/...Slink_day.time.dat format
  
  TObjArray *fname = InFileName.Tokenize("_");
  TString str = ((TObjString *)fname->At(1))->GetString();
  TObjArray *sstr = str.Tokenize(".");
  TString yymmdd = ((TObjString *)sstr->At(0))->GetString();
  TString hhmmss = ((TObjString *)sstr->At(1))->GetString();
  
  // Open the output root file
  TString const OutRootName = "GainCalFits_" + yymmdd + "." + hhmmss + ".root";
  std::cout<<"RootFile: " << OutRootName <<std::endl;
  TFile fOutRoot(OutRootName, "recreate");
  if (!fOutRoot.IsOpen()) {
    std::cerr << "ERROR: Cannot open output root file: " << OutRootName << std::endl;
    throw;
  }

  // Open root file outout
  TString const OutFitsName = "GainCalFits_" + yymmdd + "." + hhmmss + ".dat";
  std::cout<<"Gaincal: " << OutFitsName <<std::endl;
  FILE* fOutFits = fopen(OutFitsName.Data(), "w");
  if (!fOutFits) {
    std::cerr << "ERROR: cannot open out data file: " << OutFitsName << std::endl;
    throw;
  }

  // Map of all all pixels
  std::map<int, std::vector< std::pair<float, float> > > Map;

  // stringstream to be used for each line of input data file
  std::stringstream s;

  // veriales we care about
  int channel, roc, col, row;
  float adc, vcal;

  // Keep track of which ROCs and what VCals we use
  std::set<int> ROCNames;
  std::set<int>     VCals;


  // Loop over header linesin the input data file
  for (std::string line; std::getline(f, line); ) {
    fprintf(fOutFits, "%s\n", line.c_str());
    if (line == "") {
      break;
    }
  }

  // Loop over all lines in the input data file
  for (std::string line; std::getline(f, line); ) {
    s.clear();
    s.str(line);
    s >> channel
      >> col
      >> row
      >> roc
      >> adc
      >> vcal;

    if (channel==2&&roc==0&&col==3&&row==12){std::cout<<"ADC: "<<adc<<"VCAL " <<vcal<<std::endl;}
    // Get a simple string for the pixel and pair adc and vcal for this hit
    // which gets added to the map
    int Id = Pack(channel, roc, col, row);
    Map[Id].push_back( std::make_pair(adc, vcal) );

    // Add ROC and VCal if not there already
    ROCNames.insert(10 * channel + roc);
    VCals.insert(vcal);
  }

  // Define the function we will fit for each pixel
  TF1 FitFunc("FitFunc", "[0]*x*x + [1]*x + [2] + TMath::Exp( (x-[3]) / [4]  )", 150, 400);

  // Define Chi2 plot for all pixels
  TH1F FitChi2("FitChi2", "FitChi2", 200, 0, 5000);


  // These are the 5 parameters from the fit we care about
  float Param[5];

  // Min and max for each TGraph below
  double adcMin, vcalMin, adcMax, vcalMax;

  std::map<int, std::vector<float> > ROCSaturationValues;
  std::map<int, std::vector<float> > ROCChi2;
  std::map<int, std::vector<float> > ROCParam0;
  std::map<int, std::vector<float> > ROCParam1;
  std::map<int, std::vector<float> > ROCParam2;
  std::map<int, std::vector<float> > ROCParam3;
  std::map<int, std::vector<float> > ROCParam4;
  std::map<int, std::vector<float> > ROCSaturationValues_bad;
  std::map<int, std::vector<float> > ROCChi2_bad;
  std::map<int, std::vector<float> > ROCParam0_bad;
  std::map<int, std::vector<float> > ROCParam1_bad;
  std::map<int, std::vector<float> > ROCParam2_bad;
  std::map<int, std::vector<float> > ROCParam3_bad;
  std::map<int, std::vector<float> > ROCParam4_bad;
  std::map<int, TH2F*> hGoodFitMap;
  std::map<int, TH2F*> hBadFitMap;
  std::map<int, TH2F*> hAllFitMap;
  TDirectory* dGoodFits;
  TDirectory* dBadFits;

  // Loop over all entries in the map (which is by definiteion organized by pixel already, how nice
  for (std::map<int, std::vector< std::pair<float, float> > >::iterator It = Map.begin(); It != Map.end(); ++It) {

    // Which pixel is this anyway.. get the info
    UnPack(It->first, channel, roc, col, row);

    // This is ROC
    int const ROCId = 10 * channel + roc;

    //    Make a directory for good and bad fits
    TString gooddirname, baddirname;
    gooddirname.Form("Fits_Good_Channel%i_ROC%i", channel, roc);
    baddirname.Form("Fits_Bad_Channel%i_ROC%i", channel, roc);

    if (!fOutRoot.GetKey(gooddirname)){
      fOutRoot.cd();
      dGoodFits = fOutRoot.mkdir(gooddirname);
      dBadFits = fOutRoot.mkdir(baddirname);
    }

    // Hist for good bad and all fits
    //kga switched from GoodfitMap to AllFitMap...think that's right...
    if (hAllFitMap.count(ROCId) == 0) {
      TString MyName;
      MyName.Form("GoodFitMap_Channel%i_ROC%i", channel, roc);
      hGoodFitMap[ROCId] = new TH2F(MyName, MyName, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
      MyName.Form("BadFitMap_Channel%i_ROC%i", channel, roc);
      hBadFitMap[ROCId] = new TH2F(MyName, MyName, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
      MyName.Form("AllFitMap_Channel%i_ROC%i", channel, roc);
      hAllFitMap[ROCId] = new TH2F(MyName, MyName, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL + 1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW + 1);
    }

    // Input for TGraph, define size, and fill them
    float X[It->second.size()];
    float Y[It->second.size()];
    for (size_t i = 0; i != It->second.size(); ++i) {
      X[i] = It->second[i].first;
      Y[i] = It->second[i].second;
    }

    // Actually make a TGraph
    TGraphErrors g(It->second.size(), X, Y);
    TString const Name = TString::Format("Fit_Channel%i_ROC%1i_Col%2i_Row%2i", channel, roc, col, row);
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
//      printf("FitResult = %4i for %2i %1i %2i %2i\n", FitResult, channel, roc, col, row);

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


    // Save the graph to output file
    fOutRoot.cd();

    // Print the fit parameters to the output params file
    fprintf(fOutFits, "%2i %1i %2i %2i %12E %12E %12E %12E %12E\n", 
	    channel, roc, col, row, Param[0], Param[1], Param[2], Param[3], Param[4]);

  }

  fOutRoot.cd();
  for (std::map<int, TH2F*>::iterator It = hGoodFitMap.begin(); It != hGoodFitMap.end(); ++It) {
    It->second->Write();
    TCanvas Can;
    Can.cd();
    It->second->Draw("colz");
    Can.SaveAs(TString("plots/GainCal/") + It->second->GetName() + SUFFIX);
  }
  for (std::map<int, TH2F*>::iterator It = hBadFitMap.begin(); It != hBadFitMap.end(); ++It) {
    It->second->Write();
    TCanvas Can;
    Can.cd();
    It->second->Draw("colz");
    Can.SaveAs(TString("plots/GainCal/") + It->second->GetName() + SUFFIX);
  }
  for (std::map<int, TH2F*>::iterator It = hAllFitMap.begin(); It != hAllFitMap.end(); ++It) {
    It->second->Write();
    TCanvas Can;
    Can.cd();
    It->second->Draw("colz");
    Can.SaveAs(TString("plots/GainCal/") + It->second->GetName() + SUFFIX);
  }

  // Plot the Good values
  for (std::map<int, std::vector<float> >::iterator It = ROCChi2.begin(); It != ROCChi2.end(); ++It) {
    channel = It->first / 10;
    roc = It->first % 10;

    //Fit Chi2/NDF
    TString Name = TString::Format("Chi2_Channel%i_ROC%1i", channel, roc);
    fOutRoot.cd();
    TH1F hC(Name, Name, 100, 0, 1500);
    hC.GetXaxis()->SetTitle("Chi2/NDF");
    hC.SetNdivisions(5);
    for (size_t i = 0; i !=  ROCChi2[It->first].size(); ++i) {
      hC.Fill(ROCChi2[It->first][i]);
    }
    hC.Write();

    TString Namebad = TString::Format("Chi2_Channel%i_ROC%1i_bad", channel, roc);
    TH1F hCb(Namebad, Namebad, 100, 0, 1500);
    hCb.GetXaxis()->SetTitle("Chi2/NDF");
    hCb.SetNdivisions(5);
    for (size_t i = 0; i != ROCChi2_bad[It->first].size(); ++i) {
      hCb.Fill(ROCChi2_bad[It->first][i]);
    }
    hCb.Write();


    //Fit Saturation
    Name = TString::Format("Saturation_Channel%i_ROC%1i", channel, roc);
    TH1F hS(Name, Name, 100, 200, 300);
    hS.GetXaxis()->SetTitle("Fit Saturation Values");
    hS.SetNdivisions(5);
    for (size_t i = 0; i != ROCSaturationValues[It->first].size(); ++i) {
      hS.Fill(ROCSaturationValues[It->first][i]);
    }
    hS.Write();

    Namebad = TString::Format("Saturation_Channel%i_ROC%1i_bad", channel, roc);
    TH1F hSb(Namebad, Namebad, 100, 200, 300);
    hSb.GetXaxis()->SetTitle("Fit Saturation Values");
    hSb.SetNdivisions(5);
    for (size_t i = 0; i != ROCSaturationValues_bad[It->first].size(); ++i) {
      hSb.Fill(ROCSaturationValues_bad[It->first][i]);
    }
    hSb.Write();


    //Fit Parameter 0
    Name = TString::Format("Param0_Channel%i_ROC%1i", channel, roc);
    TH1F h0(Name, Name, 100, -1, 1);
    h0.GetXaxis()->SetTitle("Fit Parameter 0");
    h0.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam0[It->first].size(); ++i) {
      h0.Fill(ROCParam0[It->first][i]);
    }
    h0.Write();

    Namebad = TString::Format("Param0_Channel%i_ROC%1i_bad", channel, roc);
    TH1F h0b(Namebad, Namebad, 100, -1, 1);
    h0b.GetXaxis()->SetTitle("Fit Parameter 0");
    h0b.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam0_bad[It->first].size(); ++i) {
      h0b.Fill(ROCParam0_bad[It->first][i]);
    }
    h0b.Write();

    //Fit Parameter 1
    Name = TString::Format("Param1_Channel%i_ROC%1i", channel, roc);
    TH1F h1(Name, Name, 100, -150, 150);
    h1.GetXaxis()->SetTitle("Fit Parameter 1");
    h1.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam1[It->first].size(); ++i) {
      h1.Fill(ROCParam1[It->first][i]);
    }
    h1.Write();
    
    Namebad = TString::Format("Param1_Channel%i_ROC%1i_bad", channel, roc);
    TH1F h1b(Namebad, Namebad, 100, -150, 150);
    h1b.GetXaxis()->SetTitle("Fit Parameter 1");
    h1b.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam1_bad[It->first].size(); ++i) {
      h1b.Fill(ROCParam1_bad[It->first][i]);
    }
    h1b.Write();


    //Fit Parameter 2
    Name = TString::Format("Param2_Channel%i_ROC%1i", channel, roc);
    TH1F h2(Name, Name, 100, -10000, 10000);
    h2.SetNdivisions(5);
    h2.GetXaxis()->SetTitle("Fit Parameter 2");
    for (size_t i = 0; i != ROCParam2[It->first].size(); ++i) {
      h2.Fill(ROCParam2[It->first][i]);
    }
    h2.Write();

    Namebad = TString::Format("Param2_Channel%i_ROC%1i_bad", channel, roc);
    TH1F h2b(Namebad, Namebad, 100, -10000, 10000);
    h2b.SetNdivisions(5);
    h2b.GetXaxis()->SetTitle("Fit Parameter 2");
    for (size_t i = 0; i != ROCParam2_bad[It->first].size(); ++i) {
      h2b.Fill(ROCParam2_bad[It->first][i]);
    }
    h2b.Write();


    //Fit Parameter 3
    Name = TString::Format("Param3_Channel%i_ROC%1i", channel, roc);
    TH1F h3(Name, Name, 100, -400, 225);//100, 225);
    h3.GetXaxis()->SetTitle("Fit Parameter 3");
    h3.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam3[It->first].size(); ++i) {
      h3.Fill(ROCParam3[It->first][i]);
    }
    h3.Write();

    Namebad = TString::Format("Param3_Channel%i_ROC%1i_bad", channel, roc);
    TH1F h3b(Namebad, Namebad, 100, -600, 225);//100, 225);
    h3b.GetXaxis()->SetTitle("Fit Parameter 3");
    h3b.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam3_bad[It->first].size(); ++i) {
      h3b.Fill(ROCParam3_bad[It->first][i]);
    }
    h3b.Write();

    //Fit Parameter 4
    Name = TString::Format("Param4_Channel%i_ROC%1i", channel, roc);
    TH1F h4(Name, Name, 100, -10,200);//-2,20);
    h4.GetXaxis()->SetTitle("Fit Parameter 4");
    h4.SetNdivisions(5);
    for (size_t i = 0; i != ROCParam4[It->first].size(); ++i) {
      h4.Fill(ROCParam4[It->first][i]);
    }
    h4.Write();

    Namebad = TString::Format("Param4_Channel%i_ROC%1i_bad", channel, roc);
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

    can.Print(TString::Format("plots/GainCal/GainCalFitParams_Channel%i_ROC%1i", channel, roc) + SUFFIX);
    
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
