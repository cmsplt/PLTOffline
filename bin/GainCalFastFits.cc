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

#include "TString.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"
#include "TFitResult.h"



TString Pack (int const mFec, int const mFecChannel, int const hubAddress, int const roc, int const col, int const row)
{
  TString ret;
  char BUFF[10];
  sprintf(BUFF, "%1i%1i%2i%1i%2i%2i", mFec, mFecChannel, hubAddress, roc, col, row);
  ret = BUFF;
  return ret;
}

void UnPack (const char* in, int& mFec, int& mFecChannel, int& hubAddress, int& roc, int& col, int& row)
{
  sscanf(in, "%1i%1i%2i%1i%2i%2i", &mFec, &mFecChannel, &hubAddress, &roc, &col, &row);
  return;
}


int GainCalFastFits (TString const InFileName)
{
  std::ifstream f(InFileName.Data());
  if (!f) {
    std::cerr << "ERROR; Cannot open file: " << InFileName << std::endl;
    exit(1);
  }

  TString const OutRootName = "Test.root";
  TFile Out(OutRootName, "recreate");
  if (!Out.IsOpen()) {
    std::cerr << "ERROR: Cannot open output root file: " << OutRootName << std::endl;
    throw;
  }

  TString const OutDatName = "TestOut.dat";
  FILE* OutDat = fopen(OutDatName.Data(), "w");
  if (!OutDat) {
    std::cerr << "ERROR: cannot open out data file: " << OutDatName << std::endl;
    throw;
  }

  std::map<TString, std::vector< std::pair<float, float> > > Map;

  std::stringstream s;

  int mFec, mFecChannel, hubAddress, roc, col, row;
  float adc, vcal;

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

    TString Id = Pack(mFec, mFecChannel, hubAddress, roc, col, row);
    Map[Id].push_back( std::make_pair(adc, vcal) );

  }

  TF1 FitFunc("FitFunc", "TMath::Exp( (x-[0]) * [1]  ) + [2]*x*x + [3]*x + [4] ", 150, 400);
  TH1F FitChi2("FitChi2", "FitChi2", 200, 0, 1000);

  float Param[5];
  for (std::map<TString, std::vector< std::pair<float, float> > >::iterator It = Map.begin(); It != Map.end(); ++It) {
    float X[It->second.size()];
    float Y[It->second.size()];
    for (size_t i = 0; i != It->second.size(); ++i) {
      X[i] = It->second[i].first;
      Y[i] = It->second[i].second;
    }

    UnPack(It->first.Data(), mFec, mFecChannel, hubAddress, roc, col, row);
    TGraphErrors g(It->second.size(), X, Y);
    g.SetName("Fit_"+It->first);
    g.SetTitle("Fit_"+It->first);
    double adcMin, vcalMin, adcMax, vcalMax;
    g.GetPoint(0, adcMin, vcalMin);
    g.GetPoint(g.GetN()-1, adcMax, vcalMax);
    FitFunc.SetRange(120, 300);
    FitFunc.SetParameter(0, adcMax);
    FitFunc.SetParameter(1, 60);
    FitFunc.SetParameter(2, 0.1);
    FitFunc.SetParameter(3, -30);
    FitFunc.SetParameter(4, 2000);
    int FitResult = g.Fit("FitFunc", "MQ");

    FitChi2.Fill(FitFunc.GetChisquare() / FitFunc.GetNDF());
    //if (FitFunc.GetChisquare() / FitFunc.GetNDF() > 500) {
    //  std::cout << g.GetName() << std::endl;
    //}


    Param[0] = FitFunc.GetParameter(0);
    Param[1] = FitFunc.GetParameter(1);
    Param[2] = FitFunc.GetParameter(2);
    Param[3] = FitFunc.GetParameter(3);
    Param[4] = FitFunc.GetParameter(4);
    g.Write();


    fprintf(OutDat, "%1i %2i %2i %1i %2i %2i %12E %12E %12E %12E %12E\n", mFec, mFecChannel, hubAddress, roc, col, row, Param[0], Param[1], Param[2], Param[3], Param[4]);


  }

  FitChi2.Write();

  Out.Close();

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
