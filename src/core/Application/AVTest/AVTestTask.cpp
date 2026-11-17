////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: An application module for AV testing purposes.
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
#include "AVTestTask.h"
#include "BCIStream.h"

RegisterFilter(AVTestTask, 3);

AVTestTask::AVTestTask() :
    mrDisplay(Window()),
    mOnSinkTick(&AVTestTask::OnSinkTick, this)
{
}

AVTestTask::~AVTestTask()
{
    Halt();
}

void
AVTestTask::Publish()
{
    BEGIN_PARAMETER_DEFINITIONS
        "Application:AVTest float SpinningWheel= 10 % % %"
        " // display a spinning wheel with angular velocity of x degrees per frame",
        "Application:AVTest int ShowDisplayStatistics= 1 0 0 1 "
        " // add an overlay with statistical information (boolean)",
        "Application:AVTest float TestSoundVolume= 0.5 0 0 1 "
        " // test sound volume between 0 and 1",
    END_PARAMETER_DEFINITIONS
}

void
AVTestTask::Preflight(const SignalProperties& Input, SignalProperties& Output) const
{
    Output = Input;
}


void
AVTestTask::Initialize(const SignalProperties& Input, const SignalProperties& Output)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mpStream = nullptr;
    mpSink = *mrDisplay.Display().AssociatedAudio();
    if (mpSink)
        mpStream = mpSink->Stream(); // this will start the stream if it has not been started yet

    mVolume = Parameter("TestSoundVolume");
    if (mVolume > 1)
        mVolume = 1;
    else if (mVolume < 0)
        mVolume = 0;

    if (mpStream) {
        mSynth.SetVolume(0).SetPan(0);
        mSynth.SetPitchHz(1000);
        mSynth.SetChannels(mpStream->PlaybackChannels());
        mSynth.SetSamplingRateHz(mpStream->SampleRate().Hertz());
        mSynth.SetWaveform(SimpleSynth::sine);
    }

    if (mpSink)
        mpSink->AddListener(AudioSink::OnTick, &mOnSinkTick);
}

void
AVTestTask::StartRun()
{
    std::unique_lock<std::mutex> lock(mMutex);
    mSynth.SetVolume(1);
}


void
AVTestTask::Process(const GenericSignal& Input, GenericSignal& Output)
{
    Output = Input;
}

void
AVTestTask::StopRun()
{
    std::unique_lock<std::mutex> lock(mMutex);
    mSynth.SetVolume(0);
}

void
AVTestTask::Halt()
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mpSink) {
        mpSink->RemoveListener(AudioSink::OnTick, &mOnSinkTick);
    }
    mpStream = nullptr; // this will stop the stream if no other clients are using it
    mpSink = nullptr;
}

void
AVTestTask::OnSinkTick()
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mpStream)
        return;
    if (!mpSink)
        return;

    auto buf = mpSink->MixBuffer();
    mSampleBuffer.resize(buf.RemainingFrames() * mSynth.Channels());
    mSynth.ProduceSamples(mSampleBuffer);
    buf.Put(mSampleBuffer.data(), buf.RemainingFrames(), 0, mVolume);
}
