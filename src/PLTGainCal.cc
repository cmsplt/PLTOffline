#include "PLTGainCal.h"



PLTGainCal::PLTGainCal ()
{
  fIsGood = false;
}

PLTGainCal::PLTGainCal (std::string const GainCalFileName, int const NParams)
{
  fIsGood = false;
  if (NParams == 5) {
    ReadGainCalFile5(GainCalFileName);
  } else if (NParams == 3) {
    ReadGainCalFile3(GainCalFileName);
  } else {
    std::cerr << "ERROR: I have no idea how many params you have" << std::endl;
    throw;
  }
}


PLTGainCal::~PLTGainCal ()
{
}


int PLTGainCal::RowIndex (int const i)
{
  return i - PLTU::FIRSTROW;
}


int PLTGainCal::ColIndex (int const i)
{
  return i - PLTU::FIRSTCOL;
}


int PLTGainCal::ChIndex (int const i)
{
  return i - 1;
}


int PLTGainCal::RocIndex (int const i)
{
  return i;
}


float PLTGainCal::GetCoef(int const i, int const ch, int const roc, int const col, int const row)
{
  // Get a coef, note roc number is 0, 1, 2
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
  // Get charge, note roc number is 0, 1, 2
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

  float const charge = 65. * (TMath::Power( (float) adc, 2) * GC[ich][iroc][icol][irow][0] + (float) adc * GC[ich][iroc][icol][irow][1] + GC[ich][iroc][icol][irow][2]
                       + (GC[ich][iroc][icol][irow][4] != 0 ? TMath::Exp( (adc - GC[ich][iroc][icol][irow][3]) / GC[ich][iroc][icol][irow][4] ) : 0)
                       );
  //printf("ich %2i  iroc %1i  icol %2i  row %2i  irow %2i  charge %12.3E\n", ich, iroc, icol, row, irow, charge);
  //printf("%12.3E %12.3E %12.3E %12.3E %12.3E\n",  GC[ich][iroc][icol][irow][0],  GC[ich][iroc][icol][irow][1],  GC[ich][iroc][icol][irow][2],  GC[ich][iroc][icol][irow][3],  GC[ich][iroc][icol][irow][3]);
  if (PLTGainCal::DEBUGLEVEL) {
    printf("%2i %1i %2i %2i %4i %10.1f\n", ch, roc, col, row, adc, charge);
  }
  //printf("%4i  %10.1f\n", adc, charge);
  return charge;
}

void PLTGainCal::ReadGainCalFile (std::string const GainCalFileName, int const NParams)
{
  if (GainCalFileName == "") {
    fIsGood = false;
    return;
  }

  if (NParams == 5) {
    ReadGainCalFile5(GainCalFileName);
  } else if (NParams == 3) {
    ReadGainCalFile3(GainCalFileName);
  } else {
    std::cerr << "ERROR: I have no idea how many params you have" << std::endl;
    throw;
  }

  return;
}


void PLTGainCal::ReadGainCalFile5 (std::string const GainCalFileName)
{
  int mFec, mFecChannel, hubAddress, row, col, roc, ch;
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
      for (int k = 0; k != PLTU::NCOL; ++k) {
        for (int m = 0; m != PLTU::NROW; ++m) {
          for (int n = 0; n != 5; ++n) {
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
    mFec = mFecChannel = hubAddress = row = col = ch = 0;
    ss >> mFec >> mFecChannel >> hubAddress >> roc >> col >> row;

    if (mFecChannel == 1) {
      if (hubAddress == 5) {
        ch = 13;
      } else if (hubAddress == 13) {
        ch = 99;
      } else if (hubAddress == 21) {
        ch = 14;
      } else if (hubAddress == 29) {
        ch = 15;
      } else {
        std::cerr << "ERROR: I don't recognize this hubAddress: " << hubAddress << std::endl;
        continue;
      }
    } else if (mFecChannel == 2) {
      if (hubAddress == 5) {
        ch = 16;
      } else if (hubAddress == 13) {
        ch = 99;
      } else if (hubAddress == 21) {
        ch = 17;
      } else if (hubAddress == 29) {
        ch = 99;
      } else {
        std::cerr << "ERROR: I don't recognize this hubAddress: " << hubAddress << std::endl;
        continue;
      }
    }


    if (ch  >= MAXCHNS) { printf("ERROR: over MAXCHNS %i\n", ch); };
    if (row >= MAXROWS) { printf("ERROR: over MAXROWS %i\n", row); };
    if (col >= MAXCOLS) { printf("ERROR: over MAXCOLS %i\n", col); };
    if (roc >= MAXROCS) { printf("ERROR: over MAXROCS %i\n", roc); };
    if (PLTGainCal::DEBUGLEVEL) {
      printf("%i %i %i\n", ch, row, col);
    }

    irow = RowIndex(row);
    icol = ColIndex(col);
    ich  = ChIndex(ch);

    if (irow < 0 || icol < 0 || ich < 0) {
      continue;
    }

    ss >> GC[ich][roc][icol][irow][0]
       >> GC[ich][roc][icol][irow][1]
       >> GC[ich][roc][icol][irow][2]
       >> GC[ich][roc][icol][irow][3]
       >> GC[ich][roc][icol][irow][4];

    // dude, you really don't want to do this..
    if (PLTGainCal::DEBUGLEVEL) {
      for (int i = 0; i != 3; ++i) {
        for (int j = 0; j != 5; ++j) {
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


void PLTGainCal::CheckGainCalFile(std::string const GainCalFileName, int const Channel)
{
  ReadGainCalFile5(GainCalFileName);

  for (int j = 0; j != NROCS; ++j) {
    for (int k = 0; k != PLTU::NCOL; ++k) {
      for (int m = 0; m != PLTU::NROW; ++m) {
        if (
          GC[Channel][j][k][m][0] == 0 &&
          GC[Channel][j][k][m][1] == 0 &&
          GC[Channel][j][k][m][2] == 0 &&
          GC[Channel][j][k][m][3] == 0 &&
          GC[Channel][j][k][m][4] == 0) {
          printf("Missing Coefs: Ch %2i  Roc %1i  Col %2i  Row %2i\n", Channel, j, k, m);
        }
      }
    }
  }
  return;
}


void PLTGainCal::PrintGainCal5 ()
{
  // dude, you really don't want to do this..
  for (int ich = 19; ich <= 23; ++ich) {
    for (int iroc = 0; iroc != 3; ++iroc) {
      for (int icol = 0; icol != 26; ++icol) {
        for (int irow = 0; irow != 40; ++irow) {

          for (int j = 0; j != 5; ++j) {
            printf("%6.2E ", GC[ich][iroc][icol][irow][j]);
          }
          printf("\n");
        }
      }
    }
  }

  return;
}

void PLTGainCal::ReadGainCalFile3 (std::string const GainCalFileName)
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
      for (int k = 0; k != PLTU::NCOL; ++k) {
        for (int m = 0; m != PLTU::NROW; ++m) {
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
