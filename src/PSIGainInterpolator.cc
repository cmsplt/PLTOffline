#include "PSIGainInterpolator.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "TString.h"

PSIGainInterpolator::PSIGainInterpolator ()
{
  SetInterpoleratorAlgorithm(kInterpoleratorAlgorithm_Linear);
}



PSIGainInterpolator::~PSIGainInterpolator ()
{
}



bool PSIGainInterpolator::ReadFile (std::string const InFileName, int const roc)
{
  // Open the input file
  std::ifstream f(InFileName.c_str());
  if (!f) {
    std::cerr << "ERROR; Cannot open file: " << InFileName << std::endl;
    throw;
  }

  // Clear previous values
  fVCalValues.clear();


  // For PSI board we always use ch1
  int const ch = 1;

  std::string OneLine;
  std::string LowRangeLine;
  std::string HighRangeLine;
  std::getline(f, OneLine);
  std::getline(f, LowRangeLine);
  std::getline(f, HighRangeLine);
  std::getline(f, OneLine);

  int VCalValue;
  std::istringstream is;

  is.str(LowRangeLine);
  is >> OneLine >> OneLine;
  while (is >> VCalValue) {
    fVCalValues.push_back(VCalValue);
  }
  is.clear();
  is.str(HighRangeLine);
  is >> OneLine >> OneLine;
  while (is >> VCalValue) {
    fVCalValues.push_back(7*VCalValue);
  }

  size_t const NPoints = fVCalValues.size();
  std::cout << "NPoints = " << NPoints << std::endl;

  // Loop over all lines in the input data file
  std::istringstream s;
  TString Pix;
  int row, col;
  for (std::string line; std::getline(f, line); ) {
    s.clear();
    s.str(line);

    std::string adcstring;
    int adc;
    std::vector<int> TempVec;
    for (size_t i = 0; i != NPoints; ++i) {
      s >> adcstring;
      if (adcstring != "N/A") {
        adc = atoi(adcstring.c_str());
        TempVec.push_back(adc);
        //printf(" %5i", adc);
      } else {
        //std::cout << "   N/A";
        TempVec.push_back(-999999);
      }
    }
    s >> Pix >> col >> row;

    int const id = row + 100*col * 10000*roc * 100000*ch;
    fCalibrationMap[id] = TempVec;
  }

  return true;
}



void PSIGainInterpolator::SetInterpoleratorAlgorithm (InterpoleratorAlgorithm const in)
{
  fInterpoleratorAlgorithm = in;
  return;
}






float PSIGainInterpolator::GetCharge (int const ch, int const roc, int const col, int const row, int const adc)
{
  switch (fInterpoleratorAlgorithm) {
    case kInterpoleratorAlgorithm_Linear:
      return 65. * GetLinearInterpolation(ch, roc, col, row, adc);
      break;
    case kInterpoleratorAlgorithm_Other:
      return 65. * GetInterpolation(ch, roc, col, row, adc);
      break;
    default:
      std::cerr << "ERROR: No fInterpoleratorAlgorithm selected" << std::endl;
      throw;
  }
  return 0.0;
}



void PSIGainInterpolator::SetCharge(PLTHit& Hit)
{
  Hit.SetCharge( GetCharge(Hit.Channel(), Hit.ROC(), Hit.Column(), Hit.Row(), Hit.ADC()) );
  return;
}




float PSIGainInterpolator::GetLinearInterpolation (int const ch, int const roc, int const col, int const row, int adc)
{
  int Low  =  999999;
  int High = -999999;
  int LowIndex  = -1;
  int HighIndex = -1;

  int const id = row + 100*col * 10000*roc * 100000*ch;

  std::vector<int>& V = fCalibrationMap[id];

  if (V.size() < 2) {
    std::cerr << "WARNING: Not enough points for interpolation.  I will return 0.0" << std::endl;
    return 0.0;
  }

  for (size_t i = 0; i != V.size(); ++i) {
    if (V[i] == -999999) {
      continue;
    }
    if (V[i] <= adc) {
      Low = V[i];
      LowIndex = i;
    } 
    if (V[i] >= adc) {
      High = V[i];
      HighIndex = i;
      break;
    }
  }

//  std:: cout << "Low Index = " << LowIndex << " High Index = " << HighIndex << std::endl;
//  std:: cout << "Low  = " << Low << " High  = " << High << std::endl;

  if (LowIndex >= 0 && HighIndex > 0) {
    float const Slope = ( (float) fVCalValues[HighIndex] - (float) fVCalValues[LowIndex]) / ((float) High - (float) Low );
    return (float) fVCalValues[LowIndex] + Slope * (float) (adc - Low);
  } else if (LowIndex >= 1) {
    //std::cerr << "WARNING: Extrapolation on high end used" << std::endl;
    float const Slope = ( (float) fVCalValues[LowIndex] - (float) fVCalValues[LowIndex - 1]) / ((float) Low - (float) V[LowIndex - 1] );
    return (float) fVCalValues[LowIndex] + Slope * (float) (adc - Low);
  } else if (HighIndex == 0) {
    // std::cerr << "WARNING: Extrapolation on low end used" << std::endl;
    float const Slope = ( (float) fVCalValues[HighIndex+1] - (float) fVCalValues[HighIndex]) / ((float) V[HighIndex + 1] - (float) High );
    // std::cout << High << std::endl;
    return (float) fVCalValues[HighIndex] + Slope * (float) (adc - High);
  } else {
    std::cerr << "ERROR: For some reason you have reached a strange state, or my logic has failed me" << std::endl;
    throw;
  }



}


float PSIGainInterpolator::GetInterpolation (int const ch, int const roc, int const col, int const row, int adc)
{
  return 0.0;
}
