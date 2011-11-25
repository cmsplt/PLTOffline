#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>


using namespace std;

int convPXL (int ROC)
{
  return ROC % 2 ? 80 - (ROC - 1) / 2 : (ROC - 1) / 2 - 80;
}

/////////////////////////////////////////////////////////////////////////////
// Decode error FIFO 
void decodeErrorFifo(uint32_t unsigned word)
{
  // Works for both, the error FIFO and the SLink error words. d.k. 25/04/07
  const uint32_t unsigned  errorMask      = 0x3e00000;
  const uint32_t unsigned  timeOut        = 0x3a00000;
  const uint32_t unsigned  eventNumError  = 0x3e00000;
  const uint32_t unsigned  trailError     = 0x3c00000;
  const uint32_t unsigned  fifoError      = 0x3800000;


  if (word & 0xffffffff) {
    if ((word & errorMask) == timeOut) {

      // TIMEOUT - More than 1 channel within a group can have a timeout error
      uint32_t unsigned index = (word & 0x1F);  // index within a group of 4/5

      for (int i = 0; i < 5; i++) {
        index = index >> 1;
      }
    } else if ((word & errorMask) == eventNumError) {

    } else if (((word & errorMask) == trailError)) {


    } else if ((word & errorMask) == fifoError) {
    }
  }
  return;
}


//////////////////////////////////////////////////////////////////////////////
//
// Decode FIFO 2 spy data 
// The headers are still not treated correctly.
// 
void decodeSpyDataFifo (uint32_t unsigned word, int event)
{
  if (word & 0xfffffff) {

    const uint32_t unsigned plsmsk = 0xff;
    const uint32_t unsigned pxlmsk = 0xff00;
    const uint32_t unsigned dclmsk = 0x1f0000;
    const uint32_t unsigned rocmsk = 0x3e00000;
    const uint32_t unsigned chnlmsk = 0xfc000000;
    uint32_t unsigned chan = ((word & chnlmsk) >> 26);
    uint32_t unsigned roc  = ((word & rocmsk)  >> 21);

    // Check for embeded special words
    if (roc > 25) {

      if ((word & 0xffffffff) == 0xffffffff) {
      } else if (roc == 26) {
      } else if (roc == 27) {
      } else {
        decodeErrorFifo(word);
      }
    } else if (chan > 0 && chan < 37) {

      int mycol=0;
      if (convPXL((word & pxlmsk) >> 8) > 0) {
        mycol = ((word & dclmsk) >> 16) * 2 + 1;
      } else {
        mycol = ((word & dclmsk) >> 16) * 2;
      }

      printf("%2lu %1i %2i %2i %3lu %i\n", chan,(int) roc - 1, mycol, abs(convPXL((word & pxlmsk) >> 8)), (word & plsmsk), event);

    } else {
    }
  } else {
  }
}


int main(int argc, char *argv[])
{
  bool bheader = false;

  if (argc < 2) {
    cout << argc << " usage: decodefil filename" << endl;
    return 1;
  }

  // Open input file
  std::ifstream in(argv[1], std::ios::in | std::ios::binary); 
  if (!in) { 
    std::cout << "Cannot open file. " << endl; 
    return 1; 
  }

  unsigned long long totevent = 0;
  uint32_t unsigned header = 0;
  uint32_t unsigned n1, n2;
  int wrdcount  =  0;
  int event = 0;
  int stopit = 0;

  //begin of file word
  in.read((char *) &n2, sizeof n2);
  in.read((char *) &n1, sizeof n1);

  int hitcount = 0;

  for (int i = 0; i < 2000000000; i++) {

    // Read a word
    in.read((char *) &n2, sizeof n2);
    in.read((char *) &n1, sizeof n1);


    if (in.gcount() != sizeof(n1)) {
      // End of file!
      return 0;
    }

    wrdcount++;

    if ((n1 == 0x53333333) && (n1 == 0x53333333)) {
      //tdc buffer, special handling

      for (int ih = 0; ih < 100; ih++) {

        in.read((char *) &n1, sizeof n1);

        if ((n1 & 0xf0000000) == 0xa0000000) {

          in.read((char *) &n2, sizeof n1);
          wrdcount = 0;
          break;
        }
      }

    } else {
      if (((n1 & 0xff000000) == 0x50000000) & (wrdcount == 1)) {
        // Found the header
        wrdcount = 0;
        header = n1;
        bheader = true;
        stopit = 0;
        event = (n1 & 0xffffff);
        totevent++;
        //printf("Found header event:  %i\n", event);
      } else if ((n1 & 0xf0000000) == 0xa0000000) {
        bheader = false;
        //printf("nhits: %i\n", hitcount);
        //printf("Found trailer event: %i\n", event);
        if (stopit > 0) {
          string input;
          getline(cin, input);
        }

        wrdcount = 0;
        wrdcount = 0;
        hitcount = 0;

        if ((n2 & 4) != 0) {
        }
      } else if (((n1 & 0xfff00000) == 0x3600000) && ((n2 & 0xfff00000) == 0x3600000)) {
        if ((n1 & 0xff) != (n2 & 0xff)) {
          bheader = false;
        }
      } else if (((n1 & 0x3e00000) > 0x3700000)) {
        if (bheader) {
          decodeSpyDataFifo(n1,event);
        }
        if (((n2 & 0x3e00000) > 0x3700000)) {
          if (bheader) {
            decodeSpyDataFifo(n2,event);
          }
        }
        if (((n2 & 0x3e00000) < 0x3200000)) {
          if (bheader) {
            decodeSpyDataFifo(n2,event);
          }
        }
      } else if(((n2 & 0x3e00000) > 0x3700000)) {
        if (bheader) {
          decodeSpyDataFifo(n2,event);
        }
        if (((n1 & 0x3e00000) > 0x3700000)) {
          if (bheader) {
            decodeSpyDataFifo(n1,event);
          }
        }
        if (((n1 & 0x3e00000) < 0x3200000)) {
          if (bheader) {
            decodeSpyDataFifo(n1,event);
          }
        }
      } else if(((n1 & 0x3e00000) < 0x3200000)) {
        hitcount++;
        if (bheader) {
          decodeSpyDataFifo(n1,event);
        }
        if (((n2 & 0x3e00000) > 0x3700000)) {
          if (bheader) {
            decodeSpyDataFifo(n2,event);
          }
        }
        if (((n2 & 0x3e00000) < 0x3200000)) {
          if (bheader) {
            decodeSpyDataFifo(n2,event);
          }
        }
      } else if (((n2 & 0x3e00000) < 0x3200000)) {
        hitcount++;
        if (bheader) {
          decodeSpyDataFifo(n2,event);
        }
        if (((n1 & 0x3e00000) > 0x3700000)) {
          if (bheader) {
            decodeSpyDataFifo(n1,event);
          }
        }
        if (((n1 & 0x3e00000) < 0x3200000)) {
          if (bheader) {
            decodeSpyDataFifo(n1,event);
          }
        }
      }
    }
  }
  in.close();
}

