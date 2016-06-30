////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Wed June 30
//
// Group track parameters per TrackID==TimeStamp
//
////////////////////////////////////////////////////////////////////
// This is a modified version of the generic class template that
// root generates via MakeClass:
// You feed in the Dip File, you get back TrackParams per intervals
//
//
////////////////////////////////////////////////////////////////////

#define trackClass_cxx
#include "trackClass.h"
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
#include <numeric>

////////////:::Usage::://///////////
//   In a ROOT session, you can do:
//      Root > .L trackClass.C+
//      Root > trackClass t
//      Root > t.Loop("./dip5013.txt");
//


void trackClass::Loop(std::string const DipFile)
{


  if (fChain == 0) return;


  int timestamp;
  int timeOffset = 1465603200;

  std::map<int, std::pair<int,int> > timeStamp;

  int min1Now = 0;
  int min1Last = -1;


  Long64_t nentries = fChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;

    timestamp = timeOffset + EventT/1000; //convert milliseconds since day start to Epoch time


    double hrs = (((EventT/1000.)/60)/60);
    double mins = (hrs - std::floor(hrs))*60;
    double secs = (mins - std::floor(mins))*60;

    if(jentry == 0) {
      std::cout << "Time Start: " << std::floor(hrs) <<" hrs " << std::floor(mins)
                << " mins " << secs << " secs " << endl;
      //                << timestamp<< std::endl;
    }
    if (jentry == nentries - 1){
      std::cout << "Time Stop: " << std::floor(hrs) <<" hrs " << std::floor(mins)
                << " mins " << secs << " secs \n" << endl;
          //                << timestamp<< std::endl;
    }


    min1Now = TrackID;
    if (min1Now > min1Last) {
      timeStamp[TrackID] = std::make_pair(timestamp, timestamp+60);
      //timeFrom to timeFrom + 1 minute because TrackParams
      //are grouped by 1 minute intervals
      //      std::cout << TrackID << " " << timestamp << " " << timestamp+60 << "\n";
    }
    min1Last = TrackID;
  }


  //Read Dip Text File, Look at timeStamp map, group dip lumi values to
  // corresponding TrackID as a vector so that you can divide the lumi sum
  // by the vector size later.

  Int_t nlWBM = 0;
  int row, col, dipTime;
  Float_t lumi;

  ifstream wbm;

  wbm.open(Form(DipFile.c_str()));
  std::map<int, std::vector<double> > timeHZ;

  while (1) {
    wbm >> row >> col >> dipTime >> lumi;
    //    std::cout << row << " " << col << " " << dipTime << " " << lumi << std::endl;
    if (!wbm.good()) break;
    if (nlWBM < 5) printf("dipTime=%8d, Lumi=%8f\n",dipTime, lumi);
    nlWBM++;

    // Does this timeValue in dipFile belong to any timeStamp value from Slink?
    for (std::map<int,  std::pair<int,int> >::iterator it = timeStamp.begin(); it != timeStamp.end(); ++it) {
      if (dipTime >= timeStamp[it->first].first && dipTime <= timeStamp[it->first].second) {
        timeHZ[it->first].push_back(lumi);
        //        std::cout <<it->first<< ": " << timeStamp[it->first].first << " " << dipTime <<  " \n";
      }

    }

  }


  printf("found %d points @ DipFile\n",nlWBM);

  wbm.close();
  std::cout << "DipFile is grouped into " << timeHZ.size() << " chunks"<< "\n";


  // Now, assing SBIL value to Slink TrackParams


  // Caution: Do not trust first and the last value from Slink.
  // This is because when grouping Track Parameters by minute intervals,
  // the first and last timeStamps can't be guaranteed to be 1 minute long

  // So, while generating the SBIL values, skip the first and last index of
  // timeStamp as following

  int counter = 1; //skip first and map.size()th :)
                   //good thing that maps are ordered
  int mpsz = timeHZ.size();


  ofstream SBIL;
  SBIL.open("trackSBIL.txt");
  
  for (std::map<int,  std::vector<double> >::iterator it = timeHZ.begin(); it != timeHZ.end(); ++it) {
    double sum_of_elems = std::accumulate(timeHZ[it->first].begin(), timeHZ[it->first].end(), 0.0);
    // std::cout << it->first << " " << sum_of_elems*1.0/(2028*timeHZ[it->first].size())
    //           <<" "<< timeHZ[it->first].size() << " " << counter <<"\n";

    if (counter > 1 && counter < mpsz){
      SBIL << it->first << " " << sum_of_elems*1.0/(2028*timeHZ[it->first].size())
           << " " << counter<< "\n";
    }
    counter++;
  }
  SBIL.close();
  //  std::cout << "Done! Wrote to trackSBIL.txt" << "\n";
  std::cout << "Done writing to trackSBIL.txt, master! \n Would you care for some tea?" << "\n";

}
