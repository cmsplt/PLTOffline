/////////////////////////////////////////////////////////////////////////////
// 
// TrackMC.cc: MonteCarlo simulation of Tracks to measure and simulate
//   pixel dead-time effects. Based on PLTMC.cc.
// 
// Daniel Gift, July 2016
// 
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <set>
#include <limits>

#include "PLTHit.h"
#include "PLTAlignment.h"
#include "PLTU.h"

#include "TRandom.h"

// Parameters of the channels
static const int ROWSTART = 0;
static const int ROWSTOP = 79;
static const int COLSTART = 0;
static const int COLSTOP = 51;

//Global variables to keep track of previous hits
std::vector<std::pair<int, PLTHit*> > AllHits; //list of when previous hits have happened
static const int recoveryDelay = 6; //Number of events before pixel recovers
static const float TracksPerChannel = 0.3; //Average tracks/event/channel
std::string outputFileName = "TrackMC_30_6q.dat"; //Output file name
static int const NEvents = 100000; //Number of MC events

// Function declaration
// Reads pixel mask. Takes as input the name of the mask file. 
std::set<int> ReadPixelMaskFile(const char*);
// Generate the events. Takes as input a pointer to the array where the hits generated shoudl be stored, 
//  A pointer to the Alignment object, the set of bad pixels from ReadPixelMaskFile, and a count of how 
//  many MC events have been generated so far (a crude version of a clock)
void GetTracksCollisions (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment, std::set<int> BadRegion, int count);
// Runs the Monte Carlo and writes the data to file
int PLTMC ();

//Function to read the pixel mask.
std::set<int> ReadPixelMaskFile(const char* maskFileName) {
  
  std::ifstream MaskFile(maskFileName);
  std::set<int> fPixelMask;
  if (!MaskFile.is_open()) {
    std::cerr << "ERROR: cannot open MaskFile: " << maskFileName << std::endl;
    return fPixelMask;
  }
  
  // So far so good, but if not flag this
  //bool success = true;
  
  int mFec, mFecChannel, hubAddress, roc, maskVal;
  std::string colString, rowString;
  while (!MaskFile.eof()) {
    // Look for and skip comments and blank lines. This is a rather crude method of doing
    // things -- we really should just have a proper parser.
    int nextChar = MaskFile.peek();
    while (nextChar == '#' || nextChar == '\n') {
      // discard line
      MaskFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      nextChar = MaskFile.peek();
    }

    MaskFile >> mFec >> mFecChannel >> hubAddress >> roc >> colString >> rowString >> maskVal;

    if (MaskFile.eof()) {
      break;
    }

    int firstCol, lastCol, firstRow, lastRow;

    // check to see if we want a range of columns or just one column
    size_t cdash = colString.find('-');
    if (cdash == std::string::npos) {
      firstCol = atoi(colString.c_str());
      lastCol = atoi(colString.c_str());
    } else {
      firstCol = atoi(colString.c_str());
      lastCol = atoi(colString.substr(cdash+1).c_str());
    }

    // same, for rows
    size_t rdash = rowString.find('-');
    if (rdash == std::string::npos) {
      firstRow = atoi(rowString.c_str());
      lastRow = atoi(rowString.c_str());
    } else {
      firstRow = atoi(rowString.c_str());
      lastRow = atoi(rowString.substr(rdash+1).c_str());
    }

    // Check for validity
    if (firstRow < ROWSTART || firstCol < COLSTART || lastRow > ROWSTOP || lastCol > COLSTOP) {
      std::cout<<"Row or column numbers are out of range; this line will be skipped"<<std::endl;
      //success = false;
    }
    
    //Convert mFec and mFecChannel to channel number
    int num1 = 0, num2 = 0, num3 = 0;
    if (mFec == 7)
      num1 = 12;
    if (mFecChannel == 2) 
      num2 = 6;
    num3 = hubAddress/8;
    if (num3 <= 1)
      num3 += 1;
    else
      num3 += 2;
    int ch = num1 + num2 + num3;
	
    //Append invalid pixels to an array
    for (int col = firstCol; col <= lastCol; ++col) {
      for (int row = firstRow; row <= lastRow; ++row) {
	fPixelMask.insert( ch*100000 + roc*10000 + col*100 + row );	
      }
    } // column & row loops
  } // input file read loop
  return fPixelMask;
}


