////////////////////////////////////////////////////////////////////////////////
// $Id: BlackrockGeminiADC.cpp 9338 2026-04-20 16:26:34Z nluczak $
// Authors: griffin.milsap@gmail.com
// Description: Implementation of a source module for Blackrock systems
//
// This module works in a slightly nonstandard way.  The Blackrock NSP is very
// configurable using the Central application.  The initial version of this
// source module was originally intended to configure the NSP entirely
// from BCI2000, but the device is SOOO configurable (even to the extent of
// being able to set different sampling rates for individual channels), it
// became infeasable.  As such, this module will simply query the current
// channel configuration from the device and work with that.  This module is
// no longer capable of changing the Blackrock configuration; all configuration
// changes must be made in Central.
//
// The only really configurable parameter is the SamplingRate parameter which
// decides a sampling group to record from.  Because the device can record
// individual channels at different sampling rates, you must choose the sampling
// rate of the system, and channels that are currently being sampled at that rate
// are recorded into the BCI2000 data stream.  BCI2000 does not support per-channel
// sampling rates, so all channels must be configured to sample at the same rate.
// That is, you can still have some channels sampled at a different rate, but
// BCI2000 will ignore them.
//
// See more documentation on the BCI2000 WIKI.
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
#include "BlackrockGeminiADC.h"
#include "BCIStream.h"
#include <algorithm>
#include <cstring>
#include <map>
#include <stdlib.h>

using namespace std;

// This list should REALLY be included in the CBSDK library somewhere
// There are a maximum of 8 sample groups. When you query sample groups
// from the API, they are 1-indexed (hence, GroupRates[0] = 0).  These
// sample groups correspond to the output NSx files.  NS2 files are sampled
// at 1k, ns4 at 10k, etc.  The raw sample group is cbRAWGROUP (6) and is
// also sampled at 30k.  Currently, there is no way to query this information
// from the CBSDK API, but may change in the future.
int gGroupRates[] = { 0, 500, 1000, 2000, 10000, 30000 }; // samples per second
int gBlockSizes[] = { 0, 10,  20,   40,   200,   600 }; // samples per block (50 Hz)

// IT IS CRUCIAL THAT THE SYSTEM IS NOT RUN THROUGH MORE THAN ONE SWITCH!
// IF IT ISN'T, THERE WILL VERY LIKELY BE DROPPED PACKETS DURING THE
// CONFIGURATION PROCESS AND CBSDK HAS UNDEFINED BEHAVIOR.
string gPktErrMsg =
"This error suggests an incomplete system configuration due to dropped "
"packets early in the connection process.  Please ensure that this machine "
"is connected to the NSP through a SINGLE switch of high commercial quality.";

// Multi-NSP Synchronization Channels
// Connect the output sync channel on ONE amp to the input sync channel on all amps.
// The analog sync input channels on all amps will be monitored and delays will be adjusted
#define NSP_SYNC_OUTPUT_CHANNEL cbFIRST_DIGOUT_CHAN + 1 // Digital Output 1 (153)
#define NSP_SYNC_INPUT_CHANNEL cbFIRST_ANAIN_CHAN + cbNUM_ANAIN_CHANS // Analog Input 16 (144)
#define NSP_SYNC_THRESHOLD 1000 // Threshold for sync pulse from Digital Output 1

RegisterFilter(BlackrockADC, 1);

BlackrockADC::BlackrockADC()
{
    BEGIN_PARAMETER_DEFINITIONS

        "Source:Signal%20Properties float SamplingRate= 2000Hz "
        "10000Hz 2000 30000 // sample rate",

        // Although you CAN specify these, it'd be best if you let the module
        // autoconfigure them individually based on the SamplingRate.
        "Source int SourceCh= auto (allow_override)",
        "Source int SampleBlockSize= auto ",
        "Source list ChannelNames= 1 auto ",
        "Source list SourceChOffset= 1 auto ",
        "Source list SourceChGain= 1 auto ",

        END_PARAMETER_DEFINITIONS

        BEGIN_STREAM_DEFINITIONS
        "NSPSyncState 2 0 0 0",
        "BlackrockSourceTimeHigh 32 0 0 0",
        "BlackrockSourceTimeLow 32 0 0 0",
        END_STREAM_DEFINITIONS
}

