////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Oct 18 10:03:56 CEST 2011
//

////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <vector>
#include "TMath.h"


int MakeStraightAlignment (std::string const FileName)
{
  FILE* f = fopen(FileName.c_str(), "w");
  if (!f) {
    std::cerr << "ERROR: cannot open file for writing: " << FileName << std::endl;
    throw;
  }

  float const Radius = 4.698;
  std::vector<int> ChNo;

  for(int j =1; j <24; j+=j%3){
    ChNo.push_back(j % 24);
  }
  
  fprintf(f, "#first line:  Channel,-1, Tele.GRZ, Tele.GRY, Tele.GX, Tele.GY, Tele.GZ \n");
  fprintf(f, "#subsequent lines:  Channel, iroc, C.LR, C.LX, C.LY, C.LZ \n");
  //fprintf(f, "# numbers are:\n");
  //fprintf(f, "# Channel   Plane   CWLRotation   LXTrans  LYTrans  LZTrans\n\n");
  for (int ich = 1; ich <= 16; ++ich) {

    double halfrot = ich >= 9 ? TMath::Pi() : 0;
    double Phi    = (ich - 1.) * TMath::Pi() / 4. + TMath::Pi()/8 + halfrot;
    double PhiDet = (ich - 1.) * TMath::Pi() / 4. + TMath::Pi()/8+TMath::Pi()/2 + halfrot;
    double Theta = ich >= 9 ? 0 : TMath::Pi();

    float X = Radius * TMath::Cos(PhiDet);
    float Y = Radius * TMath::Sin(PhiDet);
    
    //X = (X < 0.001 ? 0. : X);
    //Y = (Y < 0.001 ? 0. : Y);
    if (fabs(X) < 0.00001) {
      X = 0.0;
    }
    if (fabs(Y) < 0.00001) {
      Y = 0.0;
    }

    //if (X > 0) X += 2.0;
    //if (X < 0) X -= 2.0;

    fprintf(f, "%2i  %2i ", ChNo[ich-1], -1);
    fprintf(f, "    %15E", Phi);
    fprintf(f, "    %15E", Theta);
    fprintf(f, "    %15E", X);
    fprintf(f, "    %15E", Y);
    fprintf(f, "    %15E", 171.41); //1450. for castor set-up
    fprintf(f, "\n");
    for (int iroc = 0; iroc != 3; ++iroc) {
      fprintf(f, "%2i  %2i ", ChNo[ich-1], iroc);
      fprintf(f, "    %15E", 0.);
      fprintf(f, "                   ");
      fprintf(f, "    %15E", 0.);
      fprintf(f, "    %15E", 0.102 * iroc);
      fprintf(f, "    %15E", 3.77  * iroc);
      fprintf(f, "\n");
    }
    fprintf(f, "\n");
  }



  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [OutFileName]" << std::endl;
    return 1;
  }

  MakeStraightAlignment(argv[1]);

  return 0;
}
