#include "PLTBinaryFileReader.h"



PLTBinaryFileReader::PLTBinaryFileReader ()
{
  fPlaneFiducialRegion = PLTPlane::kFiducialRegion_Diamond;
  fLastTime = 0;
  fTimeMult = 0;
}


PLTBinaryFileReader::PLTBinaryFileReader (std::string const in, bool IsText)
{
  SetIsText(IsText);

  if (fIsText) {
    OpenTextFile(in);
  } else {
    Open(in);
  }
  fPlaneFiducialRegion = PLTPlane::kFiducialRegion_Diamond;
  fLastTime = 0;
  fTimeMult = 0;
}


PLTBinaryFileReader::~PLTBinaryFileReader ()
{
}



bool PLTBinaryFileReader::Open (std::string const DataFileName)
{
  if (fIsText) {
    return OpenTextFile(DataFileName);
  } else {
    return OpenBinary(DataFileName);
  }
}



bool PLTBinaryFileReader::OpenBinary (std::string const DataFileName)
{
  fFileName = DataFileName;
  fInfile.open(fFileName.c_str(),  std::ios::in | std::ios::binary);
  if (!fInfile) {
    std::cerr << "ERROR: cannot open input file: " << fFileName << std::endl;
    return false;
  }

  return true;
}



bool PLTBinaryFileReader::OpenTextFile (std::string const DataFileName)
{
  fFileName = DataFileName;
  fInfile.open(fFileName.c_str());
  if (!fInfile) {
    std::cerr << "ERROR: cannot open input file: " << fFileName << std::endl;
    return false;
  }

  return true;
}



void PLTBinaryFileReader::SetIsText (bool const in)
{
  fIsText = in;
  return;
}



int PLTBinaryFileReader::convPXL (int IN)
{
  return IN % 2 == 1 ? 80 - (IN - 1) / 2 : (IN) / 2 - 80;
}



bool PLTBinaryFileReader::DecodeSpyDataFifo (uint32_t unsigned word, std::vector<PLTHit*>& Hits)
{
  if (word & 0xfffffff) {

    const uint32_t unsigned plsmsk = 0xff;
    const uint32_t unsigned pxlmsk = 0xff00;
    const uint32_t unsigned dclmsk = 0x1f0000;
    const uint32_t unsigned rocmsk = 0x3e00000;
    const uint32_t unsigned chnlmsk = 0xfc000000;
    uint32_t unsigned chan = ((word & chnlmsk) >> 26);
    uint32_t unsigned roc  = ((word & rocmsk)  >> 21);

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
        // Odd
        mycol = ((word & dclmsk) >> 16) * 2 + 1;
        //std::cout << "MYCOL A: " << mycol << std::endl;
      } else {
        // Even
        mycol = ((word & dclmsk) >> 16) * 2;
        //std::cout << "MYCOL B: " << mycol << std::endl;
      }

      roc -= 1; // The fed gives 123, and we use the convention 012
      //kga changed fiducial region
      if (roc <= 2) {

        // Check the pixel mask
        if ( !IsPixelMasked( chan*100000 + roc*10000 + mycol*100 + abs(convPXL((word & pxlmsk) >> 8)) ) ) {

          //printf("IN OUT: %10i %10i\n", (word & pxlmsk) >> 8, convPXL((word & pxlmsk) >> 8));
          PLTHit* Hit = new PLTHit((int) chan, (int) roc, (int) mycol, (int) abs(convPXL((word & pxlmsk) >> 8)), (int) (word & plsmsk));

          // only keep hits on the diamond
          if (PLTPlane::IsFiducial(fPlaneFiducialRegion, Hit)) {
            Hits.push_back(Hit);
          } else {
            delete Hit;
          }
        }
      } else {
        //std::cerr << "WARNING: PLTBinaryFileReader found ROC with number and chan: " << roc << "  " << chan << std::endl;
      }

      //printf("%2lu %1lu %2i %2i %3lu %i\n", chan, (word & rocmsk) >> 21, mycol, abs(convPXL((word & pxlmsk) >> 8)), (word & plsmsk), -1);
      return true;

    }
  }
  return false;
}


int PLTBinaryFileReader::ReadEventHits (std::vector<PLTHit*>& Hits, unsigned long& Event, uint32_t& Time, uint32_t& BX)
{
  if (fIsText) {
    return ReadEventHitsText(fInfile, Hits, Event, Time, BX);
  } else {
    return ReadEventHits(fInfile, Hits, Event, Time, BX);
  }
}


