////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Sun Apr 27 16:22:43 CEST 2014
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

#include "TString.h"
#include "TGraph.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"


int PSIGainCalFits (TString const InFileName, int const roc)
{
  // Open the input file
  std::ifstream f(InFileName.Data());
  if (!f) {
    std::cerr << "ERROR; Cannot open file: " << InFileName << std::endl;
    throw;
  }

  TFile OutRoot(InFileName+".fit.root", "recreate");
  OutRoot.cd();
  FILE* OutFits = fopen( (InFileName+".fit.dat").Data(), "w");

  // These are the 5 parameters from the fit we care about
  float Param[5];

  // All will be in channe "1"
  int const channel = 1;

  // print header to output file
  fprintf(OutFits, "8 1 5 %i\n\n", channel);

  std::string OneLine;
  std::getline(f, OneLine);
  std::getline(f, OneLine);
  std::getline(f, OneLine);
  std::getline(f, OneLine);

  int VCAL[10] =  {50, 100, 150, 200, 250, 30*7,  50*7,  70*7,  90*7, 200*7};
  int X[10], Y[10];

  // Define the function we will fit for each pixel
  TF1 FitFunc("FitFunc", "[0]*x*x + [1]*x + [2] + TMath::Exp( (x-[3]) / [4]  )", 150, 400);

  // Define Chi2 plot for all pixels
  TH1F FitChi2("FitChi2", "FitChi2", 200, 0, 5000);


  // Loop over all lines in the input data file
  std::istringstream s;
  TString Pix;
  int row, col;
  for (std::string line; std::getline(f, line); ) {
    s.clear();
    s.str(line);

    std::string adcstring;
    int adc;
    int n = 0;
    for (int i = 0; i != 10; ++i) {
      s >> adcstring;
      if (adcstring != "N/A") {
        adc = atoi(adcstring.c_str());
        X[n] = adc;
        Y[n] = VCAL[i];
        ++n;
        //printf(" %5i", adc);
      } else {
        //std::cout << "   N/A";
      }
    }
    s >> Pix >> col >> row;


    // Set the range of the fit
    FitFunc.SetRange(-2000, 2000);

    // Some default parameters to start off with
    FitFunc.SetParameter(0, 0.1);
    FitFunc.SetParameter(1, -30);
    FitFunc.SetParameter(2, 2000);
    FitFunc.SetParameter(3, 500);
    FitFunc.SetParameter(4, 60);

    // Do the fit
    TGraph g(n, X, Y);
    g.SetName( TString::Format("phCal_c%02i_r%02i", col, row) );
    int FitResult = g.Fit("FitFunc", "QM", "", -2000, 2000);
    g.Write();

    // Grab the parameters from the fit
    Param[0] = FitFunc.GetParameter(0);
    Param[1] = FitFunc.GetParameter(1);
    Param[2] = FitFunc.GetParameter(2);
    Param[3] = FitFunc.GetParameter(3);
    Param[4] = FitFunc.GetParameter(4);

    //printf(" col %2i row %2i  FitResult %i  Chi2/ndf %9.3E\n", col, row, FitResult, FitFunc.GetChisquare()/FitFunc.GetNDF());
    FitChi2.Fill(FitFunc.GetChisquare()/FitFunc.GetNDF());

    // Print the fit parameters to the output params file
    fprintf(OutFits, "%2i %1i %2i %2i %12E %12E %12E %12E %12E\n", 
	    channel, roc, col, row, Param[0], Param[1], Param[2], Param[3], Param[4]);

  }

  FitChi2.Write();
  OutRoot.Close();

  fclose(OutFits);

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFileName] [ROC]" << std::endl;
    return 1;
  }

  TString const InFileName = argv[1];
  int const roc = atoi(argv[2]);

  PSIGainCalFits(InFileName, roc);

  return 0;
}
