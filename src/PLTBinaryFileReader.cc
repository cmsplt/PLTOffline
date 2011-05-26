#include "PLTBinaryFileReader.h"



PLTBinaryFileReader::PLTBinaryFileReader ()
{
}


PLTBinaryFileReader::PLTBinaryFileReader (std::string const in)
{
  Open(in);
}


PLTBinaryFileReader::~PLTBinaryFileReader ()
{
}



bool PLTBinaryFileReader::Open (std::string const DataFileName)
{
  fFileName = DataFileName;
  fInfile.open(fFileName.c_str(),  std::ios::in | std::ios::binary);
  if (!fInfile) {
    std::cerr << "ERROR: cannot open input file: " << fFileName << std::endl;
    return false;
  }

  return true;
}



int PLTBinaryFileReader::convPXL (int ROC)
{
  return ROC % 2 ? 80 - (ROC - 1) / 2 : (ROC - 1) / 2 - 80;
}



bool PLTBinaryFileReader::DecodeSpyDataFifo (unsigned long word, std::vector<PLTHit>& Hits)
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
      return false;
    } else if (chan > 0 && chan < 37) {
      // Oh, NOW we have a hit!
      int mycol = 0;
      if (convPXL((word & pxlmsk) >> 8) > 0) {
        mycol = ((word & dclmsk) >> 16) * 2 + 1;
      } else {
        mycol = ((word & dclmsk) >> 16) * 2;
      }

      roc -= 1; // The fed gives 123, and we use the convention 012
      PLTHit Hit((int) chan, (int) roc, (int) mycol, (int) abs(convPXL((word & pxlmsk) >> 8)), (int) (word & plsmsk));
      Hits.push_back(Hit);

      //printf("%2lu %1lu %2i %2i %3lu %i\n", chan, (word & rocmsk) >> 21, mycol, abs(convPXL((word & pxlmsk) >> 8)), (word & plsmsk), -1);
      return true;

    }
  }
  return false;
}


int PLTBinaryFileReader::ReadEventHits (std::vector<PLTHit>& Hits, unsigned long& Event)
{
  return ReadEventHits(fInfile, Hits, Event);
}


int PLTBinaryFileReader::ReadEventHits (std::ifstream& InFile, std::vector<PLTHit>& Hits, unsigned long& Event)
{
  unsigned int n1, n2;

  int wordcount = 0;
  bool bheader = true;
  while (bheader) {

    // Read 64-bit word
    InFile.read((char *) &n2, sizeof n2);
    InFile.read((char *) &n1, sizeof n1);

    if (InFile.eof()) {
      return -1;
    }

    if ((n1 == 0x53333333) && (n2 == 0x53333333)) {
      //tdc buffer, special handling

      for (int ih = 0; ih < 100; ih++) {

        InFile.read((char *) &n1, sizeof n1);

        if ((n1 & 0xf0000000) == 0xa0000000) {
          InFile.read((char *) &n2, sizeof n1);
          wordcount = 1;
          break;
        }
      }

    } else if (((n1 & 0xff000000) == 0x50000000) && (wordcount == 1)) {
      // Found the header
      wordcount = 0;
      bheader = true;
      Event = (n1 & 0xffffff);
      //std::cout << "Found Event Header: " << Event << std::endl;

      while (bheader) {
        InFile.read((char *) &n2, sizeof n2);
        InFile.read((char *) &n1, sizeof n1);

        if ((n1 & 0xf0000000) == 0xa0000000) {
          bheader = false;
          //std::cout << "Found Event Trailer: " << Event << std::endl;
        } else {
          DecodeSpyDataFifo(n2, Hits);
          DecodeSpyDataFifo(n1, Hits);
        }
      }
    }
  }

  return Hits.size();
}

