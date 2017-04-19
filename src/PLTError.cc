#include "PLTError.h"
#include <iostream>

PLTError::PLTError(uint32_t channel, ErrorType errorType, uint32_t errorDetails):
  fChannel(channel), fErrorType(errorType), fErrorDetails(errorDetails) {
}

PLTError::~PLTError() {}

// Print out error details for a given error.

void PLTError::Print(void) const {
  switch (fErrorType) {

  case kTimeOut:
    std::cout << "Channel " << fChannel << " time out (counter: " << fErrorDetails << ")" << std::endl;
    break;

  case kEventNumberError:
    std::cout << "Channel " << fChannel << " event number error (TBM event number: " << fErrorDetails << ")" << std::endl;
    break;

  case kNearFull:
    std::cout << "FIFO near full:";
    if (fErrorDetails & 1) std::cout << " Ia";
    if (fErrorDetails & (1 << 1)) std::cout << " Ib";
    if (fErrorDetails & (1 << 2)) std::cout << " Ic";
    if (fErrorDetails & (1 << 3)) std::cout << " Id";
    if (fErrorDetails & (1 << 4)) std::cout << " Ie";

    if (fErrorDetails & (1 << 6)) std::cout << " II";
    if (fErrorDetails & (1 << 7)) std::cout << " L1A";
    std::cout << std::endl;
    break;

  case kFEDTrailerError:
    std::cout << "Channel " << fChannel << " FED trailer error:";
    if (fErrorDetails & 1) std::cout << " data overflow";
    if (fErrorDetails & (3 << 1)) std::cout << " FSM error " << ((fErrorDetails >> 1) & 3);
    if (fErrorDetails & (1 << 3)) std::cout << " invalid # of ROCs";
    std::cout << std::endl;
    break;

  case kTBMError:
    std::cout << "Channel " << fChannel << " TBM error:";
    if (fErrorDetails & 1) std::cout << " stack full";
    if (fErrorDetails & (1 << 1)) std::cout << " precal trigger issued";
    if (fErrorDetails & (1 << 2)) std::cout << " event counter cleared";
    if (fErrorDetails & (1 << 3)) std::cout << " sync trigger";
    if (fErrorDetails & (1 << 4)) std::cout << " sync trigger error";
    if (fErrorDetails & (1 << 5)) std::cout << " ROC reset";
    if (fErrorDetails & (1 << 6)) std::cout << " TBM reset";
    if (fErrorDetails & (1 << 7)) std::cout << " no token pass";
    std::cout << std::endl;
    break;

  case kUnknownError:
    std::cout << "Unknown error" << std::endl;
    break;

  default:
    std::cout << "Unknown error type" << std::endl;
  }
}
