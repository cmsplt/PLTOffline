////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Sun Jun 26 16:16:58 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <stdint.h>

#include "PLTHit.h"
#include "PLTAlignment.h"
#include "PLTU.h"

#include "TRandom.h"


void GetPureNoise (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // Grab list of telescopes
  static std::vector<int> Channels = Alignment.GetListOfChannels();

  int Row, Column;
  for (std::vector<int>::iterator ch = Channels.begin(); ch != Channels.end(); ++ch) {
    for (int iroc = 0; iroc != 3; ++iroc) {
      Column = gRandom->Integer(PLTU::NCOL) + PLTU::FIRSTCOL;
      Row    = gRandom->Integer(PLTU::NROW) + PLTU::FIRSTROW;
      if (Column >= PLTU::FIRSTCOL && Column <= PLTU::LASTCOL && Row >= PLTU::FIRSTROW && Row <= PLTU::LASTROW) {
        Hits.push_back( new PLTHit(*ch, iroc, Column, Row, gRandom->Poisson(150)) );
      } else {
        std::cout << "out of range" << std::endl;
      }
    }
  }


  return;
}


void GetTracksCollisions (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // Grab list of telescopes
  static std::vector<int> Channels = Alignment.GetListOfChannels();



  static int const NTracks = 1;//gRandom->Integer(10);

  for (int itrack = 0; itrack < NTracks; ++itrack) {
    int const Channel = Channels[ gRandom->Integer(Channels.size()) ];
    int const ROC     = gRandom->Integer(3);
    //printf("Channel: %2i  ROC: %i\n", Channel, ROC);

    float const lx = 0.45 * (gRandom->Rndm() - 0.5);
    float const ly = 0.45 * (gRandom->Rndm() - 0.5);
    //printf(" lx ly: %15E  %15E\n", lx, ly);

    static std::vector<float> TXYZ;
    Alignment.LtoTXYZ(TXYZ, lx, ly, Channel, ROC);
    //printf(" TXYZ: %15E  %15E  %15E\n", TXYZ[0], TXYZ[1], TXYZ[2]);

    static std::vector<float> GXYZ;
    Alignment.TtoGXYZ(GXYZ, TXYZ[0], TXYZ[1], TXYZ[2], Channel, ROC);
    //printf(" GXYZ: %15E  %15E  %15E\n", GXYZ[0], GXYZ[1], GXYZ[2]);

    float const SlopeX = GXYZ[0] / GXYZ[2];
    float const SlopeY = GXYZ[1] / GXYZ[2];
    //if (Channel == 3)
    //printf(" Slope X Y: %15E  %15E\n", SlopeX, SlopeY);

    for (size_t iroc = 0; iroc != 3; ++iroc) {
      std::vector<float> VP;
      Alignment.LtoGXYZ(VP, 0, 0, Channel, iroc);
      //printf("  VP XYZ: %15E  %15E  %15E\n", VP[0], VP[1], VP[2]);

      float const GZ = VP[2];
      float const GX = SlopeX * GZ;
      float const GY = SlopeY * GZ;
      //printf("ROC %1i  GXYZ: %15E  %15E  %15E\n", iroc, GX, GY, GZ);


      std::vector<float> T;
      Alignment.GtoTXYZ(T, GX, GY, GZ, Channel, iroc);
      //if (Channel == 3) printf("HI %15E\n", GX - T[0]);
      //if (Channel == 3)
      //printf("ROC %1i TX TY TZ  %15E %15E %15E\n", iroc, T[0], T[1], T[2]);

      std::pair<float, float> LXY = Alignment.TtoLXY(T[0], T[1], Channel, iroc);
      std::pair<int, int>     PXY = Alignment.PXYfromLXY(LXY);

      // Add some jitter
      PXY.first  += (int) gRandom->Gaus(0, 1.2);
      PXY.second += (int) gRandom->Gaus(0, 1.2);
      int const PX = PXY.first;
      int const PY = PXY.second;

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it as a hit if it's in the range of the diamond
      //printf("ROC %1i LX PX LY PY   %15E %2i  %15E %2i\n", iroc, LXY.first, PX, LXY.second, PY);
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
        Hits.push_back( new PLTHit(Channel, iroc, PX, PY, adc) );
      } else {
        //printf("LX PX LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", LXY.first, PX, rXY.second, PY);
      }
    }




  }


  return;
}



