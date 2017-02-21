#include "PLTHistReader.h"

PLTHistReader::PLTHistReader (std::string const InFileName)
{
  fInFile.open(InFileName.c_str(), std::ios::in | std::ios::binary);
  if (!fInFile.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    throw;
  }
  
  do {
    GetNextBuffer();
  } while (fTempOrbit < 25000);
}


PLTHistReader::~PLTHistReader ()
{
}




std::vector<uint32_t>* PLTHistReader::Channels ()
{
  return &fChannels;
}


std::vector<uint32_t>* PLTHistReader::Buckets ()
{
  return &fBuckets;
}


int PLTHistReader::GetNextBuffer ()
{
  fChannels.clear();
  fBuckets.clear();
  static uint32_t LastOrbitTime = 0;
  static uint32_t MyOrbitTime;
  if (LastOrbitTime == 0) {
    fInFile.read( (char*) &MyOrbitTime, sizeof(uint32_t));
    fTempOrbitTime = MyOrbitTime;
  }

  if (fInFile.eof()) {
    return -1;
  }


  //for (int i = 0; i != NMAXTELESCOPES; ++i) {
  //  for (int j = 0; j != NBUCKETS; ++j) {
  //    fBigBuff[i][j] = 0;
  //  }
  //  fOrbitTime[i] = 0;
  //  fOrbit[i] = 0;
  //}

  do {
    fInFile.read( (char*) &fTempOrbit, sizeof(uint32_t));
    fInFile.read( (char*) &fTempChannel, sizeof(uint32_t));
    fInFile.read( (char*) fBigBuff[fTempChannel], NBUCKETS * sizeof(uint32_t));

    fOrbitTime[fTempChannel] = fTempOrbitTime;
    fOrbit[fTempChannel] = fTempOrbit;
    fChannels.push_back(fTempChannel);

    for (int ib = 0; ib < NBUCKETS; ++ib) {
      if ((fBigBuff[fTempChannel][ib] & 0xfff) != 0) {
        fBuckets.push_back(ib);
      }
    }
    std::sort(fBuckets.begin(), fBuckets.end());
    std::unique(fBuckets.begin(), fBuckets.end());


    if (fInFile.eof()) {
      return -1;
    }

    fInFile.read( (char*) &fTempOrbitTime, sizeof(uint32_t));

  } while (fTempOrbitTime == MyOrbitTime);

  LastOrbitTime = fTempOrbitTime;
  MyOrbitTime = fTempOrbitTime;

  return (int) fChannels.size();
}


uint32_t PLTHistReader::GetOrbitTime ()
{
  return fOrbitTime[fChannels[0]];
}

uint32_t PLTHistReader::GetOrbit ()
{
  return fOrbit[fChannels[0]];
}



int PLTHistReader::AverageNext (int const NToAvg)
{
  for (int i = 0; i < NToAvg; ++i) {
    if (GetNextBuffer() == -1) {
      return -1;
    }

    for (std::vector<uint32_t>::iterator ich = fChannels.begin(); ich != fChannels.end(); ++ich) {
      //fAvgHist[*ich];
    }
  }

  return 0;
}




int PLTHistReader::GetChBucket (int ic, int ib)
{
  return (fBigBuff[ic][ib] & 0xfff);
}

uint64_t PLTHistReader::GetTotal ()
{
  uint64_t Sum = 0;
  for (size_t ich = 0; ich != fChannels.size(); ++ich) {
    for (int i = 0; i != NBUCKETS; ++i) {
      Sum += (fBigBuff[ich][i] & 0xfff);
    }
  }

  return Sum;
}

uint64_t PLTHistReader::GetTotalInChannel (size_t const Channel)
{
  uint64_t Sum = 0;
  for (int i = 0; i != NBUCKETS; ++i) {
    Sum += (fBigBuff[Channel][i] & 0xfff);
  }

  return Sum;
}

uint64_t PLTHistReader::GetTotalInBucket (size_t const Bucket)
{
  uint64_t Sum = 0;
  for (size_t ich = 0; ich != fChannels.size(); ++ich) {
    Sum += (fBigBuff[ich][Bucket] & 0xfff);
  }

  return Sum;
}

