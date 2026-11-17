////////////////////////////////////////////////////////////////////////////////
// Authors: Nicholas Luczak
// Description: PicoScopeADC implementation (tested on PicoScope 5244D model)
////////////////////////////////////////////////////////////////////////////////
#include "PicoScopeADC.h"
#include "PicoStatusText.h"
#include "BCIStream.h"
#include "BCIEvent.h"
#include "ThreadUtils.h"

// Pull in the Pico 5000A API (SDK must be installed, or include path set)
#include "ps5000aApi.h" // Provided with the Pico SDK

#include <cstring>
#include <algorithm>
#include <stdexcept>

// Make the source filter known to the framework.
RegisterFilter( PicoScopeADC, 1 );

// ---- Local helpers (range table) ----
static const uint16_t kInputRanges_mV[PS5000A_MAX_RANGES] = {
  10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000
};

double
PicoScopeADC::rangeMv(PS5000A_RANGE r) const
{
  int idx = static_cast<int>(r);
  if (idx < 0 || idx >= PS5000A_MAX_RANGES) return 1000.0;
  return static_cast<double>(kInputRanges_mV[idx]);
}

PicoScopeADC::PicoScopeADC()
  : mHandle(-1),
    mResolution(PS5000A_DR_12BIT),
    mMaxADCValue(32767),
    mNumberOfSignalChannels(2),
    mRange(PS5000A_5V),
    mCoupling(PS5000A_DC),
    mOverviewBufferSize(200000), // ~200k samples per channel ring
    mSampleInterval(1),          // 1 us default -> 1 MHz max request (device will quantize)
    mTimeUnits(PS5000A_US),
    mDownsampleRatio(1),
    mRatioMode(PS5000A_RATIO_MODE_NONE),
    mReady(false),
    mSampleCountFromCallback(0),
    mStartIndexFromCallback(0),
    mOverflowFromCallback(0),
    mAutoStopped(false)
{
}

PicoScopeADC::~PicoScopeADC()
{
  try
  {
    OnStopAcquisition();
  }
  catch(...) { /* swallow in destructor */ }

  if (mHandle >= 0)
  {
    ps5000aCloseUnit(mHandle);
    mHandle = -1;
  }
}

void
PicoScopeADC::OnPublish()
{
  BEGIN_PARAMETER_DEFINITIONS

    // Core signal properties
      "Source:Signal%20Properties int SourceCh= auto "
      "auto 1 % // The PicoScope can support up to 2 or 4 channels depending on the model",

      "Source:Signal%20Properties int SampleBlockSize= auto "
      "auto 1 % // number of samples transmitted at a time",

      "Source:Signal%20Properties float SamplingRate= auto "
      "auto 0.0 % // Maximum rate: 1GHz at 8bit, 500MHz at 12bit, and 662.5MHz at 16bit",

    // PicoScope setup (enum-like lists: first value is default, rest are allowed options)
    "PicoScope:Setup int Coupling= 2 1 1 2"
      " // Supported Coupling: 1 AC, 2 DC (enumeration)",
    "PicoScope:Setup int Range= 1 1 1 12"
       " // Supported ranges: 1 10mV, 2 20mV, 3 50mV, 4 100mV, 5 200mV, 6 500mV, 7 1V, 8 2V, 9 5V, 10 10V, 11 20V, 12 50V (enumeration)",

    "PicoScope:Setup int Resolution= 1 1 1 5"
       " // ADC resolutions: 1 8bit, 2 12bit, 3 14bit, 4 15bit, 5 16bit (enumeration)",

    // Per-channel names and scaling (auto-sized later to SourceCh)
    "Source:Signal%20Properties list SourceChGain= 1 auto % % "
       " // physical units per raw A/D unit",

    "Source:Signal%20Properties list SourceChOffset= 1 auto % % "
       " // raw A/D offset to subtract, typically 0",

    "Source:Signal%20Properties list ChannelNames= 1 auto % % "
       " // names of amplifier channels",

  END_PARAMETER_DEFINITIONS

  BEGIN_STREAM_DEFINITIONS
    "PicoScopeADCSyncTriggers 8 0",
  END_STREAM_DEFINITIONS

  BEGIN_EVENT_DEFINITIONS
    "PicoScopeADCAsyncTriggers 8 0",
  END_EVENT_DEFINITIONS
}

