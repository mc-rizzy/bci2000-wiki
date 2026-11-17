////////////////////////////////////////////////////////////////////////////////
// $Id: BlackrockGeminiADC.h 9338 2026-04-20 16:26:34Z nluczak $
// Authors: griffin.milsap@gmail.com
// Description: Implementation of a source module for Blackrock systems
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_BLACKROCKADC_H
#define INCLUDED_BLACKROCKADC_H

#include "BufferedADC.h"
#include "PrecisionTime.h"
#include "Expression.h"
#include "WaitableEvent.h"

#include "cbsdk.h"

#include <array>
#include <cstdint>
#include <vector>
#include <queue>
#include <mutex>
class BlackrockADC : public BufferedADC
{
 public:
           BlackrockADC();
  virtual ~BlackrockADC();
  virtual void OnAutoConfig();
  virtual void OnHalt();
  virtual void OnPreflight( SignalProperties& Output ) const;
  virtual void OnInitialize( const SignalProperties& Output );
  virtual void OnProcess();
  virtual void StartRun();
  virtual void OnStartAcquisition();
  virtual void DoAcquire( GenericSignal& Output );
  virtual void OnStopAcquisition();
  virtual void StopRun();

  // Bit of a hack necessary for digital output based on expressions
  virtual void Process( const GenericSignal& Input, GenericSignal& Output );

 private:
  struct ChanInfo
  {
    unsigned int inst;
    unsigned int idx;
    double gain;
    double offset;
    std::string label;
    uint8_t instrument;  // cbpkt_header.instrument that owns this channel's data packets
  };

  // Per-instrument layout within the composited frame assembled by OnData().
  struct InstrumentBin
  {
    uint8_t instrument;
    size_t  baseOffset;
    size_t  channelCount;
  };

  bool Connect( int nInstances = 1 ) const;
  void Disconnect( int nInstances = 1 ) const;
  bool GetChannelConfig( int iNumInstances, int iGroup,
                         std::vector< ChanInfo > &oChanConfig,
                         std::vector< int > &oSyncChans ) const;
  static void DataCallback( UINT32 iInstance, const cbSdkPktType iType, const void* iData, void* iBlackrockADC );
  void OnData(UINT32 iInstance, const cbSdkPktType iType, const void* iData);
  static bool CereLinkError( cbSdkResult res );
  int GetRequestedSampleGroup() const;
  double ScalingToGain( cbSCALING scaling ) const;

  // Debug Functionality
  void OutputChannelDebugInfo( int nInstances ) const;

  std::recursive_mutex mDataMutex;
  typedef std::pair<uint64_t, std::vector<int32_t>> SampleData;
  typedef std::queue<SampleData> SampleQueue;
  std::vector<SampleQueue> mDataPacketBuffers;
  std::vector<ChanInfo> mChannelConfig;
  std::vector<int> mSyncChans;
  int mSampleGroup = 0;
  unsigned int mNSPInstances = 0;
  unsigned int mSampleBlockSize = 0;
  WaitableEvent mDataAvailable;

  // Per-timestep combiner. On Gemini, the hub emits one cbPKT_GROUP
  // per instrument per timestep
  // and two channels must be paired by arrival order rather than by
  // cbpkt_header.time, which is a per-packet send-time rather than shared time.
  std::vector<InstrumentBin> mInstruments;
  std::array<int, 256>       mInstrumentToIndex{};
  size_t                     mTotalGroupChannels = 0;
  std::vector<std::queue<std::pair<uint64_t, std::vector<int32_t>>>> mInstrumentQueues;
};

#endif // INCLUDED_BLACKROCKADC_H
