////////////////////////////////////////////////////////////////////////////////
// Authors: Nicholas Luczak
// Description: PicoScopeADC header
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_PICOSCOPEADC_H
#define INCLUDED_PICOSCOPEADC_H

#include "BufferedADC.h"
#include <vector>
#include <cstdint>
#include <string>

// Use the official Pico SDK types & macros.
#include "ps5000aApi.h"

class PicoScopeADC : public BufferedADC
{
 public:
  PicoScopeADC();
  ~PicoScopeADC() override;

  void OnPublish() override;
  void OnAutoConfig() override;
  void OnPreflight( SignalProperties& Output ) const override;
  void OnInitialize( const SignalProperties& Output ) override;
  void OnStartAcquisition() override;
  void DoAcquire( GenericSignal& Output ) override;
  void OnStopAcquisition() override;

  void OnTrigger( int ); // optional async trigger

 private:
  // Allow the SDK callback to update internal fields
  friend void PREF4 streamingCallback(
    int16_t handle,
    int32_t noOfSamples,
    uint32_t startIndex,
    int16_t overflow,
    uint32_t triggerAt,
    int16_t triggered,
    int16_t autoStop,
    void* pParameter);

  // ---- Pico 5000 device state ----
  int16_t mHandle;                         // Pico device handle
  PS5000A_DEVICE_RESOLUTION mResolution;   // Selected resolution
  int16_t mMaxADCValue;                    // Max ADC code at current resolution
  int mNumberOfSignalChannels;             // A..D
  PS5000A_RANGE mRange;                    // Single range for all channels (simplified)
  PS5000A_COUPLING mCoupling;              // DC/AC

  // ---- Streaming configuration ----
  uint32_t mOverviewBufferSize;
  uint32_t mSampleInterval;                // in time units below
  PS5000A_TIME_UNITS mTimeUnits;
  uint32_t mDownsampleRatio;
  PS5000A_RATIO_MODE mRatioMode;

  // ---- Streaming buffers ----
  std::vector<int16_t*> mDrvMax;           // driver "max" buffers (one per channel)
  std::vector<int16_t*> mDrvMin;           // driver "min" buffers (unused but required by API)
  struct FIFO {
    std::vector<int16_t> data;
  };
  std::vector<FIFO> mFifos;                // per-channel FIFO of unconsumed samples

  // ---- Streaming control ----
  volatile bool mReady;
  int32_t  mSampleCountFromCallback;
  uint32_t mStartIndexFromCallback;
  int16_t  mOverflowFromCallback;
  bool     mAutoStopped;

  // Utility
  void allocateBuffers();
  void freeBuffers();
  void ensureFifoCapacity(size_t perChannelReserve);
  double rangeMv(PS5000A_RANGE r) const;
};

#endif // INCLUDED_PICOSCOPEADC_H
