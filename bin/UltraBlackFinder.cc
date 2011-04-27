////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri Apr  1 12:54:53 CDT 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include "TString.h"
#include "TMath.h"

int const kNUMBER = 900;
int const MAX_ULTRABLACKS = 10;



unsigned long GetDataFromFedBuffer(unsigned long* b, int i) { return b[i]; }

void FindUltrablacks(unsigned long* buffer, double baseline, double rms, int* ultrablacks, int& nub) {


   // Now we look for the first excursion above 10*RMS
   int firstUB = -1, lastTrailerBin = -1, lastUB = -1;
   unsigned long dat;
   for(int i = 0; i<900; i++) {
      dat = GetDataFromFedBuffer(buffer,i);
      if((dat < (baseline - 30*rms)) && (firstUB==-1)) {
        firstUB = i;
      } else if((dat < (baseline - 10*rms))) {
         lastTrailerBin = i;
      }
   }

   lastUB = lastTrailerBin - 5;

   double ultrablackLevel = (baseline - GetDataFromFedBuffer(buffer,firstUB))/2.0 + GetDataFromFedBuffer(buffer,firstUB);

   nub = 0;
   for(int i = 0; i< kNUMBER; i++) {
      dat = GetDataFromFedBuffer(buffer,i);
      if((dat < ultrablackLevel) ) {
         if(nub+1 >= MAX_ULTRABLACKS) {
            nub = -1;
            return;
         }
         ultrablacks[nub++]=i;
      }
   }
   return;
}





std::pair<float, float> FindBaseline(long unsigned* Value)
{
  long double Sum  = 0;
  long double Sum2 = 0;
  for (int i = 600; i < 900; ++i) {
    Sum  += Value[i];
    Sum2 += Value[i]*Value[i];
  }

  Sum /= 300.0;
  Sum2 = TMath::Sqrt(Sum2)/300.0;
  return std::make_pair<float, float>(Sum, Sum2);
}


void FindUltraBlacks (float const Baseline, float const BaselineErr, long unsigned* Value, std::vector<int>& UBs)
{
  int MinI = -1;
  int Min = 9999;
  for (int i = 0; i < kNUMBER; ++i) {
    if (Value[i] < Baseline - 4*BaselineErr) {
      MinI = i;
      Min = Value[i];
      for ( ; Value[i] < Baseline - 4*BaselineErr; ++i) {
        if (Value[i] < Min) {
          MinI = i;
        }
      }
      UBs.push_back(MinI);
      //printf("ultrablack found: %i  %8u\n", MinI, Min);
      Min = 9999;
    }
  }
  return;
}


int UltraBlackFinder (int const N, TString const InFileName)
{
  // Open input file
  std::ifstream InFile(InFileName.Data());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open file for reading: " << InFileName << std::endl;
    exit(1);
  }

 
  long unsigned Value[kNUMBER];


  std::string line;
  for (int i = 1; i < N && !InFile.eof(); ++i) {
    std::getline(InFile, line);
  }

  for (int i = 0; i != kNUMBER; ++i) {
    InFile >> Value[i];
  }

  std::pair<float, float> const Baseline = FindBaseline(Value);
  printf("Baseline is: %12.2f +/- %12.4f\n", Baseline.first, Baseline.second);
  std::vector<int> MyUBs;
  FindUltraBlacks(Baseline.first, Baseline.second, Value, MyUBs);
  for (int i = 0; i < MyUBs.size(); ++i) {
    printf("UB:%3i %5u  %5u\n", i, MyUBs[i], Value[ MyUBs[i] ]);
  }
  int UBs[10];
  int nub;

  FindUltrablacks(Value, Baseline.first, Baseline.second, UBs, nub);
  for (int i = 0; i < nub; ++i) {
    printf("ub:%3i %5u  %5u\n", i, UBs[i], Value[ UBs[i] ]);
  }


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [N] [Input.dat]" << std::endl;
    return 1;
  }

  int const N = atoi(argv[1]);
  TString const InFileName = argv[2];

  UltraBlackFinder(N, InFileName);

  return 0;
}