// Asynchronous trigger (not used here but kept for completeness)
void
PicoScopeADC::OnTrigger( int trigger )
{
  bcievent << "PicoScopeADCAsyncTriggers " << trigger;
}

void
PicoScopeADC::OnAutoConfig()
{
  // (Re-)open device
  if (mHandle >= 0)
  {
    ps5000aCloseUnit(mHandle);
    mHandle = -1;
  }

  PICO_STATUS st = ps5000aOpenUnit(&mHandle, nullptr, PS5000A_DR_12BIT);
  if (st == PICO_POWER_SUPPLY_NOT_CONNECTED ||
      st == PICO_POWER_SUPPLY_CONNECTED ||
      st == PICO_USB3_0_DEVICE_NON_USB3_0_PORT ||
      st == PICO_POWER_SUPPLY_UNDERVOLTAGE)
  {
    // Accept the reported power state and continue
    ps5000aChangePowerSource(mHandle, st);
  }
  else if (st != PICO_OK)
  {
    throw bcierr << "PicoScope: ps5000aOpenUnit failed with status "
        << PicoStatusName((uint32_t)st) << " (0x" << std::hex << (uint32_t)st << std::dec << ") "
        << PicoStatusDescription((uint32_t)st);
  }

  // Resolution
  PS5000A_DEVICE_RESOLUTION res = PS5000A_DR_12BIT;
  int resChoice = Parameter("Resolution");
  if (resChoice == 1) res = PS5000A_DR_8BIT;
  else if (resChoice == 2) res = PS5000A_DR_12BIT;
  else if (resChoice == 3) res = PS5000A_DR_14BIT;
  else if (resChoice == 4) res = PS5000A_DR_15BIT;
  else if (resChoice == 5) res = PS5000A_DR_16BIT;

  st = ps5000aSetDeviceResolution(mHandle, res);
  if (st != PICO_OK)
    throw bcierr << "PicoScope: ps5000aSetDeviceResolution failed, status " 
      << PicoStatusName((uint32_t)st) << " (0x" << std::hex << (uint32_t)st << std::dec << ") "
      << PicoStatusDescription((uint32_t)st);
  mResolution = res;

  // Get max ADC value for current resolution
  st = ps5000aMaximumValue(mHandle, &mMaxADCValue);
  if (st != PICO_OK)
    throw bcierr << "PicoScope: ps5000aMaximumValue failed, status " 
      << PicoStatusName((uint32_t)st) << " (0x" << std::hex << (uint32_t)st << std::dec << ") "
      << PicoStatusDescription((uint32_t)st);

  // Channel count: default to 2. Users can set SourceCh to 2 or 4.
  int channels = 2;
  Parameter("SourceCh") = channels;
  channels = ActualParameter("SourceCh");
  mNumberOfSignalChannels = std::min(std::max(channels, 1), 4);

  // Names, gains, offsets
  Parameter( "ChannelNames" )->SetNumValues( mNumberOfSignalChannels );
  Parameter( "SourceChGain" )->SetNumValues( mNumberOfSignalChannels );
  Parameter( "SourceChOffset" )->SetNumValues( mNumberOfSignalChannels );

  // Range
  int iniRange = Parameter("Range");
  PS5000A_RANGE r = PS5000A_5V;
  if (iniRange == 1) r = PS5000A_10MV;
  else if (iniRange == 2) r = PS5000A_20MV;
  else if (iniRange == 3) r = PS5000A_50MV;
  else if (iniRange == 4) r = PS5000A_100MV;
  else if (iniRange == 5) r = PS5000A_200MV;
  else if (iniRange == 6) r = PS5000A_500MV;
  else if (iniRange == 7) r = PS5000A_1V;
  else if (iniRange == 8) r = PS5000A_2V;
  else if (iniRange == 9) r = PS5000A_5V;
  else if (iniRange == 10) r = PS5000A_10V;
  else if (iniRange == 11) r = PS5000A_20V;
  else if (iniRange == 12) r = PS5000A_50V;
  mRange = r;

  // Coupling
  int cpl = Parameter("Coupling");
  if (cpl == 1) mCoupling = PS5000A_AC;
  else if (cpl == 2) mCoupling = PS5000A_DC;

  // Sample block size (auto default)
  int samplesPerBlock = 1024;
  Parameter("SampleBlockSize") = samplesPerBlock;

  // Sampling rate (auto default)
  double samplingRate = 10000.0; // 10 kHz
  Parameter("SamplingRate") << samplingRate << "Hz";

  // Channel properties and scaling
  const double uV_per_count = (rangeMv(mRange) * 1000.0) / std::max(1, (int)mMaxADCValue);
  for (int i = 0; i < mNumberOfSignalChannels; ++i)
  {
    Parameter("ChannelNames")(i) << "PS" << char('A'+i);
    Parameter("SourceChGain")(i) << uV_per_count << "uV";
    Parameter("SourceChOffset")(i) = 0;
  }
}

