////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Wed June 19
//
// Group track parameters per ScanPoint
//
////////////////////////////////////////////////////////////////////
// This is a modified version of the generic class template that
// root generates via MakeClass:
// * root -l TrackParams4954.root
// * Tracks->MakeClass("VdMClass")
// You feed in the ScanPoint information, you get back TrackParams
// per Scan Point
//
////////////////////////////////////////////////////////////////////

#define VdMClass_cxx
#include "VdMClass.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

#include <map>
#include <vector>
#include "Riostream.h"
#include "THStack.h"
#include "TGraphErrors.h"
#include <TGraph.h>
#include <TFrame.h>
#include <TF1.h>
#include <cmath>
#include <fstream>

////////////:::Usage::://///////////
// root -l
// .L VdMClass.C+
// VdMClass m
// m.Loop("scanY1")

void VdMClass::Loop(std::string scpFile)
{
  if (fChain == 0) return;



  std::string frname = std::string("./t4954")+scpFile+".root";
  TFile f(frname.c_str(),"RECREATE");

  Int_t pt, ch, nbx;
  Float_t sx, sy, bsx, bsy, rx, ry;

  //make ttree with per ScanPoint track information
  TTree *perSCP = new TTree("perSCP","perSCP");

  perSCP->Branch("nbx", &nbx,"nbx/I");
  perSCP->Branch("ch", &ch,"ch/I");
  perSCP->Branch("pt", &pt,"pt/I");
  perSCP->Branch("sx", &sx,"sx/F");
  perSCP->Branch("sy", &sy,"sy/F");
  perSCP->Branch("bsx", &bsx,"bsx/F");
  perSCP->Branch("bsy", &bsy,"bsy/F");
  perSCP->Branch("rx", &rx,"rx/F");
  perSCP->Branch("ry", &ry,"ry/F");


  // Containers for Scan Point information==timeFrom, timeTo, nominalSeparation
  std::map<int, double> scSep;
  std::map<int, std::vector<int> > scTiming;
  for (int i = 1; i <= 25; i++){
    scTiming[i].resize(2);
  }

  // get scanpoint information from file

  int ptNum;
  double tFrom,tTo,dSep;
  ifstream myfile;
  std::string sfname = std::string("./")+scpFile+".txt";
  myfile.open(sfname.c_str());
  int nlines = 0;
  while(1) {
    myfile >> ptNum >> tFrom >> tTo >> dSep;
    if (!myfile.good()) break;
    if (nlines < 5) printf("ptNum=%3i, tFrom=%8f, tTo=%8f,dSep=%8f\n",ptNum,tFrom,tTo,dSep);

    scTiming[ptNum][0] = int(tFrom);
    scTiming[ptNum][1] = int(tTo);

    scSep[ptNum] = dSep;
    nlines++;

  }
  myfile.close();


  int timestamp;
  int timeOffset = 1464307200; // 00 Epoch time @05/27/2016<==Date for Fill 4954

  int ptIndex = -99;


  // Loop ttree from TrackParams
  Long64_t nentries = fChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;


    timestamp = timeOffset + EventT/1000; //convert ms since day start to Epoch time


    double hrs = (((EventT/1000.)/60)/60);
    double mins = (hrs - std::floor(hrs))*60;
    double secs = (mins - std::floor(mins))*60;

    if(jentry == 1 || jentry == nentries-1) {
      std::cout << "Time: " << std::floor(hrs) <<" hrs " << std::floor(mins)
                << " mins " << secs << " secs "
                << timestamp<< std::endl;
    }

    // Check if this timeStamp from TrackParams corresponds to any timeStamps
    // from ScanPoint File. If it does, label that track by that ScanPoint
    for (std::map<int, std::vector<int> >::iterator it = scTiming.begin(); it != scTiming.end(); ++it) {
      if (timestamp >= it->second[0] && timestamp <=  it->second[1]){
        ptIndex = it->first;
      }
    }
      if (ptIndex != -99) {
        rx = *ResidualX;
        ry = *ResidualY;

        nbx = EvntBX;
        ch = Channel;
        pt = ptIndex;
        sx = *SlopeX;
        sy = *SlopeY;
        bsx = *BeamspotX;
        bsy = *BeamspotY;
        perSCP->Fill();
      }

      ptIndex = -99;
    }//jentry
  perSCP->Write();
  }//Loop
