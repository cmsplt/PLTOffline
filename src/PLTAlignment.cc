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


  fTelescopeMap.clear();


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
    float R, RZ, RY, X, Y, Z;


    // If the ROC is -1 it is telescope coords, 0,1,2 are ROCs, anything else is bad.
    if (ROC == -1) {
      LineStream >> RZ
                 >> RY
                 >> X
                 >> Y
                 >> Z;
      fTelescopeMap[Channel].GRZ = RZ;
      fTelescopeMap[Channel].GRY = RY;
      fTelescopeMap[Channel].GX  = X;
      fTelescopeMap[Channel].GY  = Y;
      fTelescopeMap[Channel].GZ  = Z;
    } else if (ROC == 0 || ROC == 1 || ROC == 2) {
      if (fTelescopeMap.count(Channel) == 0) {
        std::cerr << "ERROR: Telescope coords not defined which must be defined before ROCs in alignment file" << std::endl;
        throw;
      }

      LineStream >> R
                 >> X
                 >> Y
                 >> Z;

      // Construct the alignment obj
      CP C;
      C.LR = R;
      C.LX = X;
      C.LY = Y;
      C.LZ = Z;
      C.GRZ = fTelescopeMap[Channel].GRZ;
      C.GRY = fTelescopeMap[Channel].GRY;
      C.GX = fTelescopeMap[Channel].GX;
      C.GY = fTelescopeMap[Channel].GY;
      C.GZ = fTelescopeMap[Channel].GZ;

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


void PLTAlignment::WriteAlignmentFile (std::string const OutFileName)
{
  // Open output file
  FILE* Out = fopen(OutFileName.c_str(), "w");
  if (!Out) {
    std::cerr << "ERROR: cannot open file: " << OutFileName << std::endl;
    throw;
  }

  for (std::map<int, TelescopeAlignmentStruct>::iterator it = fTelescopeMap.begin(); it != fTelescopeMap.end(); ++it) {
    int const Channel = it->first;
    TelescopeAlignmentStruct& Tele = it->second;

    fprintf(Out, "\n");
    fprintf(Out, "%2i  -1        %15E      %15E  %15E  %15E  %15E\n", Channel, Tele.GRZ, Tele.GRY, Tele.GX, Tele.GY, Tele.GZ);

    for (int iroc = 0; iroc != 3; ++iroc) {
      std::pair<int, int> ChROC = std::make_pair<int, int>(Channel, iroc);

      if (!fConstantMap.count(ChROC)) {
        std::cerr << "ERROR: No entry in fConstantMap for Ch ROC: " << Channel << " " << iroc << std::endl;
        continue;
      }

      CP& C = fConstantMap[ChROC];
      fprintf(Out, "%2i   %1i        %15E                       %15E  %15E  %15E\n", Channel, iroc, C.LR, C.LX, C.LY, C.LZ);

    }
  }


  return;
}


bool PLTAlignment::IsGood ()
{
  return fIsGood;
}


float PLTAlignment::PXtoLX (int const px)
{
  return PLTU::PIXELWIDTH * (25.5 - (px - 0.0000001));
}

float PLTAlignment::PYtoLY (int const py)
{
  return PLTU::PIXELHEIGHT * (59.5 - (py - 0.0000001));
}

int PLTAlignment::PXfromLX (float const lx)
{
  return (int) (25.5 - lx / PLTU::PIXELWIDTH);
}

int PLTAlignment::PYfromLY (float const ly)
{
  return (int) (59.5 - ly /  PLTU::PIXELHEIGHT);
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

  std::vector<float> TXYZ;
  LtoTXYZ(TXYZ, LX, LY, Hit.Channel(), Hit.ROC());


  if (DEBUG) {
    printf("TtoL - L XY DIFF %12.3f %12.3f\n",
        TtoLX(TXYZ[0], TXYZ[1], Hit.Channel(), Hit.ROC()) - LX,
        TtoLY(TXYZ[0], TXYZ[1], Hit.Channel(), Hit.ROC()) - LY);
  }

  std::vector<float> GXYZ;
  TtoGXYZ(GXYZ, TXYZ[0], TXYZ[1], TXYZ[2], Hit.Channel(), Hit.ROC());


  // Set the local, telescope, and global hit coords
  Hit.SetLXY(LX, LY);
  Hit.SetTXYZ(TXYZ[0], TXYZ[1], TXYZ[2]);
  Hit.SetGXYZ(GXYZ[0], GXYZ[1], GXYZ[2]);

  //printf("Channel %2i ROC %1i  Col %2i Row %2i  %12.3E  %12.3E - %12.3E  %12.3E  %12.3E - %12.3E  %12.3E  %12.3E\n",
  //    Hit.Channel(), Hit.ROC(), Hit.Column(), Hit.Row(), LX, LY, TXYZ[0], TXYZ[1], TXYZ[2], GXYZ[0], GXYZ[1], GXYZ[2]);

  //std::vector<float> TV;
  //GtoTXYZ(TV, GX, GY, GZ, Hit.Channel(), Hit.ROC());
  //if (DEBUG) {
  //  printf("GtoT - T XYZ DIFF %12.3f %12.3f %12.3f\n",
  //      TV[0] - TX, TV[1] - TY, TV[2] - TZ);
  //}

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


void PLTAlignment::VTtoVGXYZ (std::vector<float>& VOUT, float const TX, float const TY, float const TZ, int const Channel, int const ROC)
{
  // Get the constants for this telescope/plane etc
  std::pair<int, int> CHROC = std::make_pair<int, int>(Channel, ROC);
  CP* C = fConstantMap.count(CHROC) == 1 ? &fConstantMap[CHROC] : (CP*) 0x0;

  if (!C) {
    std::cerr << "ERROR: cannot grab the constant mape for this CH ROC: " << CHROC.first << " " << CHROC.second << std::endl;
    return;
    throw;
  }

  // Global rotation about Zaxis
  float const cgz = cos(C->GRZ);
  float const sgz = sin(C->GRZ);
  float GXZ = TX * cgz - TY * sgz;
  float GYZ = TX * sgz + TY * cgz;
  float GZZ = TZ;


  // Global rotation about Yaxis
  float const cgy = cos(C->GRY);
  float const sgy = sin(C->GRY);
  float GXY = GXZ * cgy + GZZ * sgy;
  float GYY = GYZ;
  float GZY = GZZ * cgy - GXZ * sgy;

  VOUT.resize(3, 0);
  VOUT[0] = GXY;
  VOUT[1] = GYY;
  VOUT[2] = GZY;


  return;
}

void PLTAlignment::LtoTXYZ (std::vector<float>& VOUT, float const LX, float const LY, int const Channel, int const ROC)
{
  std::pair<int, int> CHROC = std::make_pair<int, int>(Channel, ROC);
  CP* C = fConstantMap.count(CHROC) == 1 ? &fConstantMap[CHROC] : (CP*) 0x0;

  if (!C) {
    std::cerr << "ERROR: cannot grab the constant mape for this CH ROC: " << CHROC.first << " " << CHROC.second << std::endl;
    throw;
  }

  // Start with local rotation
  float const cl = cos(C->LR);
  float const sl = sin(C->LR);
  float TX = LX * cl - LY * sl;
  float TY = LX * sl + LY * cl;
  float TZ = C->LZ;

  // Local translation
  TX += C->LX;
  TY += C->LY;

  VOUT.resize(3);
  VOUT[0] = TX;
  VOUT[1] = TY;
  VOUT[2] = TZ;

  return;
}


void PLTAlignment::TtoGXYZ (std::vector<float>& VOUT, float const TX, float const TY, float const TZ, int const Channel, int const ROC)
{
  // Get the constants for this telescope/plane etc
  std::pair<int, int> CHROC = std::make_pair<int, int>(Channel, ROC);
  CP* C = fConstantMap.count(CHROC) == 1 ? &fConstantMap[CHROC] : (CP*) 0x0;

  if (!C) {
    std::cerr << "ERROR: cannot grab the constant mape for this CH ROC: " << CHROC.first << " " << CHROC.second << std::endl;
    return;
    throw;
  }

  // Global rotation about Zaxis
  float const cgz = cos(C->GRZ);
  float const sgz = sin(C->GRZ);
  float GXZ = TX * cgz - TY * sgz;
  float GYZ = TX * sgz + TY * cgz;
  float GZZ = TZ;

  GXZ += C->GX;
  GYZ += C->GY;
  GZZ += C->GZ;


  // Global rotation about Yaxis
  float const cgy = cos(C->GRY);
  float const sgy = sin(C->GRY);
  float GXY = GXZ * cgy + GZZ * sgy;
  float GYY = GYZ;
  float GZY = GZZ * cgy - GXZ * sgy;

  VOUT.resize(3, 0);
  VOUT[0] = GXY;
  VOUT[1] = GYY;
  VOUT[2] = GZY;


  return;
}



void PLTAlignment::LtoGXYZ (std::vector<float>& VOUT, float const LX, float const LY, int const Channel, int const ROC)
{
  std::vector<float> T;
  LtoTXYZ(T, LX, LY, Channel, ROC);
  TtoGXYZ(VOUT, T[0], T[1], T[2], Channel, ROC);
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

  // Global rotation about Yaxis
  float const cgy = cos(C->GRY);
  float const sgy = sin(C->GRY);
  float GXY = GX * cgy - GZ * sgy;
  float GYY = GY;
  float GZY = GZ * cgy + GX * sgy;


  float const GXA = GXY - C->GX;
  float const GYA = GYY - C->GY;
  float const GZA = GZY - C->GZ;


  float const cl = cos(C->GRZ);
  float const sl = sin(C->GRZ);


  float const TX =  GXA * cl + GYA * sl;
  float const TY = -GXA * sl + GYA * cl;
  float const TZ =  GZA;


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


float PLTAlignment::GRZ (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].GRZ;
}


float PLTAlignment::GRY (int const ch, int const roc)
{
  return fConstantMap[ std::make_pair<int, int>(ch, roc) ].GRY;
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

PLTAlignment::CP* PLTAlignment::GetCP (std::pair<int, int> const& CHROC)
{
  if (fConstantMap.count(CHROC)) {
    return &fConstantMap[ CHROC ];
  }

  return (CP*) 0x0;
}

std::vector< std::pair<int, int> > PLTAlignment::GetListOfChannelROCs ()
{
  // returns a vector containing the pixel fed channels for everything in the alignment

  std::vector< std::pair<int, int> > ROCS;
  for (std::map< std::pair<int, int>, CP >::iterator i = fConstantMap.begin(); i != fConstantMap.end(); ++i) {
    ROCS.push_back(i->first);
  }

  return ROCS;
}

std::vector<int> PLTAlignment::GetListOfChannels ()
{
  std::vector<int> Channels;

  for (std::map<int, TelescopeAlignmentStruct>::iterator it = fTelescopeMap.begin(); it != fTelescopeMap.end(); ++it)
  {
    Channels.push_back(it->first);
  }

  return Channels;
}


