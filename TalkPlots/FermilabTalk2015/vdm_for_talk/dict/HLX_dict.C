#define HCAL_HLX_MAX_BUNCHES 4096
#define HCAL_HLX_MAX_HLXS 36

#include <stdint.h>

namespace HCAL_HLX{

  struct LUMI_SUMMARY {

    float DeadTimeNormalization; 
    float LHCNormalization; // recieved from LHC 

    float InstantLumi;
    float InstantLumiErr;
    int16_t InstantLumiQlty;

    float InstantETLumi;
    float InstantETLumiErr;
    int16_t InstantETLumiQlty;
    float ETNormalization;  // Calculated

    float InstantOccLumi[2];
    float InstantOccLumiErr[2];
    int16_t InstantOccLumiQlty[2];
    float OccNormalization[2];

    float lumiNoise[2];
  };

  struct LUMI_DETAIL {

    float LHCLumi[HCAL_HLX_MAX_BUNCHES]; // Sum of LHC.data over all HLX's

    float ETLumi[HCAL_HLX_MAX_BUNCHES];
    float ETLumiErr[HCAL_HLX_MAX_BUNCHES];
    int16_t ETLumiQlty[HCAL_HLX_MAX_BUNCHES];
    float ETBXNormalization[HCAL_HLX_MAX_BUNCHES];

    float OccLumi[2*HCAL_HLX_MAX_BUNCHES];
    float OccLumiErr[2*HCAL_HLX_MAX_BUNCHES];
    int16_t OccLumiQlty[2*HCAL_HLX_MAX_BUNCHES];
    float OccBXNormalization[2*HCAL_HLX_MAX_BUNCHES];
  };

  struct LUMI_SECTION_HEADER {
    uint32_t timestamp;
    uint32_t timestamp_micros;

    uint32_t  runNumber;   // Run number
    uint32_t  sectionNumber; // Section number

    uint32_t  startOrbit;  // Start orbit of lumi section
    uint32_t  numOrbits;   // Total number of orbits recorded in lumi section
    uint16_t  numBunches;  // Total number of bunches (from start of orbit)
    uint16_t  numHLXs;     // Number of HLXs in lumi section

    bool bCMSLive;    // Is CMS taking data?
    bool bOC0;        // Was section initialised by an OC0?
  };

  struct LUMI_SECTION_SUB_HEADER {
    uint32_t  numNibbles;  // Number of nibbles in this histogram
    bool bIsComplete; // Is this histogram complete (i.e. no missing nibbles)
  };

  struct ET_SUM_SECTION {
    LUMI_SECTION_SUB_HEADER hdr;
    float data[HCAL_HLX_MAX_BUNCHES];
  };

  struct OCCUPANCY_SECTION {
    LUMI_SECTION_SUB_HEADER hdr;
    uint32_t data[6*HCAL_HLX_MAX_BUNCHES];
  };

  struct LHC_SECTION {
    LUMI_SECTION_SUB_HEADER hdr;
    uint32_t data[HCAL_HLX_MAX_BUNCHES];
  };

  struct LUMI_SECTION {
    LUMI_SECTION_HEADER Header;
    LUMI_SUMMARY Summary;
    LUMI_DETAIL  Detail;

    ET_SUM_SECTION etSum[HCAL_HLX_MAX_HLXS];
    OCCUPANCY_SECTION occupancy[HCAL_HLX_MAX_HLXS];
    LHC_SECTION lhc[HCAL_HLX_MAX_HLXS];
  };


}//~namespace HCAL_HLX

typedef HCAL_HLX::LUMI_SECTION LUMI_SECTION;
typedef HCAL_HLX::LUMI_SUMMARY LUMI_SUMMARY;
typedef HCAL_HLX::LUMI_DETAIL LUMI_DETAIL;
typedef HCAL_HLX::LUMI_SECTION_HEADER LUMI_SECTION_HEADER;
typedef HCAL_HLX::ET_SUM_SECTION ET_SUM_SECTION;
typedef HCAL_HLX::OCCUPANCY_SECTION OCCUPANCY_SECTION;