int PLTBinaryFileReader::ReadEventHits (std::ifstream& InFile, std::vector<PLTHit*>& Hits, unsigned long& Event, uint32_t& Time, uint32_t& BX)
{
  uint32_t unsigned n1, n2;

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
          InFile.read((char *) &n2, sizeof n2);
          break;
        }
        if (InFile.eof()) {
          return -1;
        }
      }

    } else if ( ((n1 & 0xff000000) == 0x50000000 && ((n2 & 0xfff00) >> 8) == 5 && (n2 & 0xff) == 0 ) || ( ((n2 & 0xff000000) == 0x50000000) && ((n1 & 0xfff00) >> 8) == 5 ) && (n2 & 0xff) == 0 ){
      // Found the header and it has correct FEDID
      wordcount = 1;
      bheader = true;
      Event = (n1 & 0xff000000) == 0x50000000 ? n1 & 0xffffff : n2 & 0xffffff;
      //std::cout << "Found Event Header: " << Event << std::endl;

      if ((n1 & 0xff000000) == 0x50000000) {
        BX = ((n2 & 0xfff00000) >> 20);
        fFEDID = ((n2 & 0xfff00) >> 8);
      } else {
        BX = ((n1 & 0xfff00000) >> 20);
        fFEDID = ((n1 & 0xfff00) >> 8);
      }

      while (bheader) {
        InFile.read((char *) &n2, sizeof n2);
        InFile.read((char *) &n1, sizeof n1);
        if (InFile.eof()) {
          return -1;
        }

        ++wordcount;

        if ((n1 & 0xf0000000) == 0xa0000000 || (n2 & 0xf0000000) == 0xa0000000) {
          bheader = false;
          if ((n1 & 0xf0000000) == 0xa0000000) {
            Time = n2;
          } else {
            Time = n1;
          }
          if (Time < fLastTime) {
            ++fTimeMult;
          }

          fLastTime = Time;
          Time = Time + 86400000 * fTimeMult;
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




int PLTBinaryFileReader::ReadEventHitsText (std::ifstream& InFile, std::vector<PLTHit*>& Hits, unsigned long& Event, uint32_t& Time, uint32_t& BX)
{
  int LastEventNumber = -1;
  int EventNumber = -1;

  if (InFile.eof()) {
    return -1;
  }

  int Channel, ROC, Row, Col, ADC;
  std::string Line;
  std::istringstream LineStr;
  while (true) {
    std::getline(InFile, Line);
    LineStr.clear();
    LineStr.str(Line);

    LineStr >> Channel;
    if (LineStr.eof()) {
      break;
    }
    LineStr >> ROC >> Col >> Row >> ADC >> EventNumber;


    if (EventNumber != LastEventNumber && LastEventNumber != -1) {
      InFile.putback('\n');
      for (std::string::reverse_iterator It = Line.rbegin(); It != Line.rend(); ++It) {
        InFile.putback(*It);
      }
      break;
    }

    if ( !IsPixelMasked( Channel*100000 + ROC*10000 + Col*100 + Row ) ) {
      PLTHit* Hit = new PLTHit(Channel, ROC, Col, Row, ADC);
      // only keep hits on the diamond
      if (PLTPlane::IsFiducial(PLTPlane::kFiducialRegion_Diamond, Hit)) {
        Hits.push_back(Hit);
      } else {
        delete Hit;
      }
      //printf("%2i %2i %2i %2i %5i %9i\n", Channel, ROC, Col, Row, ADC, EventNumber);
    }

    LastEventNumber = EventNumber;
    Event = EventNumber;

  }
  

  return Hits.size();
}



void PLTBinaryFileReader::ReadPixelMask (std::string const InFileName)
{
  std::cout << "PLTBinaryFileReader::ReadPixelMask reading file: " << InFileName << std::endl;

  std::ifstream InFile(InFileName.c_str());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open PixelMask file: " << InFileName << std::endl;
    throw;
  }

  // Loop over header lines in the input data file
  for (std::string line; std::getline(InFile, line); ) {
    if (line == "") {
      break;
    }
  }

  std::istringstream linestream;
  int ch, roc, col, row;
  for (std::string line; std::getline(InFile, line); ) {
    linestream.str(line);
    linestream >> ch >> roc >> col >> row;

    fPixelMask.insert( ch*100000 + roc*10000 + col*100 + row );
  }

  return;
}


bool PLTBinaryFileReader::IsPixelMasked (int const ChannelPixel)
{
  if (fPixelMask.count(ChannelPixel)) {
    return true;
  }
  return false;
}


void PLTBinaryFileReader::SetPlaneFiducialRegion (PLTPlane::FiducialRegion in)
{
  std::cout << "PLTBinaryFileReader::SetPlaneFiducialRegion setting region: " << in << std::endl;
  fPlaneFiducialRegion = in;
  return;
}
