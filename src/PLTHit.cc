#include "PLTHit.h"

PLTHit::PLTHit ()
{
}

PLTHit::PLTHit (std::string& Line)
{
  // This is to read a hit from a text file

  std::istringstream ss;
  ss.str(Line.c_str());
  ss >> fChannel
     >> fROC
     >> fColumn
     >> fRow
     >> fADC;
     //>> Event; // Was at end of line, but that's not what dean wants for breakfast
  fCharge = -1; // The gaincal is somewhere else now.. just deal


  // Local X and Y defined from center of diamond
  //fLX = fColumn - 26.5;
  //fLY = fRow - 59.5;
}



PLTHit::PLTHit (int channel, int roc, int col, int row, int adc)
{
  // make me from some values

  fChannel = channel;
  fROC = roc;
  fColumn = col;
  fRow = row;
  fADC = adc;

  fCharge = -1; // The gaincal is somewhere else now.. just deal

  // Local X and Y defined from center of diamond
  //fLX = fColumn - 26.5;
  //fLY = fRow - 59.5;
}


PLTHit::~PLTHit ()
{
  // Bye!
}

void PLTHit::SetCharge (float const in)
{
  // Set the charge
  fCharge = in;
  return;
}


void PLTHit::SetLXY (float const X, float const Y)
{
  fLX = X;
  fLY = Y;
  return;
}


void PLTHit::SetTXYZ (float const X, float const Y, float const Z)
{
  fTX = X;
  fTY = Y;
  fTZ = Z;
  return;
}


void PLTHit::SetGXYZ (float const X, float const Y, float const Z)
{
  fGX = X;
  fGY = Y;
  fGZ = Z;
  return;
}


bool PLTHit::MatchesColumnRow (PLTHit* Hit)
{
  // Just a hit match checker...
  if (fColumn == Hit->Column() && fRow == Hit->Row()) {
    return true;
  }
  return false;
}

int PLTHit::Channel ()
{
  // Get the channel
  return fChannel;
}

int PLTHit::ROC ()
{
  // Get the ROC for this hit
  return fROC;
}

int PLTHit::Row ()
{
  // Get row for this hit
  return fRow;
}

int PLTHit::Column ()
{
  // Get column for this hit
  return fColumn;
}

int PLTHit::ADC ()
{
  // Get adc counts for this hit
  return fADC;
}

float PLTHit::Charge ()
{
  // Get the charge for this hit
  return fCharge;
}



float PLTHit::LX ()
{
  return fLX;
}


float PLTHit::LY()
{
  return fLY;
}



float PLTHit::TX ()
{
  return fTX;
}


float PLTHit::TY()
{
  return fTY;
}


float PLTHit::TZ()
{
  return fTZ;
}



float PLTHit::GX ()
{
  return fGX;
}


float PLTHit::GY()
{
  return fGY;
}


float PLTHit::GZ()
{
  return fGZ;
}













