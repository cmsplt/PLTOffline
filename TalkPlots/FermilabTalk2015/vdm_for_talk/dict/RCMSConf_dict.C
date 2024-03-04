#include <stdint.h>

namespace HCAL_HLX{

  struct RCMS_CONFIG {

    uint32_t runNumber;
    
    char CMSSW_Tag[32];
    char TriDAS_Tag[32];
    
    uint32_t FirmwareVersion;
    uint32_t ExpectedFirmwareVersion;
    char AddressTablePath[256];
    
    char CfgDBTag[64];
    char CfgDBAccessor[64];
    bool UseConfigDB;
    
    uint32_t DestIPHigh;
    uint32_t DestIPLow;
    
    uint32_t SrcIPBaseHigh;
    uint32_t SrcIPBaseLow;
    
    uint32_t DestMacAddrHigh;
    uint32_t DestMacAddrMed;
    uint32_t DestMacAddrLow;
    
    uint32_t SrcMacAddrHigh;
    uint32_t SrcMacAddrMed;
    uint32_t SrcMacAddrLow;
    
    uint32_t SrcPort;
    uint32_t DestPort;
    
    uint32_t DebugData;
    uint32_t DebugReadout;
    uint32_t DebugSingleCycle;
    
    uint32_t NumOrbits;
    uint32_t OrbitPeriod;
    
    uint32_t Id;
    
    uint32_t TTCBC0Pattern;
    uint32_t TTCSTARTPattern;
    uint32_t TTCSTOPPattern;
    
    uint32_t BC0Delay;
    
    uint32_t OccThresholdLowBottom;
    uint32_t OccThresholdLowTop;
    uint32_t OccThresholdHighBottom;
    uint32_t OccThresholdHighTop;
    
    uint32_t LHCThresholdBottom;
    uint32_t LHCThresholdTop;
    
    uint32_t ETSumCutoffBottom;
    uint32_t ETSumCutoffTop;
    
    uint32_t OCCMaskBottom;
    uint32_t OCCMaskTop;
    
    uint32_t LHCMaskLowBottom;
    uint32_t LHCMaskLowTop;
    uint32_t LHCMaskHighBottom;
    uint32_t LHCMaskHighTop;
    
    uint32_t SumETMaskLowBottom;
    uint32_t SumETMaskLowTop;
    uint32_t SumETMaskHighBottom;
    uint32_t SumETMaskHighTop;
  };

  struct RUN_QUALITY {

    uint32_t runNumber;
    uint32_t sectionNumber;

    int HLX;
    int HFLumi;
    int ECAL;
    int HCAL;
    int Tracker;
    int RPC;
    int DT;
    int CSC;
  };

}//~namespace HCAL_HLX

typedef HCAL_HLX::RCMS_CONFIG RCMS_CONFIG;
typedef HCAL_HLX::RUN_QUALITY RUN_QUALITY;
