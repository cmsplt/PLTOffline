#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>

const int nBX = 3564;
int totEvents[nBX] = {0};
int totHits[nBX] = {0};

int convertPixel(int pix) {
  if (pix == 160 || pix == 161) return 0;
  return pix % 2 == 1 ? 80 - (pix - 1) / 2 : (pix) / 2 - 80;
}

bool decodeDataWord (uint32_t unsigned word, int counts[]) {
  if (word & 0xfffffff) {
    //const uint32_t unsigned plsmsk = 0xff;
    const uint32_t unsigned pxlmsk = 0xff00;
    const uint32_t unsigned dclmsk = 0x1f0000;
    const uint32_t unsigned rocmsk = 0x3e00000;
    const uint32_t unsigned chnlmsk = 0xfc000000;
    uint32_t unsigned chan = ((word & chnlmsk) >> 26);
    uint32_t unsigned roc  = ((word & rocmsk)  >> 21);
    //uint32_t rawPixel = ((word & pxlmsk) >> 8);

    // Check for embeded special words: roc > 25 is special, not a hit
    if (roc > 25) {
      if ((word & 0xffffffff) == 0xffffffff) {
	counts[4]++;
      } else if (roc == 26) {
	counts[1]++;
      } else if (roc == 27) {
	counts[1]++;
      } else if (roc == 28) {
	counts[4]++;
      } else if (roc == 29) {
	counts[2]++;
      } else if (roc == 30) {
	counts[4]++;
      } else if (roc == 31) {
	counts[3]++;
      }
      return false;
    } else if (chan > 0 && chan < 37) {
      // Oh, NOW we have a hit!
      int mycol = 0;
      //if (convPXL((word & pxlmsk) >> 8) > 0) {
      if ( ((word & pxlmsk) >> 8) % 2) {
        // Odd
        mycol = ((word & dclmsk) >> 16) * 2 + 1;
      } else {
        // Even
        mycol = ((word & dclmsk) >> 16) * 2;
      }

      roc -= 1; // The fed gives 123, and we use the convention 012
      //kga changed fiducial region
      if (roc <= 2) {
	counts[0]++;
      } else {
	counts[4]++;
        //std::cerr << "WARNING: PLTBinaryFileReader found ROC with number and chan: " << roc << "  " << chan << std::endl;
      }

      //printf("%2lu %1lu %2i %2i %3lu %i\n", chan, (word & rocmsk) >> 21, mycol, abs(convPXL((word & pxlmsk) >> 8)), (word & plsmsk), -1);
      return true;

    } else {
      counts[4]++;
    }
  }
  counts[4]++;
  return false;
}

