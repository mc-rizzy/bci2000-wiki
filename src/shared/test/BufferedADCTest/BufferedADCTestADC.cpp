////////////////////////////////////////////////////////////////////////////////
// Authors: juergen@VBox-Win10.fritz.box
// Description: BufferedADCTestADC implementation
////////////////////////////////////////////////////////////////////////////////
#include "BufferedADCTestADC.h"

#include "BCIStream.h"
#include "BCIEvent.h"

// Make the source filter known to the framework.
RegisterFilter(BufferedADCTestADC, 1);

BufferedADCTestADC::BufferedADCTestADC()
    : mNumberOfSignalChannels(0)
{
}

BufferedADCTestADC::~BufferedADCTestADC()
{
}

void
BufferedADCTestADC::OnPublish()
{
    BEGIN_PARAMETER_DEFINITIONS

        "Source:Signal%20Properties int SourceCh= auto "
        "auto 1 % // number of digitized and stored channels",

        "Source:Signal%20Properties int SampleBlockSize= auto "
        "auto 1 % // number of samples transmitted at a time",

        "Source:Signal%20Properties float SamplingRate= auto "
        "auto 0.0 % // sample rate",

        "Source:Signal%20Properties list SourceChGain= 1 auto "
        " // physical units per raw A/D unit",

        "Source:Signal%20Properties list SourceChOffset= 1 auto "
        " // raw A/D offset to subtract, typically 0",

        "Source:Signal%20Properties list ChannelNames= 1 auto "
        " // names of amplifier channels",

    END_PARAMETER_DEFINITIONS

    BEGIN_STREAM_DEFINITIONS
        "Channel1Stream 32 0",
    END_STREAM_DEFINITIONS
}

void
BufferedADCTestADC::OnAutoConfig()
{
    Parameter("SourceCh") = 16;
    int channels = ActualParameter("SourceCh");
    Parameter("SourceChGain")->SetNumValues(channels);
    Parameter("SourceChOffset")->SetNumValues(channels);
    for (int i = 0; i < channels; ++i)
    {
        double gainFactor = 1.0;
        Parameter("SourceChGain")(i) << gainFactor << "muV";
        Parameter("SourceChOffset")(i) = 0;
    }
    Parameter("SampleBlockSize") = 8;
    Parameter("SamplingRate") = "256Hz";
}

void
BufferedADCTestADC::OnPreflight(SignalProperties& Output) const
{
    SignalType sigType = SignalType::float32;

    int samplesPerBlock = Output.Elements();
    int numberOfSignalChannels = Output.Channels();
    int numberOfSyncStates = 1;
    Output = SignalProperties(numberOfSignalChannels + numberOfSyncStates, samplesPerBlock, sigType);
    // A channel name starting with @ indicates a trigger channel.
    Output.ChannelLabels()[numberOfSignalChannels] = "@Channel1Stream";
}

void
BufferedADCTestADC::OnInitialize(const SignalProperties& Output)
{
    mNumberOfSignalChannels = Output.Channels() - 1;
    mSamplingRate = Parameter("SamplingRate").InHertz();
    mClock.SetInterval(Time::Seconds(Output.Elements() / mSamplingRate));
}

void
BufferedADCTestADC::OnStartAcquisition()
{
    mClock.Start();
    mTimeSeconds = 0;
}

void
BufferedADCTestADC::DoAcquire(GenericSignal& Output)
{
    mClock.Wait();
    auto* pOutput = Output.MutableData(GenericSignal::intentOverwriteContents);
    for (int ch = 0; ch < mNumberOfSignalChannels; ch++)
    {
        for (int sample = 0; sample < Output.Elements(); sample++)
        {
            int idx = Output.LinearIndex(ch, sample);
            pOutput[idx] = 100 * std::sin((mTimeSeconds + (sample / mSamplingRate)) * (ch + 1) * 2 * M_PI);
        }
    }
    mTimeSeconds += Output.Elements() / mSamplingRate;
    mTimeSeconds = std::fmod(mTimeSeconds, 1);
    for (int sample = 0; sample < Output.Elements(); sample++)
    {
        pOutput[Output.LinearIndex(mNumberOfSignalChannels, sample)] = pOutput[Output.LinearIndex(0, sample)] + (1UL << 31);
    }
}

void
BufferedADCTestADC::OnStopAcquisition()
{
    mClock.Stop();
}

