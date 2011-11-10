#include "PLTAlignment.h"

#include <map>


PLTAlignment::PLTAlignment ()
{
  fIsGood = false;
}


PLTAlignment::~PLTAlignment ()
{
}


void PLTAlignment::ReadAlignmentFile (std::string const InFileName)
{
  // So far so good..
  fIsGood = true;


  std::map<int, TelescopeAlignmentStruct> TeleMap;


  // Open file
  std::ifstream InFile(InFileName.c_str());
  if (!InFile.is_open()) {
    fIsGood = false;
    std::cerr << "ERROR: cannot open alignment constants filename: " << InFileName << std::endl;
    throw;
  }

  // Read each line in file
  int Channel, ROC;
  for (std::string InLine; std::getline(InFile, InLine); ) {
    if (InLine.size() < 1) {
      continue;
    }
    if (InLine.at(0) == '#') {
      continue;
    }

    //std::cout << InLine << std::endl;
    std::istringstream LineStream;
    LineStream.str(InLine);

    LineStream >> Channel >> ROC;
    std::pair<int, int> CHROC = std::make_pair<int, int>(Channel, ROC);

    // read one line at a time
    float R, X, Y, Z;
    LineStream >> R
               >> X
               >> Y
               >> Z;

    // If the ROC is -1 it is telescope coords, 0,1,2 are ROCs, anything else is bad.
    if (ROC == -1) {
      TeleMap[Channel].GR = R;
      TeleMap[Channel].GX = X;
      TeleMap[Channel].GY = Y;
      TeleMap[Channel].GZ = Z;
    } else if (ROC == 0 || ROC == 1 || ROC == 2) {
      if (TeleMap.count(Channel) == 0) {
        std::cerr << "ERROR: Telescope coords not defined which must be defined before ROCs in alignment file" << std::endl;
        throw;
      }

      // Construct the alignment obj
      CP C;
      C.LR = R;
      C.LX = X;
      C.LY = Y;
      C.LZ = Z;
      C.GR = TeleMap[Channel].GR;
      C.GX = TeleMap[Channel].GX;
      C.GY = TeleMap[Channel].GY;
      C.GZ = TeleMap[Channel].GZ;

      // Save this to alignment map
      fConstantMap[CHROC] = C;
    } else {
      std::cerr << "WARNING: Alignment file contains things I do not recognize: " << InLine << std::endl;
    }

  }

  // Close input file
  InFile.close();

  return;
}


void PLTAlignment::PrintAlignmentFile (std::string const OutFileName)
{
  // Open output file
  FILE* Out = fopen(OutFileName.c_str(), "w");
  if (!Out) {
    std::cerr << "ERROR: cannot open file: " << OutFileName << std::endl;
    throw;
  }

  for (std::map< std::pair<int, int>, CP >::iterator it = fConstantMap.begin(); it != fConstantMap.end(); ++it) {
    int const Channel = it->first.first;
    int const ROC     = it->first.second;
    CP C = it->second;
    fprintf(Out, "%2i  %1i %15.6E %15.6E %15.6E %15.6E %15.6E %15.6E %15.6E\n",
        Channel,
        ROC,
        C.LR,
        C.LX,
        C.LY,
        C.GR,
        C.GX,
        C.GY,
        C.GZ);
  }

  fclose(Out);

  return;
}


bool PLTAlignment::IsGood ()
{
  return fIsGood;
}


float PLTAlignment::PXtoLX (int const px)
{
  return PLTU::PIXELWIDTH * ((px + 0.0000001) - 25.5);
}

float PLTAlignment::PYtoLY (int const py)
{
  return PLTU::PIXELHEIGHT * ((py + 0.0000001) - 59.5);
}

int PLTAlignment::PXfromLX (float const lx)
{
  return (int) (lx / PLTU::PIXELWIDTH + 25.5);
}

int PLTAlignment::PYfromLY (float const ly)
{
  return (int) (ly /  PLTU::PIXELHEIGHT + 59.5);
}

std::pair<int, int> PLTAlignment::PXYfromLXY (std::pair<float, float> const& LXY)
{
  return std::make_pair<int, int>( PXfromLX(LXY.first), PYfromLY(LXY.second));
}

std::pair<float, float> PLTAlignment::PXYDistFromLXYDist (std::pair<float, float> const& LXYDist)
{
  return std::make_pair<float, float>( LXYDist.first / (float)  PLTU::PIXELWIDTH, LXYDist.second / (float)  PLTU::PIXELHEIGHT);
}