void GetTracksCollisions2 (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // Grab list of telescopes
  static std::vector<int> Channels = Alignment.GetListOfChannels();



  static int const NTracks = gRandom->Integer(4);

  for (int itrack = 0; itrack < NTracks; ++itrack) {
    int const Channel = Channels[ gRandom->Integer(Channels.size()) ];
    int const ROC     = gRandom->Integer(3);
    //printf("Channel: %2i  ROC: %i\n", Channel, ROC);

    // pick a starting point on the first ROC
    int const StartCol = gRandom->Integer(PLTU::NCOL) + PLTU::FIRSTCOL;
    int const StartRow = gRandom->Integer(PLTU::NROW) + PLTU::FIRSTROW;

    //printf(" StartCol: %4i  StartRow: %4i\n", StartCol, StartRow);

    float const lx = Alignment.PXtoLX(StartCol);
    float const ly = Alignment.PYtoLY(StartRow);

    //printf(" lx ly: %15E  %15E\n", lx, ly);

    static std::vector<float> TXYZ;
    Alignment.LtoTXYZ(TXYZ, lx, ly, Channel, ROC);
    //printf(" TXYZ: %15E  %15E  %15E\n", TXYZ[0], TXYZ[1], TXYZ[2]);

    static std::vector<float> GXYZ;
    Alignment.TtoGXYZ(GXYZ, TXYZ[0], TXYZ[1], TXYZ[2], Channel, ROC);
    //printf(" GXYZ: %15E  %15E  %15E\n", GXYZ[0], GXYZ[1], GXYZ[2]);

    float const SlopeX = GXYZ[0] / GXYZ[2];
    float const SlopeY = GXYZ[1] / GXYZ[2];
    //if (Channel == 3)
    //printf(" Slope X Y: %15E  %15E\n", SlopeX, SlopeY);

    for (size_t iroc = 0; iroc != 3; ++iroc) {
      std::vector<float> VP;
      Alignment.LtoGXYZ(VP, 0, 0, Channel, iroc);
      //printf("  VP XYZ: %15E  %15E  %15E\n", VP[0], VP[1], VP[2]);

      float const GZ = VP[2];
      float const GX = SlopeX * GZ;
      float const GY = SlopeY * GZ;
      //printf("ROC %1i  GXYZ: %15E  %15E  %15E\n", iroc, GX, GY, GZ);


      std::vector<float> T;
      Alignment.GtoTXYZ(T, GX, GY, GZ, Channel, iroc);
      //if (Channel == 3) printf("HI %15E\n", GX - T[0]);
      //if (Channel == 3)
      //printf("ROC %1i TX TY TZ  %15E %15E %15E\n", iroc, T[0], T[1], T[2]);

      std::pair<float, float> LXY = Alignment.TtoLXY(T[0], T[1], Channel, iroc);
      std::pair<int, int>     PXY = Alignment.PXYfromLXY(LXY);

      // Add some jitter
      PXY.first  += (int) gRandom->Gaus(0, 1.2);
      PXY.second += (int) gRandom->Gaus(0, 1.2);
      int const PX = PXY.first;
      int const PY = PXY.second;

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it as a hit if it's in the range of the diamond
      //printf("ROC %1i LX PX LY PY   %15E %2i  %15E %2i\n", iroc, LXY.first, PX, LXY.second, PY);
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
        Hits.push_back( new PLTHit(Channel, iroc, PX, PY, adc) );
      } else {
        //printf("LX PX LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", LXY.first, PX, rXY.second, PY);
      }
    }




  }


  return;
}