BlackrockADC::~BlackrockADC()
{

}

void
BlackrockADC::OnAutoConfig()
{
    // Attempt autoconfiguration
    if (Connect()) {
        int group = 0;
        if (group = GetRequestedSampleGroup())
        {
            // Suggest a good SampleBlockSize
            Parameter("SampleBlockSize") = gBlockSizes[group];

            // Acquire the channel configuration
            vector< ChanInfo > chanConfig;
            vector< int > syncChans;
            if (GetChannelConfig(1, group, chanConfig, syncChans))
            {
                // Resize signal source parameters accordingly
                size_t numChans = chanConfig.size();
                Parameter("SourceCh") = numChans;
                Parameter("SourceChOffset")->SetNumValues(numChans);
                Parameter("SourceChGain")->SetNumValues(numChans);
                Parameter("ChannelNames")->SetNumValues(numChans);

                // Populate channel information
                for (unsigned int i = 0; i < numChans; i++)
                {
                    Parameter("SourceChOffset")(i) = chanConfig[i].offset;
                    Parameter("SourceChGain")(i) = chanConfig[i].gain;
                    Parameter("ChannelNames")(i) = chanConfig[i].label;
                }
            }
            else bcierr << "Error acquiring channel configuration from NSP." << endl;
        }
        else bcierr << "The requested SamplingRate does not correspond to a valid NSP SampleGroup." << endl;
    }
    else bcierr << "Could not establish connection to cbsdk" << endl;

    Disconnect(1);
}