void
PicoScopeADC::OnPreflight( SignalProperties& /*Output*/ ) const
{
  // Parameters and states are checked here.
  State( "PicoScopeADCAsyncTriggers" );
  State( "PicoScopeADCSyncTriggers" );

  if (ActualParameter("SourceCh") < 1 || ActualParameter("SourceCh") > 4)
    bcierr << "SourceCh must be between 1 and 4";

  if (ActualParameter("SamplingRate").InHertz() <= 0)
    bcierr << "SamplingRate must be > 0";
}

void
PicoScopeADC::allocateBuffers()
{
  freeBuffers();
  mDrvMax.resize(mNumberOfSignalChannels, nullptr);
  mDrvMin.resize(mNumberOfSignalChannels, nullptr);
  mFifos.resize(mNumberOfSignalChannels);
  for (int ch = 0; ch < mNumberOfSignalChannels; ++ch)
  {
    mDrvMax[ch] = new int16_t[mOverviewBufferSize];
    mDrvMin[ch] = new int16_t[mOverviewBufferSize];
    std::fill(mDrvMax[ch], mDrvMax[ch] + mOverviewBufferSize, 0);
    std::fill(mDrvMin[ch], mDrvMin[ch] + mOverviewBufferSize, 0);
    mFifos[ch].data.clear();
    mFifos[ch].data.reserve(mOverviewBufferSize / 2);
  }
}

void
PicoScopeADC::freeBuffers()
{
  for (auto* p : mDrvMax) delete [] p;
  for (auto* p : mDrvMin) delete [] p;
  mDrvMax.clear();
  mDrvMin.clear();
  mFifos.clear();
}

// Small struct passed to the SDK callback
struct BufferInfo
{
  PicoScopeADC* self;
  int16_t** drvMax;
  int16_t** drvMin;
  uint32_t overview;
};

// Streaming callback: copy new samples into per-channel FIFOs
static void PREF4 streamingCallback(
  int16_t handle,
  int32_t noOfSamples,
  uint32_t startIndex,
  int16_t overflow,
  uint32_t triggerAt,
  int16_t triggered,
  int16_t autoStop,
  void* pParameter)
{
  (void)handle; (void)triggerAt; (void)triggered;
  BufferInfo* info = reinterpret_cast<BufferInfo*>(pParameter);
  if (!info || !info->self) return;
  PicoScopeADC* self = info->self;

  self->mSampleCountFromCallback = noOfSamples;
  self->mStartIndexFromCallback  = startIndex;
  self->mOverflowFromCallback    = overflow;
  self->mAutoStopped             = (autoStop != 0);
  self->mReady                   = true;

  if (noOfSamples <= 0) return;

  const uint32_t N = static_cast<uint32_t>(noOfSamples);
  const uint32_t O = info->overview;

  for (int ch = 0; ch < self->mNumberOfSignalChannels; ++ch)
  {
    int16_t* src = info->drvMax[ch]; // use max buffer as our data
    auto& fifo = self->mFifos[ch].data;
    fifo.reserve(fifo.size() + N);

    uint32_t s = startIndex;
    uint32_t end = startIndex + N;
    if (end <= O)
    {
      fifo.insert(fifo.end(), src + s, src + end);
    }
    else
    {
      // wrap-around
      uint32_t first = O - s;
      fifo.insert(fifo.end(), src + s, src + O);
      uint32_t second = N - first;
      fifo.insert(fifo.end(), src, src + second);
    }
  }
}