void GetTracksParallel (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // Grab list of telescopes
  static std::vector<int> Channels = Alignment.GetListOfChannels();


  static int const NTracks = 1;//gRandom->Integer(10);

  for (int itrack = 0; itrack < NTracks; ++itrack) {
    int const Channel = Channels[ gRandom->Integer(Channels.size()) ];
    int const ROC     = gRandom->Integer(3);

    //printf("Channel: %2i  ROC: %i\n", Channel, ROC);

    // pick a starting point on the first ROC
    int const StartCol = gRandom->Integer(PLTU::NCOL) + PLTU::FIRSTCOL;
    int const StartRow = gRandom->Integer(PLTU::NROW) + PLTU::FIRSTROW;

    //printf(" StartCol: %4i  StartRow: %4i\n", StartCol, StartRow);

    float const lx = Alignment.PXtoLX(StartCol);
    float const ly = Alignment.PYtoLY(StartRow);

    //printf(" lx ly: %15E  %15E\n", lx, ly);

    static std::vector<float> TXYZ;
    Alignment.LtoTXYZ(TXYZ, lx, ly, Channel, ROC);
    //printf(" TXYZ: %15E  %15E  %15E\n", TXYZ[0], TXYZ[1], TXYZ[2]);

    static std::vector<float> GXYZ;
    Alignment.TtoGXYZ(GXYZ, TXYZ[0], TXYZ[1], TXYZ[2], Channel, ROC);
    float const GX = GXYZ[0];
    float const GY = GXYZ[1];
    float const GZ = GXYZ[2];
    //printf(" GXYZ: %15E  %15E  %15E\n", GXYZ[0], GXYZ[1], GXYZ[2]);

    for (size_t iroc = 0; iroc != 3; ++iroc) {

      std::vector<float> T;
      Alignment.GtoTXYZ(T, GX, GY, GZ, Channel, iroc);

      std::pair<float, float> LXY = Alignment.TtoLXY(T[0], T[1], Channel, iroc);
      std::pair<int, int>     PXY = Alignment.PXYfromLXY(LXY);
      int const PX = PXY.first;
      int const PY = PXY.second;

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it as a hit if it's in the range of the diamond
      //printf("iroc StartCol LX PX StartRow LY PY  %2i  %2i %6.2f %2i   %2i %6.2f %2i\n", iroc, StartCol, LXY.first, PX, StartRow, LXY.second, PY);
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
        Hits.push_back( new PLTHit(Channel, iroc, PX, PY, adc) );
      } else {
        //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LXY.first, PX, StartRow, LXY.second, PY);
      }
    }




  }


  return;
}
void GetTracksRandomSlope (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // This function to generate events hitting telescopes head on

  int const NTelescopes = 1;
  for (int i = 1; i <= NTelescopes; ++i) {

    int const NTracks = 1;//gRandom->Integer(10);

    for (int itrack = 0; itrack < NTracks; ++itrack) {

      // pick a starting point on the first ROC
      int const StartCol = gRandom->Integer(PLTU::NCOL) + PLTU::FIRSTCOL;
      int const StartRow = gRandom->Integer(PLTU::NROW) + PLTU::FIRSTROW;

      float const SlopeX = 9.0 * (gRandom->Rndm() - 0.5);
      float const SlopeY = 9.0 * (gRandom->Rndm() - 0.5);


      for (int r = 0; r != 3; ++r) {
        //PLTAlignment::CP* C = Alignment.GetCP(i, r);

        // make straight track, see where that hits a plane if it's shifted..
        // Optionally give it some fuzz..

        // Use L coord system:
        // THINK ABOUT THIS FOR ROTATIONS...
        float const LZ = Alignment.LZ(i, r);
        float const LX = Alignment.PXtoLX(StartCol + SlopeX * LZ);
        float const LY = Alignment.PYtoLY(StartRow + SlopeY * LZ);

        std::pair<float, float> LXY = Alignment.TtoLXY(LX, LY, i, r);

        int const PX = Alignment.PXfromLX(LXY.first);
        int const PY = Alignment.PYfromLY(LXY.second);


        //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i    CLX CLY: %12.3f %12.3f\n", StartCol, LX, PX, StartRow, LY, PY, C->LX, C->LY);

        // Just some random adc value
        int const adc = gRandom->Poisson(150);

        // Add it as a hit if it's in the range of the diamond
        if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
          Hits.push_back( new PLTHit(i, r, PX, PY, adc) );
        } else {
          //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);
        }
      }
    }
  }
  return;
}
void GetTracksHeadOnFirstROCMultiTracks (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // This function to generate events hitting telescopes head on

  int const NTelescopes = 1;
  for (int i = 1; i <= NTelescopes; ++i) {

    int const NTracks = gRandom->Integer(10);

    for (int itrack = 0; itrack < NTracks; ++itrack) {

      // pick a starting point on the first ROC
      int const StartCol = gRandom->Integer(PLTU::NCOL) + PLTU::FIRSTCOL;
      int const StartRow = gRandom->Integer(PLTU::NROW) + PLTU::FIRSTROW;

      float const SlopeX = 9.0 * (gRandom->Rndm() - 0.5);
      float const SlopeY = 9.0 * (gRandom->Rndm() - 0.5);


      for (int r = 0; r != 3; ++r) {
        //PLTAlignment::CP* C = Alignment.GetCP(i, r);

        // make straight track, see where that hits a plane if it's shifted..
        // Optionally give it some fuzz..

        // Use L coord system:
        // THINK ABOUT THIS FOR ROTATIONS...
        float const LZ = Alignment.LZ(i, r);
        float const LX = Alignment.PXtoLX(StartCol + SlopeX * LZ);
        float const LY = Alignment.PYtoLY(StartRow + SlopeY * LZ);

        std::pair<float, float> LXY = Alignment.TtoLXY(LX, LY, i, r);

        int const PX = Alignment.PXfromLX(LXY.first);
        int const PY = Alignment.PYfromLY(LXY.second);


        //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i    CLX CLY: %12.3f %12.3f\n", StartCol, LX, PX, StartRow, LY, PY, C->LX, C->LY);

        // Just some random adc value
        int const adc = gRandom->Poisson(150);

        // Add it as a hit if it's in the range of the diamond
        if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
          Hits.push_back( new PLTHit(i, r, PX, PY, adc) );
        } else {
          //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);
        }
      }
    }
  }
  return;
}




