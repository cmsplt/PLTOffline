////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Jul 18 12:36:04 CEST 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <stdlib.h>

#include "TFile.h"
#include "TH1F.h"
#include "TString.h"


int PlotADCForVCal (std::string const InFileName, std::string const OutFileName)
{
  std::ifstream InFile(InFileName.c_str());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open input file: " << InFileName << std::endl;
    exit (1);
  }

  TFile OutFile(OutFileName.c_str(), "recreate");
  if (!OutFile.IsOpen()) {
    std::cerr << "ERROR: cannot open output file: " << OutFileName << std::endl;
    exit (1);
  }

  for (std::string Line; std::getline(InFile, Line); ) {
    if (Line == "") {
      break;
    }
  }

  std::map<TString, TH1F*> hMap;
  int Channel, Column, Row, ROC, NHits;
  float ADC, VCal;
  while (InFile >> Channel >> Column >> Row >> ROC >> ADC >> VCal >> NHits) {
    if (InFile.eof()) {
      break;
    }

    TString const Name = TString::Format("Channel%i_ROC%i_VCal_%i", Channel, ROC, (int) VCal);
    if (!hMap.count(Name)) {
      hMap[Name] = new TH1F(Name, Name, 100, 150, 250);
      hMap[Name]->SetDirectory(&OutFile);
    }

    hMap[Name]->Fill(ADC);
  }

  OutFile.Write();
  OutFile.Close();

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFile] [OutFile]" << std::endl;
    return 1;
  }

  std::string const InFileName  = argv[1];
  std::string const OutFileName = argv[2];

  PlotADCForVCal(InFileName, OutFileName);

  return 0;
}