void
PicoScopeADC::OnInitialize( const SignalProperties& Output )
{
  (void)Output;

  // Configure channels
  // Ensure handle exists (from OnAutoConfig)
  if (mHandle < 0)
    throw bcierr << "PicoScope not opened";

  // Derive sample interval from SamplingRate
  const double Fs = ActualParameter("SamplingRate").InHertz();
  // Choose the finest time unit that keeps sampleInterval integer and reasonable
  if (Fs >= 1e6) { mTimeUnits = PS5000A_NS; mSampleInterval = static_cast<uint32_t>(std::max(1.0, 1e9 / Fs)); }
  else if (Fs >= 1e3) { mTimeUnits = PS5000A_US; mSampleInterval = static_cast<uint32_t>(std::max(1.0, 1e6 / Fs)); }
  else { mTimeUnits = PS5000A_MS; mSampleInterval = static_cast<uint32_t>(std::max(1.0, 1e3 / Fs)); }

  // Configure each channel
  for (int i = 0; i < mNumberOfSignalChannels; ++i)
  {
    PICO_STATUS st = ps5000aSetChannel(
      mHandle,
      static_cast<PS5000A_CHANNEL>(PS5000A_CHANNEL_A + i),
      1 /*enabled*/,
      mCoupling,
      mRange,
      0.0f /*analogue offset*/);
    if (st != PICO_OK)
      throw bcierr << "PicoScope: ps5000aSetChannel failed on ch " << i << ", status "
        << PicoStatusName((uint32_t)st) << " (0x" << std::hex << (uint32_t)st << std::dec << ") "
        << PicoStatusDescription((uint32_t)st);
  }

  // Allocate & register streaming buffers
  allocateBuffers();
  for (int i = 0; i < mNumberOfSignalChannels; ++i)
  {
    PICO_STATUS st = ps5000aSetDataBuffers(
      mHandle,
      static_cast<PS5000A_CHANNEL>(PS5000A_CHANNEL_A + i),
      mDrvMax[i], mDrvMin[i],
      mOverviewBufferSize, 0, PS5000A_RATIO_MODE_NONE);
    if (st != PICO_OK)
      throw bcierr << "PicoScope: ps5000aSetDataBuffers failed on ch " << i << ", status " 
        << PicoStatusName((uint32_t)st) << " (0x" << std::hex << (uint32_t)st << std::dec << ") "
        << PicoStatusDescription((uint32_t)st);
  }

  // Reset streaming control
  mReady = false;
  mSampleCountFromCallback = 0;
  mStartIndexFromCallback = 0;
  mOverflowFromCallback = 0;
  mAutoStopped = false;
}

void
PicoScopeADC::OnStartAcquisition()
{
  // Start streaming
  BufferInfo info;
  info.self = this;
  // Build raw pointers array for callback
  std::vector<int16_t*> drvMaxPtrs(mNumberOfSignalChannels);
  std::vector<int16_t*> drvMinPtrs(mNumberOfSignalChannels);
  for (int i = 0; i < mNumberOfSignalChannels; ++i) {
    drvMaxPtrs[i] = mDrvMax[i];
    drvMinPtrs[i] = mDrvMin[i];
  }
  info.drvMax = drvMaxPtrs.data();
  info.drvMin = drvMinPtrs.data();
  info.overview = mOverviewBufferSize;

  // ps5000aRunStreaming copies the sampleInterval back to what the device actually uses.
  uint32_t preTrigger = 0;
  uint32_t postTrigger = 0; // 0 == run indefinitely
  int16_t autoStop = 0;
  mDownsampleRatio = 1;
  mRatioMode = PS5000A_RATIO_MODE_NONE;

  PICO_STATUS st = ps5000aRunStreaming(
    mHandle,
    &mSampleInterval,
    mTimeUnits,
    preTrigger,
    postTrigger,
    autoStop,
    mDownsampleRatio,
    mRatioMode,
    mOverviewBufferSize);

  if (st == PICO_POWER_SUPPLY_NOT_CONNECTED ||
      st == PICO_POWER_SUPPLY_CONNECTED ||
      st == PICO_USB3_0_DEVICE_NON_USB3_0_PORT ||
      st == PICO_POWER_SUPPLY_UNDERVOLTAGE)
  {
    ps5000aChangePowerSource(mHandle, st);
    // try again
    st = ps5000aRunStreaming(
      mHandle, &mSampleInterval, mTimeUnits, preTrigger, postTrigger, autoStop,
      mDownsampleRatio, mRatioMode, mOverviewBufferSize);
  }

  if (st != PICO_OK)
      throw bcierr << "PicoScope: ps5000aRunStreaming failed: "
      << PicoStatusName((uint32_t)st) << " (0x" << std::hex << (uint32_t)st << std::dec << ") "
      << PicoStatusDescription((uint32_t)st);


  // First poll to kick off callbacks
  ps5000aGetStreamingLatestValues(mHandle, streamingCallback, &info);
}