void
BlackrockADC::OnPreflight(SignalProperties& Output) const
{
    if (Connect()) {
        int group = 0;
        if (group = GetRequestedSampleGroup())
        {
            // Acquire the channel configuration
            vector< ChanInfo > chanConfig;
            vector< int > syncChans;
            if (GetChannelConfig(1, group, chanConfig, syncChans))
            {
                bool sync_procedure_enabled = true;
                for (size_t i = 0; i < syncChans.size(); i++)
                    if (syncChans[i] == -1)
                        sync_procedure_enabled = false;

                int numChans = chanConfig.size();

                bool goodOffsets = true,
                    goodGains = true,
                    goodNames = true;
                string matchMessage = " parameter must match the number of channels"
                    " in the requested sample group";

                // Check the SourceCh parameter
                if (Parameter("SourceCh") != numChans)
                    bcierr << "The SourceCh "
                    << matchMessage
                    << " (" << numChans << ") ";

                // Check the channel offsets
                if (Parameter("SourceChOffset")->NumValues() != numChans)
                    bcierr << "The number of values in the SourceChOffset"
                    << matchMessage
                    << " (" << numChans << ") ";
                else
                    for (unsigned int i = 0; i < numChans; ++i)
                    {
                        double chOffset = chanConfig[i].offset;
                        double prmoffset = Parameter("SourceChOffset")(i);
                        bool same = (1e-3 > ::fabs(prmoffset - chOffset) / (chOffset ? chOffset : 1.0));
                        goodOffsets &= same;

                        if (!same) bciwarn << "CBSDK suggests the offset of"
                            << " channel " << i + 1
                            << " is " << chOffset
                            << " whereas the corresponding value in the"
                            << " SourceChOffset parameter is " << prmoffset;
                    }

                if (!goodOffsets)
                    bcierr << "The SourceChOffset values must match the channel "
                    << "resolutions settings in the recording software.";

                // Check gains and ensure they match up with what is reported by cbsdk
                if (Parameter("SourceChGain")->NumValues() != numChans)
                    bcierr << "The number of values in the SourceChGain"
                    << matchMessage
                    << " (" << numChans << ") ";
                else
                    for (unsigned int i = 0; i < numChans; ++i)
                    {
                        double chGain = chanConfig[i].gain;
                        double prmgain = Parameter("SourceChGain")(i);
                        bool same = (1e-3 > ::fabs(prmgain - chGain) / (chGain ? chGain : 1.0));
                        goodGains &= same;

                        if (!same) bciwarn << "CBSDK suggests the gain of"
                            << " channel " << i + 1
                            << " is " << chGain
                            << " whereas the corresponding value in the"
                            << " SourceChGain parameter is " << prmgain;
                    }

                if (!goodGains)
                    bcierr << "The SourceChGain values must match the channel "
                    << "resolutions settings in the recording software.";

                // Check names and ensure they match up with what is reported by cbsdk
                if (Parameter("ChannelNames")->NumValues() != numChans)
                    bcierr << "The number of values in the ChannelNames"
                    << matchMessage
                    << " (" << numChans << ") ";
                else
                    for (unsigned int i = 0; i < numChans; ++i)
                    {
                        string prmlabel = Parameter("ChannelNames")(i);
                        string sdklabel = chanConfig[i].label;
                        bool same = prmlabel == sdklabel;
                        goodNames &= same;

                        if (!same) bciwarn << "The CBSDK says channel " << i + 1
                            << " is labeled " << sdklabel
                            << " whereas the corresponding value in the"
                            << " ChannelNames parameter is " << prmlabel;
                    }

                if (!goodNames)
                    bciwarn << "The ChannelNames values should ideally match the channel "
                    << "labels in Central to avoid confusion later.";

            }
            else bcierr << "Error acquiring channel configuration from NSP." << endl;
        }
        else bcierr << "The requested SamplingRate does not correspond to a valid NSP SampleGroup." << endl;
    }
    else bcierr << "Could not re-establish connection to cbsdk" << endl;

    Disconnect(1);
    State("NSPSyncState");

    // We will place no limits on SampleBlockSize because cbsdk
    // receives individual frames from the NSP anyway.
    int numberOfChannels = Parameter("SourceCh") + 2;
    int samplesPerBlock = Parameter("SampleBlockSize");
    SignalType sigType = SignalType::int32;
    Output = SignalProperties(numberOfChannels, samplesPerBlock, sigType);
    Output.ChannelLabels()[numberOfChannels - 2] = "@BlackrockSourceTimeHigh";
    Output.ChannelLabels()[numberOfChannels - 1] = "@BlackrockSourceTimeLow";
}

void
BlackrockADC::OnInitialize(const SignalProperties& Output)
{
    mSampleGroup = GetRequestedSampleGroup();
    mSampleBlockSize = Parameter("SampleBlockSize");
    mNSPInstances = 1;
    mDataPacketBuffers.clear();
    mInstruments.clear();
    mInstrumentToIndex.fill(-1);
    mTotalGroupChannels = 0;
    mInstrumentQueues.clear();
    State("NSPSyncState") = 1;
}

