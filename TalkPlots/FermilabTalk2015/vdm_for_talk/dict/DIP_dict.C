#include <stdint.h>

namespace HCAL_HLX {
 
  struct DIP_STRUCT_BASE { 
    int MessageQuality;     
    uint32_t timestamp;
    uint32_t timestamp_micros;
    uint32_t runNumber;
    uint32_t sectionNumber;
  };

  struct VDM_SCAN_DATA: public DIP_STRUCT_BASE {
    bool VdmMode; 
    int IP;
    bool RecordDataFlag;
    double BeamSeparation;
    bool isXaxis;
    int Beam; 
    double StepProgress;
  };

  struct BEAM_INFO {
    /* DCCT value, per 1.5 s, i.e per short lumi section */
    double averageBeamIntensity;

    /* FBCT with 1.5 s granularity */
    float averageBunchIntensities[3564];
    int beamConfig[3564];
    double bestLifeTime;
    float orbitFrequency;
    double primitiveLifetime;
    double totalIntensity;
  };
  
  struct BRANA_INFO {
    double meanCrossingAngle;
    int acqMode;
    double meanLuminosity;
    double bunchByBunchLuminosity[3564];
  };

  struct BRANP_INFO {
    double meanCrossingAngle;
    int acqMode;
    int counterAcquisition;
    double meanLuminosity;
    double meanCrossingAngleError;
    double meanLuminosityError;
    double bunchByBunchLuminosity[3564];
  };

  struct BRAN_INFO {
    BRANA_INFO branA;
    BRANP_INFO branP;
  };

  struct DIP_COMBINED_DATA: public DIP_STRUCT_BASE {
    char beamMode[128];
    float Energy;
    uint32_t FillNumber;
    BEAM_INFO Beam[2];
    BRAN_INFO BRAN4L2;
    BRAN_INFO BRAN4R2;
    BRAN_INFO BRAN4L5;
    BRAN_INFO BRAN4R5;
    VDM_SCAN_DATA VdMScan;
  };

}

void getBeam(HCAL_HLX::DIP_COMBINED_DATA dip, HCAL_HLX::BEAM_INFO &beam, int number){
	beam = dip.Beam[number];
	return;
}

typedef HCAL_HLX::DIP_COMBINED_DATA DIP_COMBINED_DATA;
typedef HCAL_HLX::BEAM_INFO BEAM_INFO;
