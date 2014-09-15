#ifndef GUARD_PSIGainInterpolator_h
#define GUARD_PSIGainInterpolator_h

#include <string>
#include <vector>
#include <map>

#include "PLTHit.h"

class PSIGainInterpolator
{
  public:
    PSIGainInterpolator ();
    ~PSIGainInterpolator ();

    enum InterpoleratorAlgorithm {
      kInterpoleratorAlgorithm_Linear,
      kInterpoleratorAlgorithm_Other
    };

    bool ReadFile (std::string const, int const);
    void SetInterpoleratorAlgorithm (InterpoleratorAlgorithm const);

    float GetCharge (int const, int const, int const, int const, int const);
    void  SetCharge(PLTHit&);
    float GetLinearInterpolation (int const, int const, int const, int const, int);
    float GetInterpolation (int const, int const, int const, int const, int);


  private:
    std::map<int, std::vector<int> > fCalibrationMap;
    std::vector<int> fVCalValues;
    InterpoleratorAlgorithm fInterpoleratorAlgorithm;

};



































#endif