void GetRandTracksROCEfficiencies (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment, float const Eff0, float const Eff1, float const Eff2)
{
  // This function to generate events hitting telescopes head on

  float const Eff[3] = { Eff0, Eff1, Eff2 };

  int const NTelescopes = 36;
  //std::cout << "Event" << std::endl;
  for (int i = 1; i <= NTelescopes; ++i) {

    // pick a starting point.  +/- 10 should cover shifts in alignment
    int const StartCol = gRandom->Integer(PLTU::NCOL + 20) + PLTU::FIRSTCOL - 10;
    int const StartRow = gRandom->Integer(PLTU::NROW + 20) + PLTU::FIRSTROW - 10;

    float const SlopeX = 2.0 * (gRandom->Rndm() - 0.5);
    float const SlopeY = 2.0 * (gRandom->Rndm() - 0.5);


    for (int r = 0; r != 3; ++r) {
      //PLTAlignment::CP* C = Alignment.GetCP(i, r);

      // make straight track, see where that hits a plane if it's shifted..
      // Optionally give it some fuzz..

      // Use L coord system:
      // THINK ABOUT THIS FOR ROTATIONS...
      float const LZ = Alignment.LZ(i, r);
      float const LX = Alignment.PXtoLX(StartCol + SlopeX * LZ);
      float const LY = Alignment.PYtoLY(StartRow + SlopeY * LZ);

      std::pair<float, float> LXY = Alignment.TtoLXY(LX, LY, i, r);

      int const PX = Alignment.PXfromLX(LXY.first);
      int const PY = Alignment.PYfromLY(LXY.second);


      //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i    CLX CLY: %12.3f %12.3f\n", StartCol, LX, PX, StartRow, LY, PY, C->LX, C->LY);

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it as a hit if it's in the range of the diamond
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
        //printf("Ch ROC PX PY %2i %1i %3i %3i\n", i, r, PX, PY);
        if (gRandom->Rndm() < Eff[r]) {
          Hits.push_back( new PLTHit(i, r, PX, PY, adc) );
        }
      } else {
        //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);
      }
    }
  }
  return;
}



