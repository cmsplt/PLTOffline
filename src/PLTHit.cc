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
}



PLTHit::PLTHit (int channel, int roc, int col, int row, int adc)
{
  // make me from some values

  fChannel = channel;
  fROC = roc;
  fColumn = col;
  fRow = row;
  fADC = adc;
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

