void PLTAlignment::AlignHit (PLTHit& Hit)
{
  // Grab the constants and check that they are there..
  CP* C = GetCP(Hit.Channel(), Hit.ROC());
  if (C == 0x0) {
    std::cerr << "ERROR: This is not in the aligment constants map: Channel:" << Hit.Channel() << "  ROC:" << Hit.ROC() << std::endl;
    return;
  }

  int const PX = Hit.Column();
  int const PY = Hit.Row();

  // set w.r.t. center of diamond
  float LX = PXtoLX(PX);
  float LY = PYtoLY(PY);
  float LZ = C->LZ;

  // Start with local rotation
  float const cl = cos(C->LR);
  float const sl = sin(C->LR);
  float TX = LX * cl - LY * sl;
  float TY = LX * sl + LY * cl;
  float TZ = LZ;

  // Local translation
  TX += C->LX;
  TY += C->LY;
  // TZ = TZ =)

  if (DEBUG) {
    printf("TtoL - L XY DIFF %12.3f %12.3f\n",
        TtoLX(TX, TY, Hit.Channel(), Hit.ROC()) - LX,
        TtoLY(TX, TY, Hit.Channel(), Hit.ROC()) - LY);
  }


  // Global rotation
  float const cg = cos(C->GR);
  float const sg = sin(C->GR);
  float GX = TX * cg - TY * sg;
  float GY = TX * sg + TY * cg;
  float GZ = TZ;

  // Global translation
  GX += C->GX;
  GY += C->GY;
  GZ += C->GZ;


  // Set the local, telescope, and global hit coords
  Hit.SetLXY(LX, LY);
  Hit.SetTXYZ(TX, TY, TZ);
  Hit.SetGXYZ(GX, GY, GZ);

  //printf("%12.3E  %12.3E  %12.3E  %12.3E  %12.3E  %12.3E  %12.3E  %12.3E\n", LX, LY, TX, TY, TZ, GX, GY, GZ);

  std::vector<float> TV;
  GtoTXYZ(TV, GX, GY, GZ, Hit.Channel(), Hit.ROC());
  if (DEBUG) {
    printf("GtoT - T XYZ DIFF %12.3f %12.3f %12.3f\n",
        TV[0] - TX, TV[1] - TY, TV[2] - TZ);
  }

  return;
}



float PLTAlignment::GetTZ (int const Channel, int const ROC)
{
  return GetCP(Channel, ROC)->LZ;
}




float PLTAlignment::TtoLX (float const TX, float const TY, int const Channel, int const ROC)
{
  return TtoLXY(TX, TY, Channel, ROC).first;
}


float PLTAlignment::TtoLY (float const TX, float const TY, int const Channel, int const ROC)
{
  return TtoLXY(TX, TY, Channel, ROC).second;
}


std::pair<float, float> PLTAlignment::TtoLXY (float const TX, float const TY, int const Channel, int const ROC)
{
  std::pair<int, int> CHROC = std::make_pair<int, int>(Channel, ROC);
  CP* C = fConstantMap.count(CHROC) == 1 ? &fConstantMap[CHROC] : (CP*) 0x0;

  if (!C) {
    std::cerr << "ERROR: cannot grab the constant mape for this CH ROC: " << CHROC.first << " " << CHROC.second << std::endl;
    return std::make_pair<float, float>(-999, -999);
    throw;
  }


  float const LXA = TX - C->LX;
  float const LYA = TY - C->LY;


  float const cl = cos(C->LR);
  float const sl = sin(C->LR);


  float const LX =  LXA * cl + LYA * sl;
  float const LY = -LXA * sl + LYA * cl;

  //printf("XY DIFF %12.3f  %12.3f\n", TX - LX, TY - LY);

  return std::make_pair<float, float>(LX, LY);
}



void PLTAlignment::GtoTXYZ (std::vector<float>& VOUT, float const GX, float const GY, float const GZ, int const Channel, int const ROC)
{
  // This translates global coordinates back to the telescope coordinates

  // Get the constants for this telescope/plane etc
  std::pair<int, int> CHROC = std::make_pair<int, int>(Channel, ROC);
  CP* C = fConstantMap.count(CHROC) == 1 ? &fConstantMap[CHROC] : (CP*) 0x0;

  if (!C) {
    std::cerr << "ERROR: cannot grab the constant mape for this CH ROC: " << CHROC.first << " " << CHROC.second << std::endl;
    return;
    throw;
  }


  float const GXA = GX - C->GX;
  float const GYA = GY - C->GY;


  float const cl = cos(C->GR);
  float const sl = sin(C->GR);


  float const TX =  GXA * cl + GYA * sl;
  float const TY = -GXA * sl + GYA * cl;
  float const TZ =  GZ - C->GZ;


  VOUT.resize(3);
  VOUT[0] = TX;
  VOUT[1] = TY;
  VOUT[2] = TZ;

  return;
}








float PLTAlignment::LR (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].LR;
}


float PLTAlignment::LX (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].LX;
}


float PLTAlignment::LY (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].LY;
}


float PLTAlignment::LZ (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].LZ;
}


float PLTAlignment::GR (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].GR;
}


float PLTAlignment::GX (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].GX;
}


float PLTAlignment::GY (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].GY;
}


float PLTAlignment::GZ (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].GZ;
}

PLTAlignment::CP* PLTAlignment::GetCP (int const ch, int const roc)
{
  std::pair<int, int> CHROC = std::make_pair<int, int>(ch, roc);
  if (fConstantMap.count(CHROC)) {
    return &fConstantMap[ CHROC ];
  }

  return (CP*) 0x0;
}
