////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Mar  4 17:13:09 CET 2013
//
////////////////////////////////////////////////////////////////////
#ifndef GUARD_PLTTesterReader_H
#define GUARD_PLTTesterReader_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <cstdlib>
#include <algorithm>

#include "PLTHit.h"
#include "PLTPlane.h"

#include "TFile.h"
#include "TH1I.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TLine.h"
#include "TMarker.h"

class PLTTesterReader
{
  public:
    PLTTesterReader ();
    ~PLTTesterReader ();

    void SetPlaneFiducialRegion (PLTPlane::FiducialRegion);
    void SetOutDir (TString const);
    void SetOutRootFile (TFile*);
    void SetupHistograms ();
    void MakePlots ();
    bool OpenFile (std::string const);
    int  CalculateLevels (std::string const&, int const);
    int  ReadEventHits (std::vector<PLTHit*>& Hits, int&, int&, int&);


  private:
    int LevelInfo (int const);
    std::pair<int, int> fill_pixel_info(int*, int);

    int LevelsROC[6];
    int EventData[205];
    static int const UBThreshold = 1200;
    static int const NMAXUB = 6;

    TString fOutDir;

  private:
    std::string fFileName;
    std::ifstream fFile;
    TFile* fOutRoot;
    PLTPlane::FiducialRegion fPlaneFiducialRegion;



  //H istograms
  private:
    TH1I* hROCUBLevels;
    TH1I* hTBMUBLevels;
    TH1I* hROCLevels;
    TH1I* hTBMLevels;




};

#endif
