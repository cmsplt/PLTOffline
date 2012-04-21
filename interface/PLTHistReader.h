#ifndef GUARD_PLTHistReader_h
#define GUARD_PLTHistReader_h

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

class PLTHistReader
{
  public:
    PLTHistReader (std::string const);
    ~PLTHistReader ();


    int      GetNextBuffer ();
    uint32_t GetOrbitTime ();
    uint32_t GetOrbit ();
    uint64_t GetTotal ();
    uint64_t GetTotalInChannel (size_t const);
    uint64_t GetTotalInBucket (size_t const);
    int      GetChBucket(int, int);

    int      AverageNext (int const);
    uint64_t GetAverage ();
    uint64_t GetAverageInChannel (size_t const);
    uint64_t GetAverageInBucket (size_t const);
    std::vector<uint32_t>* Channels ();
    std::vector<uint32_t>* Buckets ();

    static int const NBUCKETS = 3564;
    static int const NMAXTELESCOPES = 48;

  private:
    std::ifstream fInFile;

    int fAvgOver;
    uint64_t fHist[NBUCKETS];

    uint32_t fBigBuff[NMAXTELESCOPES][NBUCKETS];
    uint32_t fOrbitTime[NMAXTELESCOPES];
    uint32_t fOrbit[NMAXTELESCOPES];
    
    std::vector<uint32_t> fChannels;
    std::vector<uint32_t> fBuckets;

    uint32_t fAvgBigBuff[NMAXTELESCOPES][NBUCKETS];
    uint32_t fAvgOrbitTime[NMAXTELESCOPES];
    uint32_t fAvgOrbit[NMAXTELESCOPES];
    std::vector<uint32_t> fAvgChannels;


    // Temp internal scrap variables
    uint32_t fTempOrbitTime;
    uint32_t fTempOrbit;
    uint32_t fTempChannel;

};














#endif