void
PicoScopeADC::DoAcquire( GenericSignal& Output )
{
  // We need SampleBlockSize samples; accumulate from the per-channel FIFOs.
  const int needed = Output.Elements();

  int have = (int)mFifos[0].data.size();
  // Ensure all channels have at least 'have' samples
  for (int ch = 1; ch < mNumberOfSignalChannels; ++ch)
    have = std::min(have, (int)mFifos[ch].data.size());

  BufferInfo info;
  std::vector<int16_t*> drvMaxPtrs(mNumberOfSignalChannels);
  std::vector<int16_t*> drvMinPtrs(mNumberOfSignalChannels);
  for (int i = 0; i < mNumberOfSignalChannels; ++i) { drvMaxPtrs[i] = mDrvMax[i]; drvMinPtrs[i] = mDrvMin[i]; }
  info.self = const_cast<PicoScopeADC*>(this);
  info.drvMax = drvMaxPtrs.data();
  info.drvMin = drvMinPtrs.data();
  info.overview = mOverviewBufferSize;

  while (have < needed)
  {
    // Poll driver; this will invoke our callback if data is available
    PICO_STATUS st = ps5000aGetStreamingLatestValues(mHandle, streamingCallback, &info);
    if (st == PICO_POWER_SUPPLY_UNDERVOLTAGE)
    {
      ps5000aChangePowerSource(mHandle, st);
    }
    else if (st != PICO_OK && st != PICO_BUSY)
    {
      // Non-fatal: keep trying, but log once.
      bciwarn << "PicoScope: GetStreamingLatestValues status " << (int)st;
      ThreadUtils::SleepForMs(1);
    }

    // Refresh 'have'
    have = (int)mFifos[0].data.size();
    for (int ch = 1; ch < mNumberOfSignalChannels; ++ch)
      have = std::min(have, (int)mFifos[ch].data.size());

    if (have < needed)
      ThreadUtils::SleepForMs(1);
  }

  // Output raw ADC counts; scaling is provided via SourceChGain.
  for (int sample = 0; sample < needed; ++sample)
  {
    for (int ch = 0; ch < mNumberOfSignalChannels; ++ch)
    {
      Output(ch, sample) = mFifos[ch].data[sample];
    }
  }

  // Pop consumed samples
  for (int ch = 0; ch < mNumberOfSignalChannels; ++ch)
  {
    auto& v = mFifos[ch].data;
    v.erase(v.begin(), v.begin() + needed);
  }
}

void
PicoScopeADC::OnStopAcquisition()
{
  if (mHandle >= 0)
    ps5000aStop(mHandle);

  // Unset buffers on device
  for (int i = 0; i < mNumberOfSignalChannels; ++i)
  {
    ps5000aSetDataBuffers(
      mHandle,
      static_cast<PS5000A_CHANNEL>(PS5000A_CHANNEL_A + i),
      nullptr, nullptr, 0, 0, PS5000A_RATIO_MODE_NONE);
  }

  freeBuffers();
}