void GetTracksROCEfficiencies (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment, float const Eff0, float const Eff1, float const Eff2)
{
  // This function to generate events hitting telescopes head on

  float const Eff[3] = { Eff0, Eff1, Eff2 };

  int const NTelescopes = 1;
  for (int i = 1; i <= NTelescopes; ++i) {

    // pick a starting point.  +/- 10 should cover shifts in alignment
    int const StartCol = gRandom->Integer(PLTU::NCOL - 20) + PLTU::FIRSTCOL + 10;
    int const StartRow = gRandom->Integer(PLTU::NROW - 20) + PLTU::FIRSTROW + 10;


    for (int r = 0; r != 3; ++r) {
      //PLTAlignment::CP* C = Alignment.GetCP(i, r);

      // make straight track, see where that hits a plane if it's shifted..
      // Optionally give it some fuzz..

      // Use L coord system:
      // THINK ABOUT THIS FOR ROTATIONS...
      float const LX = Alignment.PXtoLX(StartCol);
      float const LY = Alignment.PYtoLY(StartRow);

      std::pair<float, float> LXY = Alignment.TtoLXY(LX, LY, i, r);

      int const PX = Alignment.PXfromLX(LXY.first);
      int const PY = Alignment.PYfromLY(LXY.second);


      //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i    CLX CLY: %12.3f %12.3f\n", StartCol, LX, PX, StartRow, LY, PY, C->LX, C->LY);

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it as a hit if it's in the range of the diamond
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
        if (gRandom->Rndm() < Eff[r]) {
          Hits.push_back( new PLTHit(i, r, PX, PY, adc) );
        }
      } else {
        //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);
      }
    }
  }
  return;
}



void GetSpecificClusters (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // This function to generate events hitting telescopes head on

  int const NTelescopes = 1;
  for (int i = 1; i <= NTelescopes; ++i) {

    // pick a starting point.  +/- 10 should cover shifts in alignment
    int const StartCol = gRandom->Integer(10) + PLTU::FIRSTCOL + 5;
    int const StartRow = gRandom->Integer(10) + PLTU::FIRSTROW + 5;


    for (int r = 0; r != 3; ++r) {
      //PLTAlignment::CP* C = Alignment.GetCP(i, r);

      // make straight track, see where that hits a plane if it's shifted..
      // Optionally give it some fuzz..

      // Use L coord system:
      // THINK ABOUT THIS FOR ROTATIONS...
      float const LX = Alignment.PXtoLX(StartCol);
      float const LY = Alignment.PYtoLY(StartRow);

      std::pair<float, float> LXY = Alignment.TtoLXY(LX, LY, i, r);

      int const PX = Alignment.PXfromLX(LXY.first);
      int const PY = Alignment.PYfromLY(LXY.second);


      //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i    CLX CLY: %12.3f %12.3f\n", StartCol, LX, PX, StartRow, LY, PY, C->LX, C->LY);

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it
      Hits.push_back( new PLTHit(i, r, PX, PY, adc) );
      Hits.push_back( new PLTHit(i, r, PX+1, PY, adc) );
    }
  }
  return;
}

void GetTracksParallelGaus (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // This function to generate events hitting telescopes head on

  // Define the offset in XY and the widths in XY
  float const ColOffset = 0.0;
  float const RowOffset = 0.0;
  float const ColWidth = 12;
  float const RowWidth = 12;

  int const NTelescopes = 1;
  for (int i = 1; i <= NTelescopes; ++i) {

    // pick a starting point with width and offset
    int const StartCol = gRandom->Gaus( (PLTU::FIRSTCOL + PLTU::LASTCOL + 1.0) / 2.0 + ColOffset, ColWidth);
    int const StartRow = gRandom->Gaus( (PLTU::FIRSTROW + PLTU::LASTROW + 1.0) / 2.0 + RowOffset, RowWidth);



    for (int r = 0; r != 3; ++r) {
      // make straight track, see where that hits a plane if it's shifted..
      // Optionally give it some fuzz..

      // Use L coord system:
      // THINK ABOUT THIS FOR ROTATIONS...
      float const LX = Alignment.PXtoLX(StartCol);
      float const LY = Alignment.PYtoLY(StartRow);

      std::pair<float, float> LXY = Alignment.TtoLXY(LX, LY, i, r);

      int const PX = Alignment.PXfromLX(LXY.first);
      int const PY = Alignment.PYfromLY(LXY.second);


      //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it as a hit if it's in the range of the diamond
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
        Hits.push_back( new PLTHit(i, r, PX, PY, adc) );
      } else {
        //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);
      }
    }
  }
  return;
}



