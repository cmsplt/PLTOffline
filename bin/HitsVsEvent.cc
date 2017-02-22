////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Sat Aug 15
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <numeric>

#include "PLTEvent.h"
#include "PLTU.h"


#include "TH1F.h"
#include "TFile.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"



int HitsvsEvent (std::string const DataFileName, int from, int to)
{
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);


  // Map for all ROC hists and canvas



  ofstream myfile ("HitsVsEvents.txt");
  
  if (myfile.is_open()) {

    // Loop over all events in file
    for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
      
      if (ientry <= from * 1000000) continue;
      if (ientry >= to * 1000000) break;
      if (ientry % 10000 == 0) {
        std::cout << "Processing event: " << ientry << std::endl;
      }

      int nHits = Event.NHits();
      
      if (nHits !=0){
        myfile << ientry << " "  << nHits << std::endl;
      }
    }
    myfile.close();  
  } else {
    std::cout << " Couldn't write to a file"<< std::endl;
  }
  
  
  return 0;
}







int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }


  std::string tempStr;
  int from, to;

  std::cout << "Enter event number beginning and end (in millions)"<<std::endl;
  std::cout << " Event From (in millions):: "<<std::endl;
  std::getline (std::cin, tempStr);
  std::stringstream(tempStr) >> from;
  std::cout << "Event to (in millions):: "<<std::endl;
  std::getline (std::cin, tempStr);
  std::stringstream(tempStr) >> to;


  std::string const DataFileName = argv[1];
  HitsvsEvent(DataFileName,from,to);

  return 0;
}
