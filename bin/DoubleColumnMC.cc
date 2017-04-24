/////////////////////////////////////////////////////////////////////////////
// 
// DoubleColumnMC.cc: MonteCarlo simulation of Tracks to measure and simulate
//   dead-time effects from double-column inefficiencies. Based on PLTMC.cc.
// 
// Daniel Gift, July 2016
// 
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <set>
#include <iomanip>

#include "PLTEvent.h"
#include "PLTHit.h"
#include "PLTAlignment.h"
#include "PLTU.h"

#include "TRandom.h"

// Parameters of the channels
static const int ROWSTART = 0;
static const int ROWSTOP = 79;
static const int COLSTART = 0;
static const int COLSTOP = 51;

//Global variables to keep track of drain queues
static const int clockCycles = 6; //Number of events before a DCOL becomes functional again
static const int bufferSize = 3; //Number of hits that can be kept during a drain
static const float TracksPerChannel = 0.3; //Average tracks/event/channel
std::string outputFileName = "DCOL.dat"; //Output file name
//Define the types necessary to construct a 4-d vector to track the DCOL drain queues
typedef std::vector<std::vector<int> > vv;
typedef std::vector<vv> vvv;
std::vector<vvv> DCOLhistory(24, vvv(3, vv(26, std::vector<int>(bufferSize))));

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
  bool success = true;
  
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
      success = false;
    }
    
    // Convert from mFEC and mFECchannel to channel number
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
  int NTracks = gRandom->Poisson(16.*TracksPerChannel);

  // Generate the tracks
  for (int itrack = 0; itrack < NTracks; ++itrack) {
    int Channel = Channels[ gRandom->Integer(Channels.size()) ];
    int ROC     = gRandom->Integer(3);
    //printf("Channel: %2i  ROC: %i\n", Channel, ROC);

    float lx = 0.45 * (gRandom->Rndm() - 0.5);
    float ly = 0.45 * (gRandom->Rndm() - 0.5);
    //printf(" lx ly: %15E  %15E\n", lx, ly);

    std::vector<float> TXYZ;
    Alignment.LtoTXYZ(TXYZ, lx, ly, Channel, ROC);
    //printf(" TXYZ: %15E  %15E  %15E\n", TXYZ[0], TXYZ[1], TXYZ[2]);

    std::vector<float> GXYZ;
    Alignment.TtoGXYZ(GXYZ, TXYZ[0], TXYZ[1], TXYZ[2], Channel, ROC);
    //printf(" GXYZ: %15E  %15E  %15E\n", GXYZ[0], GXYZ[1], GXYZ[2]);
    float SlopeX = GXYZ[0] / GXYZ[2];
    float SlopeY = GXYZ[1] / GXYZ[2];
    //if (Channel == 3)                                                                                                        
    //printf(" Slope X Y: %15E  %15E\n", SlopeX, SlopeY);
    for (size_t iroc = 0; iroc != 3; ++iroc) {
      
  
      std::vector<float> VP;
      Alignment.LtoGXYZ(VP, 0, 0, Channel, iroc);
      //printf("  VP XYZ: %15E  %15E  %15E\n", VP[0], 

      float GZ = VP[2];
      float GX = SlopeX * GZ;
      float GY = SlopeY * GZ;
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
      int PX = PXY.first;
      int PY = PXY.second;

      // Just some random adc value
      int adc = gRandom->Poisson(150);

      // Check if it's in the valid range
      int id = Channel*100000 + iroc*10000 + PX*100 + PY;
      // Add it as a hit if it's in the range  
      //printf("ROC %1i LX PX LY PY   %15E %2i  %15E %2i\n", iroc, LXY.first, PX, LX
      if (PX >= PLTU::FIRSTCOL && PX <= PLTU::LASTCOL && 
	  PY >= PLTU::FIRSTROW && PY <= PLTU::LASTROW) {
	PLTHit* tempHit = new PLTHit(Channel, iroc, PX, PY, adc);
	bool blocked = true;
	int dcol = PX/2;
	//Check what's in the queue for the relevant double column
	for (std::vector<int>::iterator it = DCOLhistory[Channel][iroc][dcol].begin(); it != DCOLhistory[Channel][iroc][dcol].end(); ++it) {
	  // If there's an event that's old enough, replace it with this event
	  if (count - (*it) > clockCycles) { 
	    *it = count;
	    blocked = false;
	    break;
	  }
	}

	//If we found a place in the queue for the event, then we're good
	if (!blocked) {
	  //Check if in a good region of the telescope
	  if (!BadRegion.count(id)) {
	    Hits.push_back(tempHit);
	  }   
	}
	//If we didn't find a place, then it gets ignored
      } 
    } //roc loop
  } // Track loop    
  return;
}

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

  PLTAlignment Alignment;
  //Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_PLTMC.dat");
  // Alignment.ReadAlignmentFile("ALIGNMENT/Alignment_IdealInstall.dat");
  Alignment.ReadAlignmentFile("ALIGNMENT/Trans_Alignment_4892.dat");

  std::set<int> BadRegion = ReadPixelMaskFile("Mask_May2016_v1.txt");
  
  // Vector of hits for each event
  std::vector<PLTHit*> Hits;

  //std::vector<PLTHit*> AllHits;
  int const NEvents = 100000;
  for (int ievent = 1; ievent <= NEvents; ++ievent) {
    if (ievent % 100 == 0) {
      printf("ievent = %12i\n", ievent);
    }
    
    GetTracksCollisions(Hits, Alignment, BadRegion, ievent);
    // std::cout<<"Hits: "<<Hits.size()<<std::endl;
    //  std::cout<<"AllHits: "<<AllHits.size()<<std::endl;
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
  //Set the values in the drain queue to be more clock cyles less than zero than
  // the number of clock cycles needed to drain
  // Basically set them so it doesn't look like there were events at time 0 in everything 
  static const int defaultStart = -clockCycles - 1;
  for (std::vector<vvv>::iterator it = DCOLhistory.begin(); it != DCOLhistory.end(); ++it) {
    for (std::vector<vv>::iterator jt = (*it).begin(); jt != (*it).end(); ++jt) {
      for (std::vector<std::vector<int> >::iterator kt = (*jt).begin(); kt != (*jt).end(); ++kt) {
	for (std::vector<int>::iterator lt = (*kt).begin(); lt != (*kt).end(); ++lt) {
	  (*lt) = defaultStart;
	}
      }
    }
  }
  PLTMC();

  return 0;
}


