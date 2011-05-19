#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int convPXL (int ROC)
{
  return ROC % 2 ? 80 - (ROC - 1) / 2 : (ROC - 1) / 2 - 80;
}

/////////////////////////////////////////////////////////////////////////////
// Decode error FIFO 
void decodeErrorFifo(unsigned long word)
{
  // Works for both, the error FIFO and the SLink error words. d.k. 25/04/07
  const unsigned long  errorMask      = 0x3e00000;
  const unsigned long  timeOut        = 0x3a00000;
  const unsigned long  eventNumError  = 0x3e00000;
  const unsigned long  trailError     = 0x3c00000;
  const unsigned long  fifoError      = 0x3800000;

  const unsigned long  eventNumMask = 0x1fe000; // event number mask
  const unsigned long  channelMask = 0xfc000000; // channel num mask
  const unsigned long  tbmEventMask = 0xff;    // tbm event num mask
  const unsigned long  overflowMask = 0x100;   // data overflow
  const unsigned long  tbmStatusMask = 0xff;   //TBM trailer info
  const unsigned long  BlkNumMask = 0x700;   //pointer to error fifo #
  const unsigned long  FsmErrMask = 0x600;   //pointer to FSM errors
  const unsigned long  RocErrMask = 0x800;   //pointer to #Roc errors 
  const unsigned long  ChnFifMask = 0x1f;   //channel mask for fifo error 
  const unsigned long  Fif2NFMask = 0x40;   //mask for fifo2 NF 
  const unsigned long  TrigNFMask = 0x80;   //mask for trigger fifo NF 

  const int offsets[8] = {0,4,9,13,18,22,27,31};


  if (word & 0xffffffff) {
    if ((word & errorMask) == timeOut) {

      // TIMEOUT - More than 1 channel within a group can have a timeout error
      unsigned long index = (word & 0x1F);  // index within a group of 4/5
      unsigned long chip = (word & BlkNumMask) >> 8;
      int offset = offsets[chip];

      for (int i = 0; i < 5; i++) {
        if( (index & 0x1) != 0) {
          int channel = offset + i + 1;
        }
        index = index >> 1;
      }
    } else if ((word & errorMask) == eventNumError) {

      // EVENT NUMBER ERROR
      unsigned long channel = (word & channelMask) >> 26;
      unsigned long tbm_event = (word & tbmEventMask);

    } else if (((word & errorMask) == trailError)) {

      unsigned long channel = (word & channelMask) >> 26;
      unsigned long tbm_status = (word & tbmStatusMask);

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
void decodeSpyDataFifo (unsigned long word, int event)
{
  const bool ignoreInvalidData = false;

  if (word & 0xfffffff) {

    const unsigned long int plsmsk = 0xff;
    const unsigned long int pxlmsk = 0xff00;
    const unsigned long int dclmsk = 0x1f0000;
    const unsigned long int rocmsk = 0x3e00000;
    const unsigned long int chnlmsk = 0xfc000000;
    unsigned long int chan = ((word & chnlmsk) >> 26);
    unsigned long int roc  = ((word & rocmsk)  >> 21);

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
      if (convPXL((word &pxlmsk) >> 8) > 0) {
        mycol = ((word & dclmsk) >> 16) * 2 + 1;
      } else {
        mycol = ((word & dclmsk) >> 16) * 2;
      }

      printf("%2i %1i %2i %2i %3i %i\n", chan, (word & rocmsk) >> 21, mycol, abs(convPXL((word & pxlmsk) >> 8)), (word & plsmsk), event);

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
  unsigned long pickit = 0;
  unsigned long header = 0;
  unsigned long n1, n2;
  unsigned long bxold = 0;
  unsigned long long n;
  int wrdcount  =  0;
  int event = 0;
  int stopit = 0;

  //begin of file word
  in.read((char *) &n2, sizeof n2);
  in.read((char *) &n1, sizeof n1);

  int hitcount = 0;

  for (int i = 0; i < 2000000000; i++) {

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
        wrdcount = 0;
        header = n1;
        bheader = true;
        stopit = 0;
        event = (n1 & 0xffffff);
        totevent++;
      } else if ((n1 & 0xf0000000) == 0xa0000000) {
        bheader = false;
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

