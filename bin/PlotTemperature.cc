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

  // Check input date format
  if (BeginDate != "" && BeginDate.Length() != 15) {
    std::cerr << "ERROR: Date format must be: YYYYMMDD:HHmmSS" << std::endl;
    return -1;
  }
  if (EndDate != "" && EndDate.Length() != 15) {
    std::cerr << "ERROR: Date format must be: YYYYMMDD:HHmmSS" << std::endl;
    return -1;
  }

  unsigned long long NLinesTotal = 0;
  for (TString Line; Line.ReadLine(f); ++NLinesTotal) {
  }
  f.close();
  f.clear();
  f.open(FileName.Data());




  // Begin and end dates
  int const bDate = atoi(BeginDate.Data());
  int const eDate = atoi(EndDate.Data());


  // Variables we'll read from file
  int Year, Month, Day, Hour, Min, Sec, ScanState;
  int ThisDate;
  float Temp;
  double DateFrac;

  Month = 1;
  Day = 1;
  Hour = 0;
  Min = 0;
  Sec = 0;

  if (BeginDate != "") {
    sscanf(BeginDate.Data(), "%4i%2i%2i.%2i%2i%2i", &Year, &Month, &Day, &Hour, &Min, &Sec);
  } else {
    Year = 1995;
  }
  printf("BeginDate: %4i%02i%02i.%02i%02i%02i\n", Year, Month, Day, Hour, Min, Sec);
  TDatime bDT(Year, Month, Day, Hour, Min, Sec);


  if (EndDate != "") {
    sscanf(EndDate.Data(), "%4i%2i%2i.%2i%2i%2i", &Year, &Month, &Day, &Hour, &Min, &Sec);
  } else {
    Year = 2133;
  }
  printf("EndDate:   %4i%02i%02i.%02i%02i%02i\n", Year, Month, Day, Hour, Min, Sec);
  TDatime eDT(Year, Month, Day, Hour, Min, Sec);

  if (bDT > eDT) {
    // You're probably thinking the same thing I am... this is totally backwards..
    std::cerr << "ERROR: Begin is after ending?  You're crazy." << std::endl;
    throw;
  }



  int const NPOINTS = 50000;
  double X[NPOINTS];
  double Y[NPOINTS];
  int Sampling = 100 * NLinesTotal / (unsigned long long) NPOINTS;
  printf("NPOINTS=%i  NLinesTotal=%i  Sampling=%i\n", NPOINTS, (int) NLinesTotal, Sampling);


  // Read each line
  int nLines = 0;
  int nUsed  = 0;
  for (TString Line; Line.ReadLine(f); ++nLines) {
    if (Sampling != 0 && 100*nLines % Sampling != 0) {
      continue;
    }

    ScanState = sscanf(Line.Data(), "%4i%2i%2i.%2i.%2i.%2i %f", &Year, &Month, &Day, &Hour, &Min, &Sec, &Temp);
    if (ScanState != 7) {
      //std::cout << "Skipping line: " << Line << "  nLine: " << nLines << std::endl;
      continue;
    }

    sscanf(Line.Data(), "%8i", &ThisDate);


    // Simple date check
    if (BeginDate != "" && ThisDate < bDate) {
      continue;
    }
    if (EndDate != "" && ThisDate > eDate) {
      continue;
    }


    TDatime D(Year, Month, Day, Hour, Min, Sec);
    DateFrac = D.Convert();

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

  float TimeDiff = X[nUsed - 1] - X[0];


  TGraph g(nUsed, X, Y);
  g.SetTitle("Temperature");
  g.GetXaxis()->SetTimeDisplay(1);
  g.GetYaxis()->SetTitle("Temperature (#circC )");
  if (TimeDiff > 60*60*24*365) {
    g.GetXaxis()->SetTimeFormat("%m/%y");
    g.GetXaxis()->SetTitle("Month/Year");
  } else if (TimeDiff > 60*60*24*30) {
    g.GetXaxis()->SetTimeFormat("%d/%m");
    g.GetXaxis()->SetTitle("Day/Month");
  } else if (TimeDiff > 60*60*24) {
    g.GetXaxis()->SetTimeFormat("%d/%m");
    g.GetXaxis()->SetTitle("Day/Month");
  } else if (TimeDiff > 60*60) {
    g.GetXaxis()->SetTimeFormat("%H:%M");
    g.GetXaxis()->SetTitle("Hour:Minute");
  } else if (TimeDiff > 60) {
    g.GetXaxis()->SetTimeFormat("%m:%s");
    g.GetXaxis()->SetTitle("Minute:Second");
  } else if (TimeDiff > 0) {
    g.GetXaxis()->SetTimeFormat("%m:%s");
  }

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
  g.Draw("al");
  c.SaveAs("Temperature.gif");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " [FileName] ([BeginDate] [EndDate])" << std::endl;
    return 1;
  }

  TString const FileName = argv[1];
  if (argc == 3) {
    PlotTemperature(FileName, argv[2], "");
  } else if (argc == 4) {
    PlotTemperature(FileName, argv[2], argv[3]);
  } else {
    PlotTemperature(FileName, "", "");
  }

  return 0;
}
