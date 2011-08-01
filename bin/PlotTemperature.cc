////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Apr 13 10:31:11 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <cstdlib>


#include "TString.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TDatime.h"


int PlotTemperature (TString const FileName, TString const BeginDate, TString const EndDate)
{
  // Input file
  std::ifstream f(FileName.Data());
  if (!f) {
    std::cerr << "ERROR: cannot open input file: " << FileName << std::endl;
    throw;
  }

  unsigned long long NLinesTotal = 0;
  for (TString Line; Line.ReadLine(f); ++NLinesTotal) {
  }
  f.close();
  f.open(FileName.Data());


  // Begin and end dates
  int const bDate = atoi(BeginDate.Data());
  int const eDate = atoi(EndDate.Data());

  // Variables we'll read from file
  int Year, Month, Day, Hour, Min, Sec, ScanState;
  int ThisDate;
  float Temp;
  double DateFrac;

  int const NPOINTS = 50000;
  double X[NPOINTS];
  double Y[NPOINTS];
  int Sampling = 100 * NLinesTotal / (unsigned long long) NPOINTS;
  printf("NPOINTS=%i  NLinesTotal=%i  Sampling=%i\n", NPOINTS, (int) NLinesTotal, Sampling);

  // Read each line
  int nLines = 0;
  int nUsed  = 0;
  for (TString Line; Line.ReadLine(f); ++nLines) {
    if (100*nLines % Sampling != 0) {
      continue;
    }

    ScanState = sscanf(Line.Data(), "%4i%2i%2i.%2i.%2i.%2i %f", &Year, &Month, &Day, &Hour, &Min, &Sec, &Temp);
    if (ScanState != 7) {
      //std::cout << "Skipping line: " << Line << "  nLine: " << nLines << std::endl;
      continue;
    }

    ScanState = sscanf(Line.Data(), "%8i", &ThisDate);


    if (BeginDate != "" && ThisDate < bDate) {
      continue;
    }
    if (EndDate != "" && ThisDate > eDate) {
      continue;
    }


    TDatime D(Year, Month, Day, Hour, Min, Sec);
    DateFrac = D.Convert();
    //DateFrac = ThisDate + (Hour*60.*60. + Min*60. + Sec) / (24.*60.*60.);

    if (nUsed >= NPOINTS) {
      std::cout << "HERE" << std::endl;
      throw;
      break;
    }


    X[nUsed] = DateFrac;
    Y[nUsed] = Temp;

    printf("%4i/%02i/%02i %02i:%02i:%02i Temperature: %5.1f\n", Year, Month, Day, Hour, Min, Sec, Temp);
    //printf("%f\n", DateFrac);

    ++nUsed;
  }



  TGraph g(nUsed, X, Y);
  g.SetTitle("Temperature");
  g.GetXaxis()->SetTimeDisplay(1);
  g.GetXaxis()->SetTitle("Date");
  g.GetYaxis()->SetTitle("Temperature (#circC )");
  g.GetXaxis()->SetTimeFormat("%d/%m");
  //g.GetXaxis()->SetTimeFormat("%d/%m %H:%m");
  //g.GetXaxis()->SetTimeFormat("%d/%m/%Y");
  TDatime D(1995, 1, 1, 0, 0, 0);
  g.GetXaxis()->SetTimeOffset(D.Convert()-25*365.25*24*60*60);
  TCanvas c("Temperature", "Temperature Graph");
  c.cd();
  //g.GetXaxis()->SetBinLabel(0, "HIHI");
  g.GetYaxis()->SetNdivisions(6);
  g.GetXaxis()->SetNdivisions(-6);
  g.SetMarkerStyle(7);
  g.Draw("ac");
  c.SaveAs("Temperature.gif");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [FileName]" << std::endl;
    return 1;
  }

  TString const FileName = argv[1];

  PlotTemperature(FileName, "", "");

  return 0;
}