void
BlackrockADC::OnStartAcquisition()
{
    if (Connect())
    {
        // Acquire the channel configuration
        if (!GetChannelConfig(1, mSampleGroup, mChannelConfig, mSyncChans))
            bcierr << "Couldn't acquire channel configuration" << endl;

        // Build the per-instrument layout of the composited frame from
        // mChannelConfig. Each bin records where this instrument's channels
        // sit in the frame that OnData() assembles.
        mInstruments.clear();
        mInstrumentToIndex.fill(-1);
        {
            std::map<uint8_t, size_t> counts, firstIdx;
            for (const auto& c : mChannelConfig)
            {
                if (counts.find(c.instrument) == counts.end())
                    firstIdx[c.instrument] = c.idx;
                counts[c.instrument]++;
            }
            for (const auto& kv : counts)
            {
                InstrumentBin ib;
                ib.instrument   = kv.first;
                ib.baseOffset   = firstIdx[kv.first];
                ib.channelCount = kv.second;
                mInstrumentToIndex[kv.first] = static_cast<int>(mInstruments.size());
                mInstruments.push_back(ib);
            }
        }
        mTotalGroupChannels = mChannelConfig.size();
        mInstrumentQueues.clear();
        mInstrumentQueues.resize(mInstruments.size());

        std::unique_lock<std::recursive_mutex> lock(mDataMutex);
        for (int instIdx = 0; instIdx < 1; instIdx++)
        {
            // Ready a queue for our data packets
            mDataPacketBuffers.push_back(SampleQueue());

            // Register data packet callback
            CereLinkError(cbSdkRegisterCallback(instIdx, CBSDKCALLBACK_CONTINUOUS, DataCallback, this));
        }
        lock.unlock();
    }
    else
    {
        // Error and ensure we disconnect from any NSPs we connected to.
        bcierr << "Could not re-establish connection to cbsdk" << endl;
        Disconnect(1);
    }
    mDataAvailable.Reset();
}

void
BlackrockADC::DoAcquire(GenericSignal& Output)
{
    // Wait while there isn't enough data in the queue
    if (!mDataAvailable.Wait())
        return;

    // Dequeue data into the signal output
    std::unique_lock<std::recursive_mutex> lock(mDataMutex);
    auto pOutput = Output.MutableData(GenericSignal::intentOverwriteContents);
    for (int fr_idx = 0; fr_idx < Output.Elements(); fr_idx++)
    {
        uint64_t timestamp = 0;
        for (int ch_idx = 0; ch_idx < Output.Channels() - 2; ch_idx++)
        {
            int inst = mChannelConfig[ch_idx].inst;
            int idx = mChannelConfig[ch_idx].idx;
            pOutput[Output.LinearIndex(ch_idx, fr_idx)] = mDataPacketBuffers[inst].front().second[idx];
            // assume a single timestamp across all instances
            timestamp = mDataPacketBuffers[inst].front().first;
        }
        uint64_t timestampHigh = timestamp >> 32,
            timestampLow = timestamp & ((1LL << 32) - 1);
        pOutput[Output.LinearIndex(Output.Channels() - 2, fr_idx)] = timestampHigh;
        pOutput[Output.LinearIndex(Output.Channels() - 1, fr_idx)] = timestampLow;

        // Pop the recorded frame off the buffers
        for (int instIdx = 0; instIdx < 1; instIdx++)
            mDataPacketBuffers[instIdx].pop();
    }
    lock.unlock();
    mDataAvailable.Reset();
}

void
BlackrockADC::StartRun()
{
}

void
BlackrockADC::OnProcess()
{
    // Handle state machine for synchronization procedure
    if (State("NSPSyncState") == 1)
    {
        State("NSPSyncState") = 2;
    }
    else if (State("NSPSyncState") == 2)
    {
        State("NSPSyncState") = 0;
    }
}

void
BlackrockADC::Process(const GenericSignal& Input, GenericSignal& Output)
{
    // Call Superclass!
    BufferedADC::Process(Input, Output);
}

void
BlackrockADC::StopRun()
{
}

void
BlackrockADC::OnStopAcquisition()
{
    Disconnect(1);
}

void
BlackrockADC::OnHalt()
{
}

