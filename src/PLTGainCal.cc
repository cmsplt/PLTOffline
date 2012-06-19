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

  float charge = -9999;
  if (fNParams == 3) {
    charge = 65. * (float(adc * adc) * GC[ich][iroc][icol][irow][2] + float(adc) * GC[ich][iroc][icol][irow][1] + GC[ich][iroc][icol][irow][0]);

  } else if (fNParams == 5) {
    charge = 65. * (TMath::Power( (float) adc, 2) * GC[ich][iroc][icol][irow][0] + (float) adc * GC[ich][iroc][icol][irow][1] + GC[ich][iroc][icol][irow][2]
                    + (GC[ich][iroc][icol][irow][4] != 0 ? TMath::Exp( (adc - GC[ich][iroc][icol][irow][3]) / GC[ich][iroc][icol][irow][4] ) : 0)
                   );
  }
  if (PLTGainCal::DEBUGLEVEL) {
    printf("%2i %1i %2i %2i %4i %10.1f\n", ch, roc, col, row, adc, charge);
  }

  return charge;
}

void PLTGainCal::ReadGainCalFile (std::string const GainCalFileName)
{
  if (GainCalFileName == "") {
    fIsGood = false;
    return;
  }

  std::ifstream InFile(GainCalFileName.c_str());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open gaincal file: " << GainCalFileName << std::endl;
    throw;
  }

  // Loop over header lines in the input data file
  for (std::string line; std::getline(InFile, line); ) {
    if (line == "") {
      break;
    }
  }



  std::string line;
  std::getline(InFile, line);
  std::istringstream linestream;
  linestream.str(line);
  int i = -4;
  for (float junk; linestream >> junk; ++i) {
  }
  InFile.close();
  fNParams = i;

  printf("PLTGainCal sees a parameter file with %i params\n", fNParams);

  if (fNParams == 5) {
    ReadGainCalFile5(GainCalFileName);
  } else if (fNParams == 3) {
    ReadGainCalFile3(GainCalFileName);
  } else {
    std::cerr << "ERROR: I have no idea how many params you have" << std::endl;
    throw;
  }

  return;
}

int PLTGainCal::GetHardwareID (int const Channel)
{ 
  return fHardwareMap[Channel];
}

void PLTGainCal::ReadGainCalFile5 (std::string const GainCalFileName)
{
  int ch, row, col, roc;
  int irow;
  int icol;
  int ich;

  ifstream f(GainCalFileName.c_str());
  if (!f) {
    std::cerr << "ERROR: cannot open file: " << GainCalFileName << std::endl;
    throw;
  }

  // Loop over header lines in the input data file
  for (std::string line; std::getline(f, line); ) {
    int mf, mfc, hub;
    if (line == "") {
      break;
    }
    std::istringstream ss;
    ss.str(line);
    ss >> mf >> mfc >> hub >> ch;

    fHardwareMap[ch] = 1000*mf + 100*mfc + hub;
    printf("Adding ch %i -> %i %i %i\n", ch, mf, mfc, hub);
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
    ch = row = col = 0;
    ss >> ch >> roc >> col >> row;

    // Just remember that on the plane tester it's channel 22

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
  ReadGainCalFile(GainCalFileName);

  int const ich  = ChIndex(Channel);

  int NMissing = 0;
  int NTotal = 0;

  for (int j = 0; j != NROCS; ++j) {
    for (int k = 0; k != PLTU::NCOL; ++k) {
      for (int m = 0; m != PLTU::NROW; ++m) {
        ++NTotal;
        if (
          GC[ich][j][k][m][0] == 0 &&
          GC[ich][j][k][m][1] == 0 &&
          GC[ich][j][k][m][2] == 0 &&
          GC[ich][j][k][m][3] == 0 &&
          GC[ich][j][k][m][4] == 0) {
          printf("Missing Coefs: iCh %2i  iRoc %1i  iCol %2i  iRow %2i\n", ich, j, k, m);
          ++NMissing;
        }
      }
    }
  }

  printf("Number missing is: %i / %i = %E\n", NMissing, NTotal, (float) NMissing / (float) NTotal);

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
  int mFec, mFecChannel, hubAddress;
  int ch, row, col, roc;
  int irow;
  int icol;
  int ich;
  int iroc;

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


  // Loop over header lines in the input data file
  for (std::string line; std::getline(f, line); ) {
    if (line == "") {
      break;
    }
  }

  std::istringstream ss;
  for (std::string line ; std::getline(f, line); ) {
    ss.clear();
    ss.str(line.c_str());
    ss >> mFec >> mFecChannel >> hubAddress >> roc >> col >> row;

    if (ch  >= MAXCHNS) { printf("ERROR: over MAXCHNS\n"); };
    if (row >= MAXROWS) { printf("ERROR: over MAXROWS\n"); };
    if (col >= MAXCOLS) { printf("ERROR: over MAXCOLS\n"); };
    if (PLTGainCal::DEBUGLEVEL) {
      printf("%i %i %i\n", ch, row, col);
    }

    irow = RowIndex(row);
    icol = ColIndex(col);
    ich  = ChIndex(ch);
    iroc = RocIndex(roc);

    if (irow < 0 || icol < 0 || ich < 0) {
      continue;
    }

    ss >> GC[ich][iroc][icol][irow][0]
       >> GC[ich][iroc][icol][irow][1]
       >> GC[ich][iroc][icol][irow][2];

    // dude, you really don't want to do this..
    if (PLTGainCal::DEBUGLEVEL) {
      for (int i = 0; i != 3; ++i) {
        printf("%1i %1i %2i %1i %2i %2i", mFec, mFecChannel, hubAddress, roc, col, row);
        for (int j = 0; j != 3; ++j) {
          printf(" %9.1E", GC[ich][i][icol][irow][j]);
        }
        printf("\n");
      }
    }
  }

  // Apparently this file was read no problem...
  fIsGood = true;

  return;
}
