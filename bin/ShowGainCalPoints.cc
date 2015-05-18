////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu Apr 21 16:27:50 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>


#include "TGraph.h"
#include "TCanvas.h"


int ShowGainCalPoints (std::string const InFileName, int const InChannel, int const Inroc, int const Incol, int const Inrow)
{

  std::ifstream f(InFileName.c_str());
  if (!f) {
    std::cerr << "ERROR; Cannot open file: " << InFileName << std::endl;
    throw;
  }

  int const NMaxPoints = 200;
  double ADC [NMaxPoints];
  double VCAL[NMaxPoints];

  std::stringstream s;

  int channel, roc, col, row;
  float adc, vcal;

  int i = 0;
  for (std::string line; std::getline(f, line); ) {
    s.clear();
    s.str(line);
    s >> channel
      >> col
      >> row
      >> roc
      >> adc
      >> vcal;
//    std::cout<<" ch "<<channel<<" roc "<<roc<<" col "<<col<<" row "<<row<<std::endl;
    if (channel == InChannel && roc == Inroc && col == Incol && row == Inrow) {

      ADC[i]  = adc;
      VCAL[i] = vcal;
      std::cout<<"count "<< i<< " adc "<<adc<<"    "<<vcal<<std::endl;
      ++i;
      if (i == NMaxPoints) {
        break;
      }
    }
  }

  TGraph g(i, VCAL, ADC);
  TCanvas c;
  c.cd();
  g.Draw("a*");
  char BUFF[500]; 
  sprintf(BUFF, "GainCalPoints_Ch%i_ROC%i_COL%i_ROW%i.png", InChannel, Inroc, Incol, Inrow);
  c.SaveAs(BUFF);


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 6) {
    std::cerr << "Usage: " << argv[0] << " [InFileName] [chan] [roc] [col] [row]" << std::endl;
    return 1;
  }

  std::string InFileName = argv[1];
  int const channel = atoi(argv[2]);
  int const roc = atoi(argv[3]);
  int const col = atoi(argv[4]);
  int const row = atoi(argv[5]);

  ShowGainCalPoints(InFileName, channel, roc, col, row);

  return 0;

}