bool
BlackrockADC::Connect(int nInstances) const
{
    for (int instIdx = 0; instIdx < nInstances; instIdx++)
    {
        // Open a connection to the SDK
        cbSdkConnectionType conType = CBSDKCONNECTION_DEFAULT;
        if (CereLinkError(cbSdkOpen(instIdx, conType, cbSdkConnection())))
        {
            bcierr << "Could not establish connection to cbsdk for instance " << instIdx << ".  "
                << "CBSDK may not always connect even if your system is set up properly. "
                << "Try to 'ping " << cbNET_UDP_ADDR_CNT << "'" << endl;
            return false;
        }

        // Determine the SDK connection type
        cbSdkInstrumentType instType;
        if (CereLinkError(cbSdkGetType(instIdx, &conType, &instType)))
        {
            bcierr << "Unable to determine connection type for instance " << instIdx << endl;
            return false;
        }

        // Get the NSP Version
        cbSdkVersion ver;
        if (CereLinkError(cbSdkGetVersion(instIdx, &ver)))
        {
            bcierr << "Unable to get NSP version for instance " << instIdx << ".  "
                << "Is device connected and on?" << endl;
            return false;
        }

        // Assemble a debug string for the connection information
        // if (conType < 0 || conType > CBSDKCONNECTION_COUNT) conType = CBSDKCONNECTION_COUNT;
        // if (instType < 0 || instType > CBSDKINSTRUMENT_COUNT) instType = CBSDKINSTRUMENT_COUNT;
        // string strConnection[] = { "Default", "Central", "UDP", "Closed", "Unknown" };
        // string strInstrument[] = { "NSP", "nPlay", "Local NSP", "Remote nPlay", "Unknown" };
        // bciout << "Instance " << instIdx << ": " << strConnection[conType]
        //     << " real-time interface to " << strInstrument[instType]
        //     << "(V" << ver.nspmajor << "." << ver.nspminor << "."
        //     << ver.nsprelease << "." << ver.nspbeta << ")";
    }

    return true;
}

bool
BlackrockADC::GetChannelConfig(int iNumInstances, int iGroup,
    vector< ChanInfo >& oChanConfig,
    vector< int >& oSyncChans) const
{ // BlackrockADC::Connect must be called first!

    oChanConfig.clear();
    oSyncChans.clear();

    // Probe every analog channel id rather than relying on
    // cbSdkGetSampleGroupList(). On Gemini, that list reports only the
    // neural-amp (FE) channels in the requested sample group; AINP channels
    // configured for the same group still stream as cbPKT_GROUP packets, but
    // under a different cbpkt_header.instrument, and do not appear in the
    // group list. A probe-based scan finds them and, as a side effect, yields
    // the instrument id that OnData()
    for (int instIdx = 0; instIdx < iNumInstances; instIdx++)
    {
        int sync_input_ch = -1;

        // Group by instrument so the block has an expected layout
        // in ascending instrument id (FE=0 first, then AINP=2, etc.).
        std::map<uint8_t, std::vector<ChanInfo>> channelsByInstrument;

        for (uint16_t chan_id = 1; chan_id <= cbNUM_ANALOG_CHANS; ++chan_id)
        {
            cbPKT_CHANINFO chanInfo;
            std::memset(&chanInfo, 0, sizeof(chanInfo));

            // Unallocated channel ids are expected, so skip withhout complaint.
            if (cbSdkGetChannelConfig(instIdx, chan_id, &chanInfo) != CBSDKRESULT_SUCCESS)
                continue;
            if (static_cast<int>(chanInfo.smpgroup) != iGroup)
                continue;

            ChanInfo nc;
            nc.inst       = instIdx;
            nc.offset     = 0;
            nc.gain       = ScalingToGain(chanInfo.physcalin);
            nc.label      = string(chanInfo.label);
            nc.instrument = chanInfo.cbpkt_header.instrument;
            nc.idx        = 0;  // assigned below, once canonical order is known
            channelsByInstrument[nc.instrument].push_back(nc);
        }

        // Emit channels in instrument id order, assigning each its final id
        for (auto& kv : channelsByInstrument)
        {
            for (auto& c : kv.second)
            {
                c.idx = static_cast<unsigned int>(oChanConfig.size());
                oChanConfig.push_back(c);
            }
        }
        oSyncChans.push_back(sync_input_ch);
    }

    // Quick sanity check
    if (oChanConfig.size() == 0)
    {
        bcierr << "There are no channels sampled at the desired SamplingRate." << endl;
        return false;
    }

    return true;
}

