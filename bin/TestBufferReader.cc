////////////////////////////////////////////////////////////////////
//
// TestBufferReader
// Paul Lujan
// April 21, 2017
//
// A utility for testing the new functionality of PLTEvent to process
// an event from a buffer by setting up a zmq receiver which captures
// events (from TestSlinkSender or from the real FEDStreamReader)
// and just dumps out the output.
//
// Note -- this is pretty verbose so don't run it on a large file
// unless you're doing something with the output other than dumping
// it to screen!
//
// Compilation note: because this won't compile on april (zmq is not
// installed there), this is removed from the Makefile so it won't try
// to compile it by default. To get it to compile, just remove the
// filter-out commands from the Makefile and add -lzmq to LIBS (and
// obviously, do it on a machine with zmq installed).
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>

#include "PLTEvent.h"
#include "zmq.hpp"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE

// CODE BELOW

int TestBufferReader(const char* zmqHost) {
  // Set up the PLTEvent reader
  PLTEvent Event("", kBuffer);
  Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  // Set up the zmq receiver

  zmq::context_t zmq_context(1);
  zmq::socket_t zmq_socket(zmq_context, ZMQ_SUB);
  char zmq_address[100];
  snprintf(zmq_address, 100, "tcp://%s:%d", zmqHost, 7776);

  zmq_socket.connect(zmq_address);
  zmq_socket.setsockopt(ZMQ_SUBSCRIBE, 0, 0);

  uint32_t buf[1024];

  // Listen to events
  while (1) {
    // get message
    int eventSize = zmq_socket.recv((void*)buf, sizeof(buf)*sizeof(uint32_t), 0);

    // feed it to Event.GetNextEvent()

    Event.GetNextEvent(buf, eventSize/sizeof(uint32_t));

    std::cout << "Event " << Event.EventNumber() << " BX " << Event.BX() << " Time " << Event.Time() << std::endl;
    int nhits = 0;
    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {
      PLTPlane* Plane = Event.Plane(ip);
      for (size_t ih = 0; ih != Plane->NHits(); ++ih) {
	PLTHit* Hit = Plane->Hit(ih);
	std::cout << "Hit  " << nhits << " chan " << Hit->Channel() << " ROC " << Hit->ROC() << " row " << Hit->Row() << " col " << Hit->Column() << std::endl;
	nhits++;
      }
    }

    const std::vector<PLTError>& errors = Event.GetErrors();
    if (errors.size() > 0) {
      for (std::vector<PLTError>::const_iterator it = errors.begin(); it != errors.end(); ++it) {
	it->Print();
      }
    }

    std::cout << nhits << " hits and " << errors.size() << " errors" <<  std::endl;
  }

  return 0;
}


int main (int argc, const char* argv[]) {
  const char* defaultHost = "pltslink.cms";
  if (argc > 1)
    TestBufferReader(argv[1]);
  else
    TestBufferReader(defaultHost);

  return 0;
}
