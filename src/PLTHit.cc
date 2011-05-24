#include "PLTHit.h"

PLTHit::PLTHit ()
{
}

PLTHit::PLTHit (std::string& Line)
{
  std::istringstream ss;
  ss.str(Line.c_str());
  ss >> fChannel
     >> fROC
     >> fColumn
     >> fRow
     >> fADC
     >> fEvent;
  fCharge = -1;
  //fCharge = fGainCal->GetCharge(fChannel, fROC, fColumn, fRow, fADC);
}



PLTHit::PLTHit (int channel, int roc, int col, int row, int adc)
{
  fChannel = channel;
  fROC = roc;
  fColumn = col;
  fRow = row;
  fADC = adc;
}


PLTHit::~PLTHit ()
{
}

void PLTHit::SetCharge (float const in)
{
  fCharge = in;
  return;
}


bool PLTHit::MatchesColumnRow (PLTHit* Hit)
{
  if (fColumn == Hit->Column() && fRow == Hit->Row()) {
    return true;
  }
  return false;
}

int PLTHit::Channel ()
{
  return fChannel;
}

int PLTHit::ROC ()
{
  return fROC;
}

int PLTHit::Row ()
{
  return fRow;
}

int PLTHit::Column ()
{
  return fColumn;
}

float PLTHit::ADC ()
{
  return fADC;
}

float PLTHit::Charge ()
{
  return fCharge;
}

float PLTHit::Event ()
{
  return fEvent;
}
