void
BlackrockADC::Disconnect(int nInstances) const
{
    if (nInstances < 1) nInstances = 1;
    for (int instIdx = 0; instIdx < nInstances; instIdx++)
        // Disconnect from the instrument
        CereLinkError(cbSdkClose(instIdx));
}

void BlackrockADC::DataCallback(uint32_t iInstance, const cbSdkPktType iType, const void* iData, void* iBlackrockADC)
{
    // Re-establish communication to the class
    BlackrockADC* pAdc = reinterpret_cast<BlackrockADC*>(iBlackrockADC);
    pAdc->OnData(iInstance, iType, iData);
}

void BlackrockADC::OnData(uint32_t iInstance, const cbSdkPktType iType, const void* iData)
{
    switch (iType)
    {
    case cbSdkPkt_PACKETLOST:
        bcinote << "Packet loss (data has been lost)";
        bcierr << "Packet loss.  Data has been lost.  Reduce system load." << endl;
        break;
    case cbSdkPkt_CONTINUOUS:
        if (iData)
        {
            // On Gemini, a single timestep is broadcast as one cbPKT_GROUP
            // for each instrument (one for FE, one for AINP).
            // take each packet into its instrument's FIFO, and send a
            // block once every instrument has contributed at
            // least one packet, pairing channels by arrival order.
            //
            // Note that cbpkt_header.time is the per-packet time, not
            // a shared sample time, so it cannot be used to match channels
            // between instruments. Pairing by arrival order is the only
            // reliable option here -- see the resync guard below.
            const cbPKT_GROUP* pPkt = reinterpret_cast<const cbPKT_GROUP*>(iData);
            const uint64_t timestamp  = pPkt->cbpkt_header.time;
            const uint16_t dlen       = pPkt->cbpkt_header.dlen;
            const uint8_t  instrument = pPkt->cbpkt_header.instrument;

            const int ibIdx = (instrument < mInstrumentToIndex.size())
                              ? mInstrumentToIndex[instrument] : -1;
            if (ibIdx < 0)
            {
                static bool warned = false;
                if (!warned)
                {
                    bciwarn << "Ignoring cbPKT_GROUP with unexpected instrument id "
                            << static_cast<int>(instrument)
                            << " (dlen=" << dlen << ")." << endl;
                    warned = true;
                }
                return;
            }
            const InstrumentBin& ib = mInstruments[ibIdx];

            // dlen gets 32-bit; A2D_DATA is int16, so each combination
            // carries two samples. Using dlen avoids copying the unused memory
            // of the packet buffer.
            const size_t actualSamples =
                (static_cast<size_t>(dlen) * sizeof(uint32_t)) / sizeof(pPkt->data[0]);

            std::unique_lock<std::recursive_mutex> lock(mDataMutex);

            if (iInstance >= mDataPacketBuffers.size() ||
                static_cast<size_t>(ibIdx) >= mInstrumentQueues.size() ||
                mTotalGroupChannels == 0)
                return;

            std::vector<int32_t> samples(ib.channelCount, 0);
            const size_t toCopy = std::min(actualSamples, ib.channelCount);
            for (size_t i = 0; i < toCopy; ++i)
                samples[i] = static_cast<int32_t>(pPkt->data[i]);
            mInstrumentQueues[ibIdx].push(std::make_pair(timestamp, std::move(samples)));

            // empty as many sets as you can.
            for (;;)
            {
                bool allReady = true;
                for (const auto& q : mInstrumentQueues)
                    if (q.empty()) { allReady = false; break; }
                if (!allReady) break;

                std::vector<int32_t> composite(mTotalGroupChannels, 0);
                uint64_t compositeTs = 0;
                for (size_t i = 0; i < mInstruments.size(); ++i)
                {
                    const InstrumentBin& ibi = mInstruments[i];
                    auto& q = mInstrumentQueues[i];
                    const auto& front = q.front();
                    if (front.first > compositeTs) compositeTs = front.first;
                    const size_t m = std::min(front.second.size(), ibi.channelCount);
                    for (size_t j = 0; j < m; ++j)
                        composite[ibi.baseOffset + j] = front.second[j];
                    q.pop();
                }
                mDataPacketBuffers[iInstance].push(
                    std::make_pair(compositeTs, std::move(composite)));
            }

            // Resync guard: if one instrument falls far behind, flush every
            // queue so pairing can re-align from a clean state.
            constexpr size_t kMaxInstrumentQueueDepth = 8192;
            for (const auto& q : mInstrumentQueues)
            {
                if (q.size() > kMaxInstrumentQueueDepth)
                {
                    for (auto& qq : mInstrumentQueues)
                        while (!qq.empty()) qq.pop();
                    break;
                }
            }

            size_t minSize = mDataPacketBuffers[0].size();
            for (unsigned int i = 1; i < mNSPInstances; ++i)
                minSize = std::min(minSize, mDataPacketBuffers[i].size());
            const size_t sampleBlockSize = mSampleBlockSize;
            lock.unlock();

            if (minSize >= sampleBlockSize)
                mDataAvailable.Set();
        }
        break;
    default:
        break;
    }
    return;
}