void GetTracksHeadOn (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // This function to generate events hitting telescopes head on

  int const NTelescopes = 1;
  for (int i = 1; i <= NTelescopes; ++i) {

    // pick a starting point.  +/- 10 should cover shifts in alignment
    int const StartCol = gRandom->Integer(PLTU::NCOL + 20) + PLTU::FIRSTCOL - 10;
    int const StartRow = gRandom->Integer(PLTU::NROW + 20) + PLTU::FIRSTROW - 10;


    for (int r = 0; r != 3; ++r) {
      //PLTAlignment::CP* C = Alignment.GetCP(i, r);

      // make straight track, see where that hits a plane if it's shifted..
      // Optionally give it some fuzz..

      // Use L coord system:
      // THINK ABOUT THIS FOR ROTATIONS...
      float const LX = Alignment.PXtoLX(StartCol);
      float const LY = Alignment.PYtoLY(StartRow);
      //printf("LY  %.13E\n", LY);

      std::pair<float, float> LXY = Alignment.TtoLXY(LX, LY, i, r);
      //printf("LXY %.13E\n", LXY.second);

      int const PX = Alignment.PXfromLX(LXY.first);
      int const PY = Alignment.PYfromLY(LXY.second);

      //std::cout << "PY " << PY << std::endl;

      //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i    CLX CLY: %12.3f %12.3f\n", StartCol, LX, PX, StartRow, LY, PY, C->LX, C->LY);

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it as a hit if it's in the range of the diamond
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
        Hits.push_back( new PLTHit(i, r, PX, PY, adc) );
      } else {
        //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);
      }
    }
  }
  return;
}



void GetTracksHeadOnFirstROC (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // This function to generate events hitting telescopes head on

  for (int i = 15; i <= 15; ++i) {

    // pick a starting point on the first ROC
    int const StartCol = gRandom->Integer(PLTU::NCOL) + PLTU::FIRSTCOL;
    int const StartRow = gRandom->Integer(PLTU::NROW) + PLTU::FIRSTROW;


    for (int r = 0; r != 3; ++r) {
      //PLTAlignment::CP* C = Alignment.GetCP(i, r);

      // make straight track, see where that hits a plane if it's shifted..
      // Optionally give it some fuzz..

      // Use L coord system:
      // THINK ABOUT THIS FOR ROTATIONS...
      float const LX = Alignment.PXtoLX(StartCol);
      float const LY = Alignment.PYtoLY(StartRow);

      std::pair<float, float> LXY = Alignment.TtoLXY(LX, LY, i, r);

      int const PX = Alignment.PXfromLX(LXY.first);
      int const PY = Alignment.PYfromLY(LXY.second);


      //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      // Add it as a hit if it's in the range of the diamond
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
        Hits.push_back( new PLTHit(i, r, PX, PY, adc) );
      } else {
        //printf("StartCol LX PX StartRow LY PY   %2i %6.2f %2i   %2i %6.2f %2i\n", StartCol, LX, PX, StartRow, LY, PY);
      }
    }
  }
  return;
}

void GetGausHitsOneROC (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment)
{
  // This function to generate events hitting telescopes head on

  int const Channel = 1;

  float const ColOffset = 0;
  float const RowOffset = 0;
  float const ColWidth = 20;
  float const RowWidth = 30;

  // pick a starting point on the first ROC
  int const PX = gRandom->Gaus( ((float) (PLTU::FIRSTCOL + PLTU::LASTCOL) + 1.0) / 2.0 + ColOffset, ColWidth);
  int const PY = gRandom->Gaus( ((float) (PLTU::FIRSTROW + PLTU::LASTROW) + 1.0) / 2.0 + RowOffset, RowWidth);


  int const ROC = 0;



  // Just some random adc value
  int const adc = gRandom->Poisson(150);

  // Add it as a hit if it's in the range of the diamond
  if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
    Hits.push_back( new PLTHit(Channel, ROC, PX, PY, adc) );
  } else {
    //printf("PX PY   %2i  %2i\n", PX, PY);
  }
  return;
}

