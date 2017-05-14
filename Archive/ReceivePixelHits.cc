#include "zmq.hpp"
#include <stdint.h>
#include <iostream>
#include <stdio.h>

#include "PLTEvent.h"
#include "PLTU.h"

int main(const int argc, const char** argv) {

  zmq::context_t m_context(1);
  
  zmq::socket_t zmq_socket(m_context, ZMQ_SUB);
  char zmq_address[100];
  sprintf(zmq_address, "tcp://localhost:%d", 7776);
  
  std::cout << "here1" << std::endl;  
  zmq_socket.connect(zmq_address);
  zmq_socket.setsockopt(ZMQ_SUBSCRIBE, 0, 0);
  std::cout << "here2" << std::endl;
  zmq::message_t zmq_PixelEvent(1001*sizeof(uint32_t));
  char * messageBuffer[1001*sizeof(uint32_t)];
  uint32_t NHits;
  uint32_t rows[200];
  uint32_t cols[200];
  uint32_t chas[200];
  uint32_t rocs[200];
  uint32_t adcs[200];

  while (1) {
    std::cout << "here3" << std::endl;
    if(zmq_socket.recv(&zmq_PixelEvent)){
      std::cout << "here4" << std::endl;
      memcpy(messageBuffer, (void*) zmq_PixelEvent.data(), 1001*sizeof(uint32_t));      

      memcpy(&NHits, messageBuffer, sizeof(uint32_t));
      memcpy(&rows, (uint32_t*)messageBuffer+1, 200*sizeof(uint32_t));
      memcpy(&cols, (uint32_t*)messageBuffer+51, 200*sizeof(uint32_t));
      memcpy(&chas, (uint32_t*)messageBuffer+101, 200*sizeof(uint32_t));
      memcpy(&rocs, (uint32_t*)messageBuffer+151, 200*sizeof(uint32_t));
      memcpy(&adcs, (uint32_t*)messageBuffer+201, 200*sizeof(uint32_t));
      std::cout << "Zmq message: received with hits " << NHits << std::endl;

    }
  }    
  return 0;
}