// Check the SamplingRate parameter and return a Blackrock group index
// If this returns 0, the sampling rate is not a valid rate.
int
BlackrockADC::GetRequestedSampleGroup() const
{
    int samplingRate = Parameter("SamplingRate").InHertz();
    int num_rates = sizeof(gGroupRates) / sizeof(int);
    for (int rate_idx = 0; rate_idx < num_rates; rate_idx++)
        if (samplingRate == gGroupRates[rate_idx])
            return rate_idx;
    return 0;
}

// Convert a cbSCALING parameter to a SourceChGain
double
BlackrockADC::ScalingToGain(cbSCALING scaling) const
{
    double anaRange = scaling.anamax - scaling.anamin;
    double digRange = scaling.digmax - scaling.digmin;
    return (anaRange / digRange);
}

// Debug channel configurations
void
BlackrockADC::OutputChannelDebugInfo(int nInstances) const
{
    for (unsigned int instIdx = 0; instIdx < 1; instIdx++)
    {
        for (unsigned int ch = 0; ch < cbMAXCHANS; ch++)
        {
            cbPKT_CHANINFO* pChanInfo = new cbPKT_CHANINFO();
            if (!CereLinkError(cbSdkGetChannelConfig(instIdx, ch, pChanInfo)))
                bciout << pChanInfo->label << " -- "
                << "Inst: " << instIdx << ", "
                << "Bank: " << pChanInfo->bank << ", "
                << "Chan: " << pChanInfo->chan;
            delete pChanInfo;
        }
    }
}