int PLTMC ()
{
  // Open the output file
  std::ofstream fout("PLTMC.dat", std::ios::binary);
  if (!fout.is_open()) {
    std::cerr << "ERROR: cannot open output file" << std::endl;
    throw;
  }

  uint32_t unsigned n;
  uint32_t unsigned n2;

  PLTAlignment Alignment;
  //Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_IdealCastor.dat");
  //Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_IdealCastor.dat");
  //Alignment.ReadAlignmentFile("straight");
  //Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_Straight.dat");
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_PLTMC.dat");

  // Vector of hits for each event
  std::vector<PLTHit*> Hits;
  int const NEvents = 250000;
  for (int ievent = 0; ievent != NEvents; ++ievent) {

    if (ievent % 10000 == 0) {
      printf("ievent = %12i\n", ievent);
    }

    switch (3) {
      case 0:
        GetTracksCollisions(Hits, Alignment);
        break;
      case 1:
        GetTracksHeadOnFirstROC(Hits, Alignment);
        break;
      case 2:
        GetTracksHeadOn(Hits, Alignment);
        break;
      case 3:
        GetTracksParallelGaus(Hits, Alignment);
        break;
      case 4:
        GetSpecificClusters(Hits, Alignment);
        break;
      case 5:
        GetTracksROCEfficiencies(Hits, Alignment, 0.20, 0.80, 0.90);
        break;
      case 6:
        GetRandTracksROCEfficiencies(Hits, Alignment, 0.20, 0.80, 0.90);
        break;
      case 7:
        GetTracksHeadOnFirstROCMultiTracks(Hits, Alignment);
        break;
      case 8:
        GetTracksRandomSlope(Hits, Alignment);
        break;
      case 9:
        GetTracksParallel(Hits, Alignment);
        break;
      case 10:
        GetPureNoise(Hits, Alignment);
        break;
      case 11:
        GetTracksCollisions2(Hits, Alignment);
      case 12:
        GetGausHitsOneROC(Hits, Alignment);
	break;
    }


    // Delete all hits and clear vector
    n2 = (5 << 8);
    n =  0x50000000;
    n |= ievent;
    fout.write( (char*) &n2, sizeof(uint32_t) );
    fout.write( (char*) &n, sizeof(uint32_t) );


    for (size_t ihit = 0; ihit != Hits.size(); ++ihit) {
      n = 0x00000000;
      PLTHit* Hit = Hits[ihit];
      //printf("Channel ROC Row Col ADC: %2i %1i %2i %2i %4i %12i\n", Hit->Channel(), Hit->ROC(), Hit->Row(), Hit->Column(), Hit->ADC(), ievent);

      n |= (Hit->Channel() << 26);
      n |= ( (Hit->ROC() + 1) << 21);

      if (Hit->Column() % 2 == 0) {
        n |= ( ((80 - Hit->Row()) * 2) << 8 );
      } else {
        // checked, correct
        n |= ( ((80 - Hit->Row()) * 2 + 1) << 8 );
      }

      if (Hit->Column() % 2 == 0) {
        n |= ( ((Hit->Column()) / 2) << 16  );
      } else {
        n |= (( (Hit->Column() - 1) / 2) << 16  );
      }
      n |= Hit->ADC();


      //if (Hit->ROC() == 2) {
      //  printf("WORD: %X\n", (n &  0x3e00000));
      //}

      fout.write( (char*) &n, sizeof(uint32_t) );
      delete Hits[ihit];
    }

    if (Hits.size() % 2 == 0) {
      // even number of hits.. need to fill out the word.. just print the number over two as 2x32  words
      n  = 0xa0000000;
      n2 = 0x00000000;
      n  |= (Hits.size() / 2 + 2);
      fout.write( (char*) &n2, sizeof(uint32_t) );
      fout.write( (char*) &n, sizeof(uint32_t) );
    } else {
      // Print number of hits in 1x32 bit
      n  = 0x00000000;
      fout.write( (char*) &n, sizeof(uint32_t) );
      fout.write( (char*) &n, sizeof(uint32_t) );
      n  = 0xa0000000;
      n  |= (Hits.size() / 2 + 1);
      fout.write( (char*) &n, sizeof(uint32_t) );
    }


    Hits.clear();
  }

  fout.close();

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " " << std::endl;
    return 1;
  }

  PLTMC();

  return 0;
}
