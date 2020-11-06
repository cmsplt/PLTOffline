////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu Jul 21 17:10:22 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <vector>

#include "TString.h"
#include "TMath.h"


float MidPoint (std::vector<float> const& xs, std::vector<float> const& ys, float const X)
{
  int iA = -1;
  int iB = -1;
  for (size_t i = 0; i != xs.size(); ++i) {
    if (xs[i] < X) {
      iA = i;
    } else if (xs[i] > X) {
      iB = i;
      break;
    }
  }

  if (iA >= 0 && iB >= 0) {
    return (ys[iA] + ys[iB]) / 2.0;
  }

  return 0;
}



int PlotPixelVsTimeMagTest (std::string const InFileName, std::string const CurrentFileName, int const imFec, int const imFecChannel, int const ihubAddress, int const iROC, int const icol, int const irow, float const ivcal)
{
  // Read input file
  std::ifstream InFile(InFileName.c_str());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open file: ";
    std::cerr << InFileName << std::endl;
    throw;
  }

  std::vector<std::string> FileNames;
  for (std::string name; std::getline(InFile, name); ) {
    FileNames.push_back(name);
  }
  InFile.close();

  std::ifstream InCurrentsFile(CurrentFileName.c_str());
  if (!InCurrentsFile.is_open()) {
    std::cerr << "ERROR: cannot open file: ";
    std::cerr << CurrentFileName << std::endl;
    throw;
  }

  std::vector<float> MagSeconds;
  std::vector<float> MagCurrent;
  int hour, min, sec, current;
  int StartSecondUTC = 0;
  for (int i = 0; !InCurrentsFile.eof(); ++i) {
    InCurrentsFile >> hour >> min >> sec >> current;
    if (StartSecondUTC == 0) {
      StartSecondUTC = (hour+2)*60*60 + min*60 + sec;
      printf("Start time in seconds is: %i\n", StartSecondUTC);
    }

    MagSeconds.push_back( float( (hour+2)*60*60 + min*60 + sec - StartSecondUTC) );
    MagCurrent.push_back( (float) current );
  }
  InCurrentsFile.close();


  // print info just to see we got that ok
  for (size_t i = 0; i != MagSeconds.size(); ++i) {
    printf("Time and Current: %12.1f  %12.1f\n", MagSeconds[i], MagCurrent[i]);
  }


  //float AvgADC[1000];
  //float Time[1000];
  int iHit = 0;

  for (size_t i = 0; i != FileNames.size(); ++i) {
    sscanf(FileNames[i].c_str(), "gaincalfast_%8i.%2i%2i%2i.avg.txt", &hour, &hour, &min, &sec);
    float SecondsSinceStart = float( hour*60*60 + min*60 + sec - StartSecondUTC);
    if (SecondsSinceStart < 0) {
      continue;
    }

    float const ThisCurrent = MidPoint(MagSeconds, MagCurrent, SecondsSinceStart);
    printf("for file %s  current is: %12.1f\n", FileNames[i].c_str(), ThisCurrent);

    std::ifstream In(FileNames[i].c_str());
    int mf, mfc, hub, roc, col, row, n;
    float adc, vcal;
    for (std::string line; std::getline(In, line); ) {
      sscanf(line.c_str(), "%1i %1i %2i %1i %2i %2i %f %f %i", &mf, &mfc, &hub, &roc, &col, &row, &adc, &vcal, &n);
    }

    if (mf != imFec || mfc != imFecChannel || hub != ihubAddress || roc != iROC || col != icol || row != irow || vcal != ivcal) {
      continue;
    }
    //AvgADC[iHit] = adc;
    //Time[iHit] = SecondsSinceStart;
    ++iHit;
  }




  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InputsFileName] [CurrentFileName]" << std::endl;
    return 1;
  }

  std::string const InputsFileName = argv[1];
  std::string const CurrentFileName = argv[2];
  PlotPixelVsTimeMagTest(InputsFileName, CurrentFileName, 8, 2, 21, 0, 44, 44, 160);

  return 0;
}