int readEvent(std::ifstream& infile) {
  static int fLastTime = -1;
  static int fTimeMult = 0;
  uint32_t n1=0, n2=0, oldn1=0, oldn2=0;
  uint32_t event, time, bx, fedid;
 
  int nWords = 0;
  int wordcount = 0;
  bool bheader = true;
  int counts[5];
  for (int i=0; i<5; ++i) counts[i] = 0;
  while (bheader) {

    // Read 64-bit word
    infile.read((char*) &n2, sizeof n2);
    infile.read((char*) &n1, sizeof n1);

    if (infile.eof()) {
      return -1;
    }

    if ((n1 == 0x53333333) && (n2 == 0x53333333)) {
      //tdc buffer, special handling

      for (int ih = 0; ih < 100; ih++) {

        infile.read((char *) &n1, sizeof n1);

        if ((n1 & 0xf0000000) == 0xa0000000) {
          infile.read((char *) &n2, sizeof n2);
          break;
        }
        if (infile.eof()) {
          return -1;
        }
      }

    } else if ( ((n1 & 0xff000000) == 0x50000000 && (n2 & 0xff) == 0 ) || ((n2 & 0xff000000) == 0x50000000 && (n1 & 0xff) == 0)) { 
      // Found the header and it has correct FEDID
      wordcount = 1;
      bheader = true;
      event = (n1 & 0xff000000) == 0x50000000 ? n1 & 0xffffff : n2 & 0xffffff;
      //std::cout << "Found Event Header: " << Event << std::endl;

      if ((n1 & 0xff000000) == 0x50000000) {
        bx = ((n2 & 0xfff00000) >> 20);
        fedid = ((n2 & 0xfff00) >> 8);
      } else {
        bx = ((n1 & 0xfff00000) >> 20);
        fedid = ((n1 & 0xfff00) >> 8);
      }

      while (bheader) {
	// The older slink files contain a bug which can sometimes cause
	// a pair of hits to be duplicated. To fix this, keep track of
	// the previous words and only process the new ones if they're different.
	oldn1=n1;
	oldn2=n2;
        infile.read((char *) &n2, sizeof n2);
        infile.read((char *) &n1, sizeof n1);
        if (infile.eof()) {
          return -1;
        }

        ++wordcount;

        if ((n1 & 0xff000000) == 0xa0000000 || (n2 & 0xff000000) == 0xa0000000) {
          bheader = false;
          if ((n1 & 0xff000000) == 0xa0000000) {
            time = n2;
          } else {
            time = n1;
          }
          if ((int)time < fLastTime) {
            ++fTimeMult;
          }

          fLastTime = time;
          time = time + 86400000 * fTimeMult;
          //std::cout << "Found Event Trailer: " << Event << std::endl;
        } else {
	  // Be very careful here. Maybe we've got an odd number of words and so n1 is actually
	  // the first word of the trailer. Let's peek and see.
	  uint32_t peekWord;
	  infile.read((char *) &peekWord, sizeof peekWord);
	  if (infile.eof()) {
	    // shouldn't happen unless the event is truncated in some way
	    return -1;
	  }

	  // Aha, it was!
	  if ((peekWord & 0xff000000) == 0xa0000000) {
	    bheader = false;
	    time = n1;
	    if ((int)time < fLastTime) {
	      ++fTimeMult;
	    }
	    fLastTime = time;
	    time = time + 86400000 * fTimeMult;

	    // but don't forget to decode the first word
	    if (n2 != oldn2)
	      decodeDataWord(n2, counts);
	    nWords += 1;
	  }
	  else {
	    // OK, it wasn't. Undo the peek and decode both words
	    infile.seekg(-sizeof peekWord, std::ios_base::cur);
	    nWords += 2;
	    if (n2 != oldn2)
	      decodeDataWord(n2, counts);
	    if (n1 != oldn1)
	    decodeDataWord(n1, counts);
	  }
        }
      } // reading event data
    }
  }
  //if (nWords % 2 == 1) std::cout << "*****";
  
  totEvents[bx]++;
  totHits[bx]+=counts[0];

  return nWords;
}

int main(const int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: TestBinaryFileReader <file>" << std::endl;
    exit(1);
  }
  const char *fileName = argv[1];

  std::cout << "Opening file " << fileName << std::endl;

  std::ifstream infile(fileName, std::ios::in | std::ios::binary);
  if (!infile.is_open()) {
    std::cerr << "Error opening file " << fileName << std::endl;
    exit(1);
  }

  int nEvents = 0;
  while (!infile.eof()) {
    readEvent(infile);
    nEvents++;
    if (nEvents >= 3e6) {
      std::cout << "Reached 3M events" << std::endl;
      break;
    }
  }
  infile.close();

  int nFilledBX = 0;
  for (int i=0; i<nBX; ++i) {
    if (totEvents[i] > 0) {
      std::cout << "[" << i+1 << "]: " << totEvents[i] << " events (" << (float)totHits[i]/totEvents[i] << " hits/evt)" << std::endl;
      nFilledBX++;
    }
  }
  std::cout << "Found a total of " << nFilledBX << " filled bunch crossings" << std::endl;
}