//Function to generate tracks
void GetTracksCollisions (std::vector<PLTHit*>& Hits, PLTAlignment& Alignment, std::set<int> BadRegion, int count)
{
  // Grab list of telescopes
  static std::vector<int> Channels = Alignment.GetListOfChannels();
  // Number of tracks is randomly drawn from a Poisson distribution around the value set above.
  // Multiply by 16 because there are 16 channels
  int const NTracks = gRandom->Poisson(16.*TracksPerChannel);

  // Generate the tracks
  for (int itrack = 0; itrack < NTracks; ++itrack) {
    int const Channel = Channels[ gRandom->Integer(Channels.size()) ];
    int const ROC     = gRandom->Integer(3);
    //printf("Channel: %2i  ROC: %i\n", Channel, ROC);

    float const lx = 0.45 * (gRandom->Rndm() - 0.5);
    float const ly = 0.45 * (gRandom->Rndm() - 0.5);
    //printf(" lx ly: %15E  %15E\n", lx, ly);

    std::vector<float> TXYZ;
    Alignment.LtoTXYZ(TXYZ, lx, ly, Channel, ROC);
    //printf(" TXYZ: %15E  %15E  %15E\n", TXYZ[0], TXYZ[1], TXYZ[2]);

    std::vector<float> GXYZ;
    Alignment.TtoGXYZ(GXYZ, TXYZ[0], TXYZ[1], TXYZ[2], Channel, ROC);
    //printf(" GXYZ: %15E  %15E  %15E\n", GXYZ[0], GXYZ[1], GXYZ[2]);
    float const SlopeX = GXYZ[0] / GXYZ[2];
    float const SlopeY = GXYZ[1] / GXYZ[2];
    //if (Channel == 3)                                                                                                        
    //printf(" Slope X Y: %15E  %15E\n", SlopeX, SlopeY);
    for (size_t iroc = 0; iroc != 3; ++iroc) {
      
  
      std::vector<float> VP;
      Alignment.LtoGXYZ(VP, 0, 0, Channel, iroc);
      //printf("  VP XYZ: %15E  %15E  %15E\n", VP[0], 

      float const GZ = VP[2];
      float const GX = SlopeX * GZ;
      float const GY = SlopeY * GZ;
      //printf("ROC %1i  GXYZ: %15E  %15E  %15E\n", iroc, GX, GY, GZ);


      std::vector<float> T;
      Alignment.GtoTXYZ(T, GX, GY, GZ, Channel, iroc);
      //if (Channel == 3) printf("HI %15E\n", GX - T[0]);
      //if (Channel == 3)
      //printf("ROC %1i TX TY TZ  %15E %15E %15E\n", iroc, T[0], T[1], T[2];
      std::pair<float, float> LXY = Alignment.TtoLXY(T[0], T[1], Channel, iroc);
      std::pair<int, int>     PXY = Alignment.PXYfromLXY(LXY);

      // Add some jitter if you want
      PXY.first  += (int) gRandom->Gaus(0, 1.2);
      PXY.second += (int) gRandom->Gaus(0, 1.2);
      int const PX = PXY.first;
      int const PY = PXY.second;

      // Just some random adc value
      int const adc = gRandom->Poisson(150);

      int id = Channel*100000 + iroc*10000 + PX*100 + PY;
      //printf("ROC %1i LX PX LY PY   %15E %2i  %15E %2i\n", iroc, LXY.first, PX, LX
      
      // Check if it's in the valid range  
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
	PLTHit* tempHit = new PLTHit(Channel, iroc, PX, PY, adc);
	bool found = false;
	//Check if there's a recent hit in the same pixel
	for(std::vector<std::pair<int, PLTHit*> >::iterator it = AllHits.begin(); it != AllHits.end();) {
	  // Check if the count difference between a hit and this hit is greater than your dead time
	  // If so, delete that hit from the array
	  if (count - ((*it).first) > recoveryDelay) {
	    it = AllHits.erase(it);
	  }
	  // If not, check if the hits are in the same pixel
	  else {
	    if ((((*it).second)->MatchesColumnRow(tempHit)) && 
		(((*it).second)->Channel() == tempHit->Channel()) && 
		(((*it).second)->ROC() == tempHit->ROC())) {
	      found = true;
	      //(*it).first = count;
	      break;
	    }
	    ++it;
	  }
	} 
	// If we had a valid hit that should be added (found == false), add it
	if (!BadRegion.count(id)) {
	  if (!found) {
	    Hits.push_back(tempHit);
	    std::pair<int, PLTHit*> newHit = std::make_pair (count, new PLTHit(Channel, iroc, PX, PY, adc));
	    AllHits.push_back(newHit);
	  }   
	}
      }
    } //roc loop
  } // Track loop    
  return;
}

// Function to actually generate teh Monte Carlo and to write the file.
int PLTMC ()
{
  // Open the output file
  std::ofstream fout(outputFileName.c_str(), std::ios::binary);
  if (!fout.is_open()) {
    std::cerr << "ERROR: cannot open output file" << std::endl;
    throw;
  }
  
  uint32_t  n;
  uint32_t  n2;

  // Read the alignment file
  PLTAlignment Alignment;
  //Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_PLTMC.dat");
  // Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_IdealInstall.dat");
  Alignment.ReadAlignmentFile("ALIGNMENT/Trans_Alignment_4892.dat");

  // Read Pixel Mask
  std::set<int> BadRegion = ReadPixelMaskFile("Mask_May2016_v1.txt");
  
  // Vector of hits for each event
  std::vector<PLTHit*> Hits;

  for (int ievent = 1; ievent <= NEvents; ++ievent) {
    if (ievent % 100 == 0) {
      printf("ievent = %12i\n", ievent);
    }
    
    GetTracksCollisions(Hits, Alignment, BadRegion, ievent);
    // Delete all hits and clear vector
    n2 = (5 << 8);
    n =  0x50000000;
    n |= ievent;
    n2 |= ((ievent%3564) << 20);
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
      n2 = ievent/3;
      fout.write( (char*) &n2, sizeof(uint32_t) );
      fout.write( (char*) &n, sizeof(uint32_t) );
    } else {
      // Print number of hits in 1x32 bit
      n  = 0x00000000;
      n = ievent/3;
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


