////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue May 24 09:54:58 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>
#include <numeric>

#include "PLTEvent.h"
#include "PLTU.h"
#include "zmq.hpp"

#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"

// CODE BELOW

int SendPixelHits(std::string const DataFileName)
{

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);

  PLTPlane::FiducialRegion MyFiducialRegion = PLTPlane::kFiducialRegion_All;
  //  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(MyFiducialRegion);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  int ientry = 1;
  bool isoff = false;

  zmq::context_t m_context(1);
  zmq::socket_t zmq_socket(m_context, ZMQ_PUB);

  uint32_t rows[500];
  uint32_t cols[500];
  uint32_t chas[500];
  uint32_t rocs[500];
  uint32_t adcs[500];

  char zmq_address[100];
  sprintf(zmq_address, "tcp://*:%d", 7776);

  zmq_socket.bind(zmq_address);
  // Loop over all events in file
  do {
    
    int status = Event.GetNextEvent();
//    if(isoff) std::cout << status << std::endl;
    if(status < 0)
      {
	if(!isoff){
//	std::cout << "Going into off mode" << std::endl;
	isoff = true;}
	usleep(100);
      }
    else
      {
	memset(rows,0,500*sizeof(uint32_t));
        memset(cols,0,500*sizeof(uint32_t));
        memset(chas,0,500*sizeof(uint32_t));
        memset(rocs,0,500*sizeof(uint32_t));
        memset(adcs,0,500*sizeof(uint32_t));

	uint32_t PixHits = Event.NHits();

	zmq::message_t zmq_PixelEvent(2501*sizeof(uint32_t));
//	std::cout << PixHits << std::endl;
	std::vector<PLTHit*> EventHits = Event.fHits;
	if(PixHits > 500) PixHits = 500;
	for(unsigned int ihits = 0; ihits < PixHits; ihits++){
	  rows[ihits] = EventHits[ihits]->Row();
	  cols[ihits] = EventHits[ihits]->Column();
	  chas[ihits] = EventHits[ihits]->Channel();
	  rocs[ihits] = EventHits[ihits]->ROC();
	  adcs[ihits] = EventHits[ihits]->ADC();
	
	}
	
	char * messageBuffer[2501*sizeof(uint32_t)];

	memcpy(messageBuffer, &PixHits, sizeof(uint32_t));
	memcpy((uint32_t*)messageBuffer+1, rows, 500*sizeof(uint32_t));
        memcpy((uint32_t*)messageBuffer+501, cols, 500*sizeof(uint32_t));
        memcpy((uint32_t*)messageBuffer+1001, chas, 500*sizeof(uint32_t));
        memcpy((uint32_t*)messageBuffer+1501, rocs, 500*sizeof(uint32_t));
        memcpy((uint32_t*)messageBuffer+2001, adcs, 500*sizeof(uint32_t));
	memcpy((void*) zmq_PixelEvent.data(), messageBuffer, 2501*sizeof(uint32_t));
	// std::cout << "sending message" << std::endl;
	zmq_socket.send(zmq_PixelEvent);
	if(isoff){
//	std::cout << "going into on mode" << std::endl;
        isoff = false;}
	ientry++;
      }
    
    if(!isoff && (ientry % 100 == 1))    std::cout << "Processing event: " << ientry << " with hits " << Event.NHits() << " and size " << sizeof(Event.fHits) << std::endl;
    sleep(1);
    
    
  }while(1);

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  SendPixelHits(DataFileName);

  return 0;
}
