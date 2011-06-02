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



TString Pack (int const mFec, int const mFecChannel, int const hubAddress, int const roc, int const col, int const row)
{
  // Just pack this into something easy to decode
  TString ret;
  char BUFF[10];
  sprintf(BUFF, "%1i%1i%2i%1i%2i%2i", mFec, mFecChannel, hubAddress, roc, col, row);
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

  // Open the output root file
  TString const OutRootName = "Test.root";
  TFile fOutRoot(OutRootName, "recreate");
  if (!fOutRoot.IsOpen()) {
    std::cerr << "ERROR: Cannot open output root file: " << OutRootName << std::endl;
    throw;
  }

  // Open root file outout
  TString const OutFitsName = "TestOut.dat";
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
  int mFec, mFecChannel, hubAddress, roc, col, row;
  float adc, vcal;

  // Keep track of which ROCs and what VCals we use
  char ROCId[10];
  std::set<TString> ROCNames;
  std::set<int>     VCals;

  // Loop over all lines in the input data file
  for (std::string line; std::getline(f, line); ) {
    s.clear();
    s.str(line);
    s >> mFec
      >> mFecChannel
      >> hubAddress
      >> roc
      >> col
      >> row
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
  TF1 FitFunc("FitFunc", "TMath::Exp( (x-[0]) / [1]  ) + [2]*x*x + [3]*x + [4] ", 150, 400);

  // Define Chi2 plot for all pixels
  TH1F FitChi2("FitChi2", "FitChi2", 200, 0, 1000);

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

  // Loop over all entries in the map (which is by definiteion organized by pixel already, how nice
  for (std::map<TString, std::vector< std::pair<float, float> > >::iterator It = Map.begin(); It != Map.end(); ++It) {

    // Which pixel is this anyway.. get the info
    UnPack(It->first.Data(), mFec, mFecChannel, hubAddress, roc, col, row);

    // This is ROC
    sprintf(ROCId, "%1i%1i%02i%1i", mFec, mFecChannel, hubAddress, roc);

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
    g.SetName("Fit_"+It->first);
    g.SetTitle("Fit_"+It->first);

    // Get the min and max point from the graph
    g.GetPoint(0, adcMin, vcalMin);
    g.GetPoint(g.GetN()-1, adcMax, vcalMax);

    // Set the range of the fit
    FitFunc.SetRange(120, 300);

    // Some default parameters to start off with
    FitFunc.SetParameter(0, adcMax);
    FitFunc.SetParameter(1, 60);
    FitFunc.SetParameter(2, 0.1);
    FitFunc.SetParameter(3, -30);
    FitFunc.SetParameter(4, 2000);

    // Do the fit
    int FitResult = g.Fit("FitFunc", "Q");
    if (FitResult != 0) {
      printf("FitResult = %4i for %1i %1i %2i %2i %2i\n", FitResult, mFec, mFecChannel, hubAddress, col, row);
    }

    // Polt the Chi2
    FitChi2.Fill(FitFunc.GetChisquare());

    // Grab the parameters from the fit
    Param[0] = FitFunc.GetParameter(0);
    Param[1] = FitFunc.GetParameter(1);
    Param[2] = FitFunc.GetParameter(2);
    Param[3] = FitFunc.GetParameter(3);
    Param[4] = FitFunc.GetParameter(4);

    // Save the graph to output file
    fOutRoot.cd();
    g.Write();

    // Print the fit parameters to the output params file
    fprintf(fOutFits, "%1i %1i %2i %1i %2i %2i %12E %12E %12E %12E %12E\n", mFec, mFecChannel, hubAddress, roc, col, row, Param[0], Param[1], Param[2], Param[3], Param[4]);

    // Add saturation value to values
    ROCSaturationValues[ROCId].push_back(Param[0]);
    ROCChi2[ROCId].push_back(FitFunc.GetChisquare());

  }

  // Plot the saturation values
  for (std::map<TString, std::vector<float> >::iterator It = ROCSaturationValues.begin(); It != ROCSaturationValues.end(); ++It) {
    fOutRoot.cd();
    TH1F h( TString("Saturation_")+It->first, TString("Saturation_")+It->first, 100, 0, 400);
    for (size_t i = 0; i != It->second.size(); ++i) {
      h.Fill(It->second[i]);
    }
    h.Write();
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