// Misc Cerelink error checking
bool
BlackrockADC::CereLinkError(cbSdkResult res)
{
    cbSdkInstrumentType instType;
    switch (res)
    {
    case CBSDKRESULT_WARNCONVERT:
        bciwarn << "File conversion is needed..." << endl;
        return false;
    case CBSDKRESULT_WARNCLOSED:
        bciwarn << "Library is already closed" << endl;
        return false;
    case CBSDKRESULT_WARNOPEN:
        bciwarn << "Library is already opened" << endl;
        return false;
    case CBSDKRESULT_SUCCESS:
        //bcidbg( 0 ) << "Success" << endl;
        return false;
    case CBSDKRESULT_NOTIMPLEMENTED:
        bcierr << "Not implemented" << endl;
        return true;
    case CBSDKRESULT_INVALIDPARAM:
        bcierr << "Invalid parameter" << endl;
        return true;
    case CBSDKRESULT_CLOSED:
        bcierr << "Interface is closed cannot do this operation" << endl;
        return true;
    case CBSDKRESULT_OPEN:
        bcierr << "Interface is open cannot do this operation" << endl;
        return true;
    case CBSDKRESULT_NULLPTR:
        bcierr << "Null pointer" << endl;
        return true;
    case CBSDKRESULT_ERROPENCENTRAL:
        bcierr << "Unable to open Central interface" << endl;
        return true;
    case CBSDKRESULT_ERROPENUDP:
        bcierr << "Unable to open UDP interface (might happen if default)" << endl;
        return true;
    case CBSDKRESULT_ERROPENUDPPORT:
        bcierr << "Unable to open UDP port" << endl;
        return true;
    case CBSDKRESULT_ERRMEMORYTRIAL:
        bcierr << "Unable to allocate RAM for trial cache data" << endl;
        return true;
    case CBSDKRESULT_ERROPENUDPTHREAD:
        bcierr << "Unable to open UDP timer thread" << endl;
        return true;
    case CBSDKRESULT_ERROPENCENTRALTHREAD:
        bcierr << "Unable to open Central communication thread" << endl;
        return true;
    case CBSDKRESULT_INVALIDCHANNEL:
        bcierr << "Invalid channel number" << endl;
        return true;
    case CBSDKRESULT_INVALIDCOMMENT:
        bcierr << "Comment too long or invalid" << endl;
        return true;
    case CBSDKRESULT_INVALIDFILENAME:
        bcierr << "Filename too long or invalid" << endl;
        return true;
    case CBSDKRESULT_INVALIDCALLBACKTYPE:
        bcierr << "Invalid callback type" << endl;
        return true;
    case CBSDKRESULT_CALLBACKREGFAILED:
        bcierr << "Callback register/unregister failed" << endl;
        return true;
    case CBSDKRESULT_ERRCONFIG:
        bcierr << "Trying to run an unconfigured method" << endl;
        return true;
    case CBSDKRESULT_INVALIDTRACKABLE:
        bcierr << "Invalid trackable id, or trackable not present" << endl;
        return true;
    case CBSDKRESULT_INVALIDVIDEOSRC:
        bcierr << "Invalid video source id, or video source not present" << endl;
        return true;
    case CBSDKRESULT_ERROPENFILE:
        bcierr << "Cannot open file" << endl;
        return true;
    case CBSDKRESULT_ERRFORMATFILE:
        bcierr << "Wrong file format" << endl;
        return true;
    case CBSDKRESULT_OPTERRUDP:
        bcierr << "Socket option error (Possibly permission issue)" << endl;
        return true;
    case CBSDKRESULT_MEMERRUDP:
        bcierr << "Socket memory assignment error" << endl
            << " Consider using sysctl -w net.core.rmem_max=8388608" << endl
            << " or sysctl -w kern.ipc.maxsockbuf=8388608" << endl;
        return true;
    case CBSDKRESULT_INVALIDINST:
        bcierr << "Invalid range or instrument address" << endl;
        return true;
    case CBSDKRESULT_ERRMEMORY:
        bcierr << "Library memory allocation error" << endl;
        return true;
    case CBSDKRESULT_ERRINIT:
        bcierr << "Library initialization error" << endl;
        return true;
    case CBSDKRESULT_TIMEOUT:
        bcierr << "Connection timeout error" << endl;
        return true;
    case CBSDKRESULT_BUSY:
        bcierr << "Resource is busy" << endl;
        return true;
    case CBSDKRESULT_ERROFFLINE:
        bcierr << "Instrument is offline" << endl;
        return true;
    case CBSDKRESULT_UNKNOWN:
    default:
        bcierr << "Unknown error.  Sorry!" << endl;
        return true;
    }
}
