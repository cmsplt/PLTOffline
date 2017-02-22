#include <iostream>
#include <fstream>

const bool fullDebug = true;

int convertPixel(int pix) {
  if (pix == 160 || pix == 161) return 0;
  return pix % 2 == 1 ? 80 - (pix - 1) / 2 : (pix) / 2 - 80;
}

bool decodeDataWord (uint32_t unsigned word, int counts[]) {
  if (fullDebug) std::cout << std::hex << "0x" << word << std::dec << ": ";
  if (word & 0xfffffff) {
    //const uint32_t unsigned plsmsk = 0xff;
    const uint32_t unsigned pxlmsk = 0xff00;
    const uint32_t unsigned dclmsk = 0x1f0000;
    const uint32_t unsigned rocmsk = 0x3e00000;
    const uint32_t unsigned chnlmsk = 0xfc000000;
    uint32_t unsigned chan = ((word & chnlmsk) >> 26);
    uint32_t unsigned roc  = ((word & rocmsk)  >> 21);
    uint32_t rawPixel = ((word & pxlmsk) >> 8);

    // Check for embeded special words: roc > 25 is special, not a hit
    if (roc > 25) {
      if ((word & 0xffffffff) == 0xffffffff) {
	counts[4]++;
	if (fullDebug) std::cout << "Unknown Error" << std::endl;
      } else if (roc == 26) {
	counts[1]++;
	if (fullDebug) std::cout << "Gap" << std::endl;
      } else if (roc == 27) {
	counts[1]++;
	if (fullDebug) std::cout << "Dummy  " << std::endl;
      } else if (roc == 28) {
	counts[4]++;
	if (fullDebug) std::cout << "Near Full " << std::endl;
      } else if (roc == 29) {
	counts[2]++;
	if (fullDebug) {
	  std::cout << "Time Out ";
	  if ((word & 0xff00000) == 0x3b00000) std::cout << "(word 1)" << std::endl;
	  else if ((word & 0xff00000) == 0x3a00000) {
	    std::cout << "(word 2) channels:";
	    
	    // More than 1 channel within a group can have a timeout error
	    unsigned int index = (word & 0x1f);  // index within a group of 4/5
	    unsigned int chip = (word & 0x700) >> 8;
	    const int offsets[8] = {0,4,9,13,18,22,27,31};
	    int offset = offsets[chip];
	    for(int i=0; i<5; i++) {
	      if( (index & 0x1) != 0) {
		int chan = offset + i + 1;
		std::cout<<" "<<chan;
	    }
	      index = index >> 1;
	    }
	    std::cout<<std::endl;
	    //end of timeout  chip and channel decoding
	  }
	else std::cout << "(unknown word)" << std::endl;
	}
      } else if (roc == 30) {
	counts[4]++;
	if (fullDebug) {
	  std::cout << "Trailer error channel " << chan;
	  if (word & (1 << 8)) std::cout << ": data overflow";
	  if (word & (3 << 9)) std::cout << ": FSM error " << (word & (3 << 9));
	  if (word & (1 << 11)) std::cout << ": invalid # of ROCs";
	  if (word & (0xff)) {
	    std::cout << " from TBM:";
	    const uint32_t tbmErr = word & (0xff);
	    if (tbmErr & 1) std::cout << " stack full";
	    if (tbmErr & (1 << 1)) std::cout << " precal trigger issued";
	    if (tbmErr & (1 << 2)) std::cout << " event counter cleared";
	    if (tbmErr & (1 << 3)) std::cout << " sync trigger";
	    if (tbmErr & (1 << 4)) std::cout << " sync trigger error";
	    if (tbmErr & (1 << 5)) std::cout << " ROC reset";
	    if (tbmErr & (1 << 6)) std::cout << " TBM reset";
	    if (tbmErr & (1 << 7)) std::cout << " no token pass";
	  }
	  std::cout << std::endl;
	}
      } else if (roc == 31) {
	counts[3]++;
	if (fullDebug) std::cout << "Event# Error: channel " << chan << " event " << (word & 0xff) << " " << std::endl;
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
	std::cout << "Hit: " << chan << "-" << roc << "@" << mycol << "," << abs(convertPixel(rawPixel)) << std::endl;
	counts[0]++;
      } else {
	counts[4]++;
	if (fullDebug) std::cout << "Hit (Bad ROC) " << std::endl;
        //std::cerr << "WARNING: PLTBinaryFileReader found ROC with number and chan: " << roc << "  " << chan << std::endl;
      }

      //printf("%2lu %1lu %2i %2i %3lu %i\n", chan, (word & rocmsk) >> 21, mycol, abs(convPXL((word & pxlmsk) >> 8)), (word & plsmsk), -1);
      return true;

    } else {
      counts[4]++;
      if (fullDebug) std::cout << "Invalid Channel " << std::endl;
    }
  }
  counts[4]++;
  if (fullDebug) std::cout << "(null)" << std::endl;
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

    } else if ( ((n1 & 0xff000000) == 0x50000000 && (n2 & 0xff) == 0 ) || ( ((n2 & 0xff000000) == 0x50000000) ) && (n1 & 0xff) == 0 ){
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

      std::cout << "Found header for event " << event << " BX " << bx << " (" << bx+1 << ")" << std::endl;

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
  std::cout << "Found trailer for event " << event << " time " << time << " data words " << nWords << std::endl;
  std::cout << "Hits: " << counts[0] << " Gap/Dummy: " << counts[1] << " Ev#Err: " << counts[3]
	    << " Timeout: " << counts[2] << " OtherErr: " << counts[4] << std::endl;
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
    // if (nEvents >= 200) break;
  }
  infile.close();
}
