#include "PLTBinaryFileReader.h"

PLTBinaryFileReader::PLTBinaryFileReader ()
{
  fPlaneFiducialRegion = PLTPlane::kFiducialRegion_All;
  fLastTime = 0;
  fTimeMult = 0;
  fInputType = kBinaryFile;
}


PLTBinaryFileReader::PLTBinaryFileReader (std::string const in, InputType inputType)
{
  fInputType = inputType;

  Open(in);
  fPlaneFiducialRegion = PLTPlane::kFiducialRegion_All;
  fLastTime = 0;
  fTimeMult = 0;
}


PLTBinaryFileReader::~PLTBinaryFileReader ()
{
}



bool PLTBinaryFileReader::Open (std::string const DataFileName)
{
  if (fInputType == kBinaryFile) {
    return OpenBinary(DataFileName);
  } else if (fInputType == kTextFile) {
    return OpenTextFile(DataFileName);
  } else if (fInputType == kBuffer) {
    // nothing to do here
    return true;
  } else {
    std::cerr << "Unknown input type " << fInputType << std::endl;
    exit(1);
  }
  return false;
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



void PLTBinaryFileReader::SetInputType(InputType inputType)
{
  fInputType = inputType;
  return;
}



int PLTBinaryFileReader::convPXL (int IN)
{
  if (IN == 160 || IN == 161) return 0;
  return IN % 2 == 1 ? 80 - (IN - 1) / 2 : (IN) / 2 - 80;
}



bool PLTBinaryFileReader::DecodeSpyDataFifo (uint32_t word, std::vector<PLTHit*>& Hits, std::vector<PLTError>& Errors, std::vector<int>& DesyncChannels)
{
  if (word & 0xfffffff) {

    const uint32_t plsmsk = 0xff;
    const uint32_t pxlmsk = 0xff00;
    const uint32_t dclmsk = 0x1f0000;
    const uint32_t rocmsk = 0x3e00000;
    const uint32_t chnlmsk = 0xfc000000;
    uint32_t chan = ((word & chnlmsk) >> 26);
    uint32_t roc  = ((word & rocmsk)  >> 21);

    // Check for embeded special words: roc > 25 is special, not a hit
    // Most of these are errors and so we will create a PLTError to add to the Errors vector.
    // Some of these can just be ignored.
    if (roc > 25) {
      if ((word & 0xffffffff) == 0xffffffff) {
	// unknown error
	Errors.push_back(PLTError(0, kUnknownError, 0));
      } else if (roc == 26) {
	// gap word -- this can be ignored
      } else if (roc == 27) {
	// dummy word -- this can be ignored
      } else if (roc == 28) {
	// near full error
	Errors.push_back(PLTError(0, kNearFull, (word & 0xff)));
      } else if (roc == 29) {
	// time out error. these are a little complicated because there are two words.
	// Fortunately, the first word is just the pedestal value and can be ignored.
	// Unfortunately, the second word is a little complicated -- rather than storing
	// the channel number normally, there is instead a bit mask showing which channels
	// have the time out error in a group, so we go through this bit mask and create
	// one error per 1 in the mask. Got all that?

	if ((word & 0xff00000) == 0x3b00000) {
	  // first word -- nothing to do here
	} else if ((word & 0xff00000) == 0x3a00000) {
	  // second word -- contains the channels which have timed out
	  uint32_t group = (word >> 8) & 0x7; // group of input channels
	  uint32_t channelMask = word & 0x1f; // bitmask for the five channels in the group
	  uint32_t timeoutCounter = (word >> 11) & 0xff;
	  const int offsets[8] = {0,4,9,13,18,22,27,31}; // the beginning of each of the eight groups
	  const int offset = offsets[group];
	  for (int i=0; i<5; i++) {
	    if (channelMask & (1 << i)) {
	      int thisChan = offset + i + 1;
	      Errors.push_back(PLTError(thisChan, kTimeOut, timeoutCounter));
	    }
	  }
	} else {
	  // what the heck, there's an error in our error?
	  // for the time being let's just ignore this
	}
      } else if (roc == 30) {
	// trailer error. for ease of decoding split this into FED trailer error and TBM error.
	// it's possible we could actually have BOTH errors, so do this check independently.
	if (word & (0xf00)) {
	  Errors.push_back(PLTError(chan, kFEDTrailerError, (word >> 8) & 0xf));
	}
	if (word & (0xff)) {
	  Errors.push_back(PLTError(chan, kTBMError, (word & 0xff)));
	}
      } else if (roc == 31) {
	// event number error. for the time of being this goes into both DesyncChannels and
	// the general error vector. perhaps later we could consolidate this.
	DesyncChannels.push_back(chan);
	Errors.push_back(PLTError(chan, kEventNumberError, (word & 0xff)));
      }
      return false;
    } else if (chan > 0 && chan < 37) {
      // Oh, NOW we have a hit!
      int mycol = 0;
      //if (convPXL((word & pxlmsk) >> 8) > 0) {
      if ( ((word & pxlmsk) >> 8) % 2) {
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


int PLTBinaryFileReader::ReadEventHits(uint32_t* buf, uint32_t bufSize, std::vector<PLTHit*>& Hits, std::vector<PLTError>& Errors, unsigned long& Event, uint32_t& Time, uint32_t& BX, std::vector<int>& DesyncChannels)
{
  if (fInputType == kBinaryFile) {
    return ReadEventHitsBinary(Hits, Errors, Event, Time, BX, DesyncChannels);
  } else if (fInputType == kTextFile) {
    // we assume no errors in a text file, so fErrors will just stay empty
    return ReadEventHitsText(Hits, Event, Time, BX);
  } else if (fInputType == kBuffer) {
    return ReadEventHitsBuffer(buf, bufSize, Hits, Errors, Event, Time, BX, DesyncChannels);
  } else {
    // uh...this should have already been caught in Open()
    return -1;
  }
}


int PLTBinaryFileReader::ReadEventHitsBinary(std::vector<PLTHit*>& Hits, std::vector<PLTError>& Errors, unsigned long& Event, uint32_t& Time, uint32_t& BX, std::vector<int>& DesyncChannels)
{
  uint32_t n1, n2, oldn1, oldn2;

  int wordcount = 0;
  bool bheader = true;
  while (bheader) {

    // Read 64-bit word
    fInfile.read((char *) &n2, sizeof n2);
    fInfile.read((char *) &n1, sizeof n1);

    if (fInfile.eof()) {
      return -1;
    }

    if ((n1 == 0x53333333) && (n2 == 0x53333333)) {
      //tdc buffer, special handling

      for (int ih = 0; ih < 100; ih++) {

        fInfile.read((char *) &n1, sizeof n1);

        if ((n1 & 0xf0000000) == 0xa0000000) {
          fInfile.read((char *) &n2, sizeof n2);
          break;
        }
        if (fInfile.eof()) {
          return -1;
        }
      }

    } else if ( ((n1 & 0xff000000) == 0x50000000 && (n2 & 0xff) == 0 ) || ((n2 & 0xff000000) == 0x50000000 && (n1 & 0xff) == 0) ){
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
        // The older slink files contain a bug which can sometimes cause
        // a pair of hits to be duplicated. To fix this, keep track of
        // the previous words and only process the new ones if they're different.
        oldn1=n1;
        oldn2=n2;
        fInfile.read((char *) &n2, sizeof n2);
        fInfile.read((char *) &n1, sizeof n1);
        if (fInfile.eof()) {
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
	  // Older versions of the slink file contain a bug where there are occasionally
	  // an odd number of 32-bit words between the header and trailer. In this case,
	  // n1 is actually the first word of the trailer, so to figure out if that's happened,
	  // peek at the next word and see if it is the other trailer word.

          uint32_t peekWord;
          fInfile.read((char *) &peekWord, sizeof peekWord);
          if (fInfile.eof()) {
            // shouldn't happen unless the event is truncated in some way
            return -1;
          }
	  
	  // If we did find the other half of the trailer, then react appropriately...
          if ((peekWord & 0xff000000) == 0xa0000000) {
            bheader = false;
            Time = n1;
            if (Time < fLastTime) {
              ++fTimeMult;
            }
            fLastTime = Time;
            Time = Time + 86400000 * fTimeMult;

            // but don't forget to decode the first word
            if (n2 != oldn2) DecodeSpyDataFifo(n2, Hits, Errors, DesyncChannels);
          }
          else {
            // OK, it wasn't a trailer. Undo the peek and decode both words
            fInfile.seekg(-sizeof peekWord, std::ios_base::cur);
            if (n2 != oldn2) DecodeSpyDataFifo(n2, Hits, Errors, DesyncChannels);
            if (n1 != oldn1) DecodeSpyDataFifo(n1, Hits, Errors, DesyncChannels);
	  }
	} // not a trailer
      } // after header
    }
  }

  return Hits.size();
}


int PLTBinaryFileReader::ReadEventHitsText (std::vector<PLTHit*>& Hits, unsigned long& Event, uint32_t& Time, uint32_t& BX)
{
  int LastEventNumber = -1;
  int EventNumber = -1;

  if (fInfile.eof()) {
    return -1;
  }

  int Channel, ROC, Row, Col, ADC;
  std::string Line;
  std::istringstream LineStr;
  while (true) {
    std::getline(fInfile, Line);
    LineStr.clear();
    LineStr.str(Line);

    LineStr >> Channel;
    if (LineStr.eof()) {
      break;
    }
    LineStr >> ROC >> Col >> Row >> ADC >> EventNumber;


    if (EventNumber != LastEventNumber && LastEventNumber != -1) {
      fInfile.putback('\n');
      for (std::string::reverse_iterator It = Line.rbegin(); It != Line.rend(); ++It) {
        fInfile.putback(*It);
      }
      break;
    }

    if ( !IsPixelMasked( Channel*100000 + ROC*10000 + Col*100 + Row ) ) {
      PLTHit* Hit = new PLTHit(Channel, ROC, Col, Row, ADC);
      // only keep hits on the diamond
      if (PLTPlane::IsFiducial(fPlaneFiducialRegion, Hit)) {
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

// This is basically the same as ReadEventHitsBinary but without the code to fix the breakage from
// old versions of FEDStreamReader, since this shouldn't have to deal with that. It might be possible
// to merge these two together but since the hard work is done in DecodeSpyDataFifo() anyway, that's
// really what I'm (successfully) trying to avoid to duplicate.

int PLTBinaryFileReader::ReadEventHitsBuffer(uint32_t* buf, uint32_t bufSize, std::vector<PLTHit*>& Hits, std::vector<PLTError>& Errors, unsigned long& Event, uint32_t& Time, uint32_t& BX, std::vector<int>& DesyncChannels)
{
  if (bufSize % 2 != 0) {
    std::cerr << "Event buffer is not of even size (" << bufSize << " words)" << std::endl;
  }

  bool inEvent = false;
  for (uint32_t i=0; i<bufSize; i+=2) {
    if ((buf[i] == 0x53333333) && (buf[i+1] == 0x53333333)) {
      //tdc buffer, special handling
      i+=2;
      while ((buf[i] & 0xf0000000) != 0xa0000000 && i<bufSize) {
	i++;
      }
      if (i>=bufSize) return -1;
      i++;
    } else if (inEvent == false && (buf[i] & 0xff) == 0 && (buf[i+1] & 0xff000000) == 0x50000000) {
      // header
      if (i!=0)
	std::cerr << "Found header at unexpected location in buffer (" << i << "/" << bufSize << ")" << std::endl;
      inEvent = true;
      Event = buf[i+1] & 0xffffff;
      //std::cout << "Found Event Header: " << Event << std::endl;
      BX = ((buf[i] & 0xfff00000) >> 20);
      fFEDID = ((buf[i] & 0xfff00) >> 8);
    } else if ((buf[i+1] & 0xf0000000) == 0xa0000000) {
      if (i!=bufSize-2)
	std::cerr << "Found trailer at unexpected location in buffer (" << i << "/" << bufSize << ")" << std::endl;
      if (inEvent == false)
	std::cerr << "Found trailer before header" << std::endl;
      inEvent = false;
      Time = buf[i];
    } else {
      if (inEvent == false)
	std::cerr << "Found hit data before header" << std::endl;
      // neither header nor trailer; decode appropriately
      DecodeSpyDataFifo(buf[i], Hits, Errors, DesyncChannels);
      DecodeSpyDataFifo(buf[i+1], Hits, Errors, DesyncChannels);
    }
    if (i == 0 && inEvent == false) {
      std::cerr << "Start of buffer wasn't the header (got " << std::hex << buf[i] << " " << buf[i+1]
		<< " instead)" << std::dec << std::endl;
    }
  } // loop over buffer

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

// This is like ReadPixelMask, but reads mask files in the "online" format (i.e. with the hardware
// address instead of FED channel number and with ranges of pixels specified). It needs the gain
// calibration to translate the hardware address into the FED channel number, so you'll have to pass
// it that as well.

void PLTBinaryFileReader::ReadOnlinePixelMask(const std::string maskFileName, const PLTGainCal& gainCal) {
  std::cout << "PLTBinaryFileReader::ReadOnlinePixelMask reading file: " << maskFileName << std::endl;

  std::ifstream maskFile(maskFileName.c_str());
  if (!maskFile.is_open()) {
    std::cerr << "ERROR: cannot open mask file: " << maskFileName << std::endl;
    throw;
  }

  std::string line;
  int mFec, mFecCh, hubId, roc, maskVal;
  std::string colString, rowString;
  while (1) {
    std::getline(maskFile, line);
    if (maskFile.eof()) break;
    if (line.empty()) continue; // skip blank lines
    if (line.at(0) == '#') continue; // skip comment lines
    
    std::istringstream ss(line);
    ss >> mFec >> mFecCh >> hubId >> roc >> colString >> rowString >> maskVal;
    if (ss.fail()) {
      std::cout << "Malformed line in mask file: " << line << std::endl;
      continue;
    }

    // convert hardware ID to fed channel
    int ch = gainCal.GetFEDChannel(mFec, mFecCh, hubId);
    if (ch == -1) continue; // conversion failed -- this is probably a scope in the mask file not actually functional

    // Now do the actual parsing. This code is taken from BaseCalibration::ReadPixelMaskFile in the online software
    int firstCol, lastCol, firstRow, lastRow;

    // check to see if we want a range of columns or just one column
    size_t cdash = colString.find('-');
    std::stringstream logmessage;
    if (cdash == std::string::npos) {
      firstCol = atoi(colString.c_str());
      lastCol = firstCol;
    } else {
      firstCol = atoi(colString.c_str());
      lastCol = atoi(colString.substr(cdash+1).c_str());
    }

    // same, for rows
    size_t rdash = rowString.find('-');
    if (rdash == std::string::npos) {
      firstRow = atoi(rowString.c_str());
      lastRow = firstRow;
    } else {
      firstRow = atoi(rowString.c_str());
      lastRow = atoi(rowString.substr(rdash+1).c_str());
    }

    // Check for validity
    if (firstRow < PLTU::FIRSTROW || firstCol < PLTU::FIRSTCOL || lastRow > PLTU::LASTROW || lastCol > PLTU::LASTCOL) {
      std::cout << "Row or column numbers are out of range; this line will be skipped" << std::endl;
      continue;
    }

    for (int col = firstCol; col <= lastCol; ++col) {
      for (int row = firstRow; row <= lastRow; ++row) {
	if (maskVal == 0) {
	  // fPixelMask contains the masked pixels. So if maskVal == 0 (means "turn this pixel off").
	  // put this pixel into fPixelMask, and if it's 1, take this pixel out of fPixelMask.
	  fPixelMask.insert(ch*100000 + roc*10000 + col*100 + row);
	} else {
	  fPixelMask.erase(ch*100000 + roc*10000 + col*100 + row);
	}	  
      }
    } // column & row loops
  } // line loop

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
