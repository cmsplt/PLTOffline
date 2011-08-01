////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Sun Jun 26 16:37:05 CEST 2011
//
// Compile and run:
//   (setup root first)
//   g++ -Wall `root-config --cflags --libs` RishiCompare.cc -o RishiCompare
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "TRandom.h"
#include "TMath.h"

int RishiCompare ()
{
  // Coordinates of "hits"
  float X[3], Y[3], Z[3];

  // This is the X and Y which are "true".  We'll smear them below
  float TargetPoint[2];

  // Make some events
  for (int ievent = 0; ievent != 100; ++ievent) {
    // Get random number "ideal" for X and Y of hit
    TargetPoint[0] = gRandom->Gaus(0, 20);
    TargetPoint[1] = gRandom->Gaus(0, 20);

    // Set the XYZ for the hit at each plane.  The X and Y are smeared by a little bit
    for (int ipoint = 0; ipoint != 3; ++ipoint) {
      X[ipoint] = TargetPoint[0] + gRandom->Gaus(1, 1.0);
      Y[ipoint] = TargetPoint[1] + gRandom->Gaus(1, 1.0);
      Z[ipoint] = 2.5 * ipoint;
    }

    // Dean's slope and average:
    float const DSlopeX = (X[2] - X[0]) / (Z[2] - Z[0]);
    float const DSlopeY = (Y[2] - Y[0]) / (Z[2] - Z[0]);
    float const AvgX = (X[0] + X[1] + X[2]) / 3.0;
    float const AvgY = (Y[0] + Y[1] + Y[2]) / 3.0;
    float const AvgZ = (Z[0] + Z[1] + Z[2]) / 3.0;

    // Dean's points where it hits each telescope
    float DXT[3], DYT[3], DZT[3];
    for (int ip = 0; ip != 3; ++ip) {
      DXT[ip] = (2.5 * ip - AvgZ) * DSlopeX + AvgX;
      DYT[ip] = (2.5 * ip - AvgZ) * DSlopeY + AvgY;
      DZT[ip] =  2.5 * ip;
    }

    // Dean's residuals
    float DRX[3], DRY[3], DRZ[3];
    for (int ip = 0; ip != 3; ++ip) {
      DRX[ip] = DXT[ip] - X[ip];
      DRY[ip] = DYT[ip] - Y[ip];
      DRZ[ip] = DZT[ip] - Z[ip];
    }


    // Rishi's points where it hits each telescope
    float RXT[3], RYT[3], RZT[3];
    //set positions:
    float x[3]; float y[3];   
    //project everything onto Z=0
    //TVector3 A; TVector3 B; TVector3 Proj;
    //float norm=sqrt(X[0] * X[0] + Y[0] * Y[0]); 
    //just do least squares routine with x-y
    float sumxy = 0;
    float sumxx = 0;
    float sumyy = 0;
    for(int i = 0; i < 3; ++i)
    {
      sumxy = X[i] * Y[i] + sumxy;
      sumxx = X[i] * X[i] + sumxx;
      sumyy = Y[i] * Y[i] + sumyy;
    }
    float a = (AvgY * sumxx - AvgX * sumxy) / (sumxx - 3 * AvgX * AvgX);
    float b = (sumxy - 3 * AvgX * AvgY) / (sumxx - 3 * AvgX * AvgX);
    float sigX = 0;
    float sigY = 0;
    sigX = sqrt((sumxx - 3 * AvgX * AvgX) / 3);
    sigY = sqrt((sumyy - 3 * AvgY * AvgY) / 3);
    for(int i = 0; i < 3; ++i)
    {
      y[i]=a+b * X[i];//expected y positions from X
      //invert the above to get expected X?
      x[i]=(Y[i]-a)/b;
      //std::cout<<" data points "<<X[i]<<", "<<Y[i]<<" calculated points "<<x[i]<<", "<<y[i]<<std::endl;
    }

    for (int ip = 0; ip != 3; ++ip) {
      RXT[ip]=x[ip];
      RYT[ip]=y[ip];
      RZT[ip]=2.5*ip;
    }


    // Compare the two
    for (int ip = 0; ip != 3; ++ip) {
      printf("Deans    XYZ: %12.3f %12.3f %12.3f\n", DXT[ip], DYT[ip], DZT[ip]);
      printf("Deans    Res: %12.3f %12.3f %12.3f\n", DRX[ip], DRY[ip], DRZ[ip]);
      printf("Rishis   XYZ: %12.3f %12.3f %12.3f\n", RXT[ip], RYT[ip], RZT[ip]);
      printf("Rishis sigma: %12.3f %12.3f\n", sigX, sigY);
    }





  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  RishiCompare();

  return 0;
}
