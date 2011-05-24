#include "PLTGainCal.h"



PLTGainCal::PLTGainCal ()
{
  fIsGood = false;
}

PLTGainCal::PLTGainCal (std::string const GainCalFileName)
{
  fIsGood = false;
  ReadGainCalFile(GainCalFileName);
}


PLTGainCal::~PLTGainCal ()
{
}


int PLTGainCal::RowIndex (int const i)
{
  return i - PLTGainCal::IROWMIN;
}


int PLTGainCal::ColIndex (int const i)
{
  return i - PLTGainCal::ICOLMIN;
}


int PLTGainCal::ChIndex (int const i)
{
  return i;
}


int PLTGainCal::RocIndex (int const i)
{
  return i - 1;
}


float PLTGainCal::GetCoef(int const i, int const ch, int const roc, int const col, int const row)
{
  int irow = RowIndex(row);
  int icol = ColIndex(col);
  int ich  = ChIndex(ch);
  int iroc = RocIndex(roc);
  if (irow < 0 || icol < 0 || ich < 0 || iroc < 0) {
    return -9999;
  }

  return GC[ich][iroc][icol][irow][i];
}


void PLTGainCal::SetCharge (PLTHit& Hit)
{
  Hit.SetCharge( GetCharge(Hit.Channel(), Hit.ROC(), Hit.Column(), Hit.Row(), Hit.ADC()) );
  return;
}



float PLTGainCal::GetCharge(int const ch, int const roc, int const col, int const row, int INadc)
{
  int const adc = INadc;
  if (ch  >= MAXCHNS) { printf("ERROR: over MAXCHNS: %i\n", ch); };
  if (row >= MAXROWS) { printf("ERROR: over MAXROWS: %i\n", row); };
  if (col >= MAXCOLS) { printf("ERROR: over MAXCOLS: %i\n", col); };

  int irow = RowIndex(row);
  int icol = ColIndex(col);
  int ich  = ChIndex(ch);
  int iroc = RocIndex(roc);

  if (irow < 0 || icol < 0 || ich < 0 || iroc < 0) {
    return -9999;
  }

  float const charge = 65. * (TMath::Power( (float) adc, 2) * GC[ich][iroc][icol][irow][0] + (float) adc * GC[ich][iroc][icol][irow][1] + GC[ich][iroc][icol][irow][2]);
  if (PLTGainCal::DEBUGLEVEL) {
    printf("%2i %1i %2i %2i %4i %10.1f\n", ch, roc, col, row, adc, charge);
  }
  //printf("%4i  %10.1f\n", adc, charge);
  return charge;
}

void PLTGainCal::ReadGainCalFile (std::string const GainCalFileName)
{
  int ch,row,col;
  int irow;
  int icol;
  int ich;

  ifstream f(GainCalFileName.c_str());
  if (!f) {
    std::cerr << "ERROR: cannot open file: " << GainCalFileName << std::endl;
    throw;
  }

  for (int i = 0; i != NCHNS; ++i) {
    for (int j = 0; j != NROCS; ++j) {
      for (int k = 0; k != NCOLS; ++k) {
        for (int m = 0; m != NROWS; ++m) {
          for (int n = 0; n != 3; ++n) {
            GC[i][j][k][m][n] = 0;
          }
        }
      }
    }
  }

  std::string line;
  std::getline(f, line);
  std::istringstream ss;

  for ( ; std::getline(f, line); ) {
    ss.clear();
    ss.str(line.c_str());
    ch = row = col = 0;
    ss >> ch >> ch >> ch >> col >> row;
    ch = 22;

    if (ch  >= MAXCHNS) { printf("ERROR: over MAXCHNS\n"); };
    if (row >= MAXROWS) { printf("ERROR: over MAXROWS\n"); };
    if (col >= MAXCOLS) { printf("ERROR: over MAXCOLS\n"); };
    if (PLTGainCal::DEBUGLEVEL) {
      printf("%i %i %i\n", ch, row, col);
    }

    irow = RowIndex(row);
    icol = ColIndex(col);
    ich  = ChIndex(ch);

    if (irow < 0 || icol < 0 || ich < 0) {
      continue;
    }

    ss >> GC[ich][0][icol][irow][0]
       >> GC[ich][0][icol][irow][1]
       >> GC[ich][0][icol][irow][2]
       >> GC[ich][1][icol][irow][0]
       >> GC[ich][1][icol][irow][1]
       >> GC[ich][1][icol][irow][2]
       >> GC[ich][2][icol][irow][0]
       >> GC[ich][2][icol][irow][1]
       >> GC[ich][2][icol][irow][2];

    // dude, you really don't want to do this..
    if (PLTGainCal::DEBUGLEVEL) {
      for (int i = 0; i != 3; ++i) {
        for (int j = 0; j != 3; ++j) {
          printf("%6.2E ", GC[ich][i][icol][irow][j]);
        }
      }
      printf("\n");
    }

  }

  // Apparently this file was read no problem...
  fIsGood = true;


  return;
}
