#include "PLTDACs.h"


PLTDAC::PLTDAC (std::string const Dir, std::string const InFileName)
{
  std::ifstream f((Dir+"/"+InFileName).c_str());
  if (!f.is_open()) {
    std::cerr << "ERROR: cannot open dacs file: " << InFileName << std::endl;
    throw;
  }

  sscanf(InFileName.c_str(), "mFec%i_mFecChannel%i_hubAddress%i_roc%i.dacs1", &fmFec, &fmFecChannel, &fhubAddress, &fROC);
  printf("Reading DAC file: mFec: %1i  mFecChannel: %1i  hubAddress: %2i  ROC: %1i\n", fmFec, fmFecChannel, fhubAddress, fROC);

  std::istringstream LineStr;
  int dac, value;
  for (std::string Line; std::getline(f, Line); ) {
    if (Line.size() < 3) {
      continue;
    }
    LineStr.clear();
    LineStr.str(Line);

    LineStr >> dac >> value;
    printf("%9i %9i\n", dac, value);
    fMap[dac] = value;
  }

}

PLTDAC::~PLTDAC ()
{
}


int PLTDAC::Val (int const dac)
{
  if (fMap.count(dac)) {
    return fMap[dac];
  } else {
    std::cerr << "ERROR: asking for DAC which doesn't exist in map!: " << dac << std::endl;
    throw;
  }

  return -999;
}


void PLTDAC::Set (int const dac, int const value)
{
  fMap[dac] = value;
  return;
}


void PLTDAC::Write ()
{
  char BUFF[600];
  sprintf(BUFF, "New_mFec%i_mFecChannel%i_hubAddress%i_roc%i.dacs1", fmFec, fmFecChannel, fhubAddress, fROC);

  FILE* f = fopen(BUFF, "w");
  if (!f) {
    std::cerr << "ERROR: cannot open file for writing: " << BUFF << std::endl;
    return;
  }

  for (std::map<int, int>::iterator It = fMap.begin(); It != fMap.end(); ++It) {
    fprintf(f, "%9i %9i\n", It->first, It->second);
  }

  fclose(f);

  return;
}








PLTDACs::PLTDACs ()
{
}

PLTDACs::~PLTDACs ()
{
}

PLTDAC* PLTDACs::GetDACS (int const mF, int const mFC, int const hub, int const roc)
{
  int const id = PLTDAC::MakeID(mF, mFC, hub, roc);
  if (fMap.count(id)) {
    return fMap[id];
  }

  std::cerr << "WARNING: cannot find DAC file with id: " << id << std::endl;

  return (PLTDAC*) 0x0;
}
