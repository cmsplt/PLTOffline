////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon May 23 09:57:45 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

int convPXL (int ROC)
{
  return ROC % 2 ? 80 - (ROC - 1) / 2 : (ROC - 1) / 2 - 80;
}

void decodeSpyDataFifo (unsigned long word, int event)
{

  if (word & 0xfffffff) {

    const unsigned long int plsmsk = 0xff;
    const unsigned long int pxlmsk = 0xff00;
    const unsigned long int dclmsk = 0x1f0000;
    const unsigned long int rocmsk = 0x3e00000;
    const unsigned long int chnlmsk = 0xfc000000;
    unsigned long int chan = ((word & chnlmsk) >> 26);
    unsigned long int roc  = ((word & rocmsk)  >> 21);

    // Check for embeded special words: roc > 25 is special, not a hit
    if (roc > 25) {
      if ((word & 0xffffffff) == 0xffffffff) {
      } else if (roc == 26) {
      } else if (roc == 27) {
      } else {
        //decodeErrorFifo(word);
      }
    } else if (chan > 0 && chan < 37) {
      // Oh, NOW we have a hit!
      int mycol = 0;
      if (convPXL((word & pxlmsk) >> 8) > 0) {
        mycol = ((word & dclmsk) >> 16) * 2 + 1;
      } else {
        mycol = ((word & dclmsk) >> 16) * 2;
      }

      printf("%2lu %1lu %2i %2i %3lu %i\n", chan, (word & rocmsk) >> 21, mycol, abs(convPXL((word & pxlmsk) >> 8)), (word & plsmsk), event);

    } else {
    }
  } else {
  }
}

int ReadEvent (std::ifstream& InFile)
{
  unsigned int n1, n2;

  int wordcount = 0;
  bool bheader = true;
  for (bool NotDone = true; NotDone; ) {

    // Read 64-bit word
    InFile.read((char *) &n2, sizeof n2);
    InFile.read((char *) &n1, sizeof n1);


    if (((n1 & 0xff000000) == 0x50000000) && (wordcount == 0)) {
      // Found the header
      wordcount = 0;
      bheader = true;
      int event = (n1 & 0xffffff);
      std::cout << "Found Event Header: " << event << std::endl;

      while (bheader) {
        InFile.read((char *) &n2, sizeof n2);
        InFile.read((char *) &n1, sizeof n1);

        if ((n1 & 0xf0000000) == 0xa0000000) {
          bheader = false;
          std::cout << "Found Event Trailer: " << std::endl;// << event << std::endl;
        } else {
          decodeSpyDataFifo(n2,event);
          decodeSpyDataFifo(n1,event);
        }
      }
    }  
  }

  return 0;
}


int ReadEvents (std::string const& InFileName)
{
  // Open input file stream
  std::ifstream InFile(InFileName.c_str(), std::ios::in | std::ios::binary);
  if (!InFile) {
    std::cerr << "ERROR: cannot open input file: " << InFileName << std::endl;
    throw;
  }

  int NEvents = 0;
  while (!InFile.eof()) {
    ReadEvent(InFile);
    ++NEvents;
  }

  return NEvents;
}



int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFileName]" << std::endl;
    return 1;
  }

  std::string const InFileName = argv[1];
  ReadEvents(InFileName);

  return 0;
}
