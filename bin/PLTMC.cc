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


void GetTracksCollisions (std::vector<PLTHit*>& Hits)
{
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
          Hits.push_back( new PLTHit(i+14, r, PX, PY, adc) );
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

  int const NTelescopes = 1;
  for (int i = 1; i <= NTelescopes; ++i) {

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


int PLTMC ()
{
  // Open the output file
  std::ofstream fout("data/PLTMC.dat", std::ios::binary);
  if (!fout.is_open()) {
    std::cerr << "ERROR: cannot open output file" << std::endl;
    throw;
  }

  uint32_t unsigned n;
  uint32_t unsigned n2;

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_PLTMC.dat");
  //Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_Straight.dat");

  // Vector of hits for each event
  std::vector<PLTHit*> Hits;
  int const NEvents = 10000;
  for (int ievent = 0; ievent != NEvents; ++ievent) {

    if (ievent % 10000 == 0) {
      printf("ievent = %12i\n", ievent);
    }

    switch (7) {
      case 0:
        GetTracksCollisions(Hits);
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
    }


    // Delete all hits and clear vector
    n2 = 0x0;
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
