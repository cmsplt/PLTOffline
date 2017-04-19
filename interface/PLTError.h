#ifndef GUARD_PLTError_h
#define GUARD_PLTError_h

// A very simple container class to hold information about a single
// error reported in the data stream. There are three fields:
// 1) channel: FED channel number that the error is in (only meaningful for
//   time out, event number error, FED trailer error, or TBM error)
// 2) errorType: what kind of error it is
// 3) errorDetails: more information on the error:
//   -- for time out, the time out counter
//   -- for event number error, the TBM event number
//   -- for near full, the bit field showing the specific FIFO information
//   -- for FED trailer error, the 4-bit nibble with the individual error bits
//   -- for TBM error, the 8-bit word with the individual error bits
// See PLTError::Print() for a little more details on what the error bits mean.

#include <stdint.h>

typedef enum ErrorTypeEnum {
  kTimeOut,
  kEventNumberError,
  kNearFull,
  kFEDTrailerError,
  kTBMError,
  kUnknownError
} ErrorType;

class PLTError {
public:
  PLTError(uint32_t channel, ErrorType errorType, uint32_t errorDetails);
  ~PLTError();

  // Getters
  uint32_t GetChannel() const { return fChannel; }
  ErrorType GetErrorType() const { return fErrorType; }
  uint32_t GetErrorDetails() const { return fErrorDetails; }

  // Print out detailed error information
  void Print(void) const;

private:
  uint32_t fChannel;
  ErrorType fErrorType;
  uint32_t fErrorDetails;
};

#endif

