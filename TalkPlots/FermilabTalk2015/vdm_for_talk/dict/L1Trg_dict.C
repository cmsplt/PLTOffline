#include <stdint.h>

namespace HCAL_HLX{

  struct LEVEL1_PATH {

    char pathName[128];
    uint64_t counts;
    uint64_t prescale;
  };

  struct LEVEL1_TRIGGER {
    uint32_t runNumber;
    uint32_t sectionNumber; // Lumi section number recorded by the daq.

    uint32_t timestamp;
    uint32_t timestamp_micros;

    uint64_t deadtimecount;

    char GTLumiInfoFormat[32];
   
    LEVEL1_PATH GTAlgo[128];
    LEVEL1_PATH GTTech[64];
  };

}//~namespace HCAL_HLX

void getL1PathAlgo(HCAL_HLX::LEVEL1_TRIGGER trg, HCAL_HLX::LEVEL1_PATH &path, int number){
	path = trg.GTAlgo[number];
	return;
}

void getL1PathTech(HCAL_HLX::LEVEL1_TRIGGER trg, HCAL_HLX::LEVEL1_PATH &path, int number){
	path = trg.GTTech[number];
	return;
}
typedef HCAL_HLX::LEVEL1_TRIGGER LEVEL1_TRIGGER;
typedef HCAL_HLX::LEVEL1_PATH LEVEL1_PATH;
