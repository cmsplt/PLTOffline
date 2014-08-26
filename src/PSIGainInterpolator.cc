#include "PSIGainInterpolator.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "TString.h"

#define DEBUG false

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

  // IMPORTANT !!!
  // For Telescope 1 and 2 the calibration order is
  // 50 / 100 / 150 / 200 / 250/ 210/ 350 / 490 / 630 / 1400
  // to make sure everything works we need to exchange the values of
  // pos 4 and 5 of the TemVec and the fCalValues vector
  // TODO: Need to change that when Vcal values change
  // !!!!!!!!!!!!!!!!

   float tmp = fVCalValues[4];
   fVCalValues[4] = fVCalValues[5];
   fVCalValues[5] = tmp;

   std::cout << "fCalValues" << std::endl;
   for (size_t i = 0; i != fVCalValues.size(); ++i)
      std::cout << fVCalValues[i] << " ";
   std::cout << std::endl;


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

    int const id = row + 100*col + 10000*roc + 1000000*ch;

    if (DEBUG)
        std::cout << row << " " << 100*col << " " << 10000*roc << " " << 100000*ch << " " << row + 100*col * 10000*roc * 100000*ch << " -> " << id << std::endl ;

    // IMPORTANT !!!
    // For Telescope 1 and 2 the calibration order is
    // 50 / 100 / 150 / 200 / 250/ 210/ 350 / 490 / 630 / 1400
    // to make sure everything works we need to exchange the values of
    // pos 4 and 5 of the TemVec and the fCalValues vector
    // TODO: Need to change that when Vcal values change
    // !!!!!!!!!!!!!!!!

    tmp = TempVec[4];
    TempVec[4] = TempVec[5];
    TempVec[5] = tmp;

    std::cout << "TempVec" << std::endl;
    for (size_t i = 0; i != TempVec.size(); ++i)
	std::cout << TempVec[i] << " ";
    std::cout << std::endl;


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

  int const id = row + 100*col + 10000*roc + 1000000*ch;

  std::vector<int>& V = fCalibrationMap[id];

  if (V.size() < 2) {
    std::cerr << "WARNING: Not enough points for interpolation.  I will return 0.0" << std::endl;
    return 0.0;
  }

  for (size_t i = 0; i != V.size(); ++i) {
    if (V[i] == -999999) {
      continue;
    }
    if (V[i] == adc){
        return fVCalValues[i];
    }

    if (V[i] < adc) {
      Low = V[i];
      LowIndex = i;
    } 
    if (V[i] > adc) {
      High = V[i];
      HighIndex = i;
      break;
    }
  }

  float return_charge = -1;

  if (LowIndex >= 0 && HighIndex > 0) {
    float const Slope = ( (float) fVCalValues[HighIndex] - (float) fVCalValues[LowIndex]) / ((float) High - (float) Low );
    return_charge = (float) fVCalValues[LowIndex] + Slope * (float) (adc - Low);
  }
  else if (LowIndex >= 1) {
    //std::cerr << "WARNING: Extrapolation on high end used" << std::endl;
    float const Slope = ( (float) fVCalValues[LowIndex] - (float) fVCalValues[LowIndex - 1]) / ((float) Low - (float) V[LowIndex - 1] );
    return_charge = (float) fVCalValues[LowIndex] + Slope * (float) (adc - Low);
  }
  else if (HighIndex == 0) {
    //std::cerr << "WARNING: Extrapolation on low end used" << std::endl;
    float const Slope = ( (float) fVCalValues[HighIndex+1] - (float) fVCalValues[HighIndex]) / ((float) V[HighIndex + 1] - (float) High );
    return_charge =  (float) fVCalValues[HighIndex] + Slope * (float) (adc - High);
  }
  else {
    std::cerr << "ERROR: For some reason you have reached a strange state, or my logic has failed me" << std::endl;
    throw;
  }



  if (DEBUG){
    for (size_t i = 0; i != V.size(); ++i)
        std::cout << V[i] << " ";
    std::cout << std::endl;

    std::cout << "roc | col | row " << roc << " | " << col << " | " << row << std::endl;

    std::cout << "low | high | adc: " << Low << " | " << High << " | " <<  adc << std::endl;

    std::cout << "val[low]| val[high] " << fVCalValues[LowIndex] << " | " << fVCalValues[HighIndex] << std::endl;

    std::cout << "val[low-1]| val[low] " << fVCalValues[LowIndex-1] << " | " << fVCalValues[LowIndex] << std::endl;

    std::cout << "val[high]| val[high+1] " << fVCalValues[HighIndex] << " | " << fVCalValues[HighIndex+1] << std::endl;

    std::cout << "Returning: " << return_charge << std::endl;
  }

  return return_charge;




}


float PSIGainInterpolator::GetInterpolation (int const ch, int const roc, int const col, int const row, int adc)
{
  return 0.0;
}
