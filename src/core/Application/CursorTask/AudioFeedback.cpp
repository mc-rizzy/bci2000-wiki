////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A class for audio feedback in the CursorFeedbackTask filter.
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
#include "AudioFeedback.h"
#include "ApplicationWindow.h"

AudioFeedback::AudioFeedback(ApplicationWindow& display)
: mrDisplay(display),
  mOnSinkTick(&AudioFeedback::OnSinkTick, this)
{
}

AudioFeedback::~AudioFeedback()
{
    Halt();
}

void AudioFeedback::Initialize()
{
    mVolume = Parameter("AudioFeedbackVolume");
    mPitch = Parameter("AudioFeedbackPitch").InHertz();
    mRangeOctaves = Parameter("AudioFeedbackRange");

    mAnnounceHit.SetFile(Parameter("AudioFeedbackHitAnnouncement"));
    mAnnounceMiss.SetFile(Parameter("AudioFeedbackMissAnnouncement"));
    mAnnounceAbort.SetFile(Parameter("AudioFeedbackAbortAnnouncement"));

    AudioSinkRef pSink = *mrDisplay.Display().AssociatedAudio();
    AudioSink::StreamRef pStream = pSink->Stream();
    
    mSynth.SetVolume(0).SetPan(0);
    mSynth.SetPitchHz(Parameter("AudioFeedbackPitch").InHertz());
    mSynth.SetChannels(pStream->PlaybackChannels());
    mSynth.SetSamplingRateHz(pStream->SampleRate().Hertz());
    SimpleSynth::WaveformType type = SimpleSynth::WaveformType(Parameter("AudioFeedbackWaveform").ToNumber());
    mSynth.SetWaveform(type);
}

void AudioFeedback::AttachToAudioSink()
{
    std::unique_lock<std::mutex> lock(mMutex);
    mpSink = *mrDisplay.Display().AssociatedAudio();
    mpStream = mpSink->Stream(); // this will start the stream if it has not been started yet
    mpSink->AddListener(AudioSink::OnTick, &mOnSinkTick);
    // From now on, OnSinkTick() will be called whenever the audio stream requires data.
    // This happens inside a separate thread, so we need to protect our data members
    // from concurrent access across threads.
}

void AudioFeedback::Halt()
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mpSink) {
        mpSink->RemoveListener(AudioSink::OnTick, &mOnSinkTick);
    }
    mpStream = nullptr; // this will stop the stream if no other clients are using it
    mpSink = nullptr;
}

void AudioFeedback::Mute()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mSynth.SetVolume(0);
}

void AudioFeedback::Unmute()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mSynth.SetVolume(1);
}

void AudioFeedback::SetCursorPosition(const Vector3D& v)
{
    float pan = (v.x - 50.0f) / 50.0f;
    float logPitch = (v.y - 50.0f) / 100.0f;
    float pitch = mPitch * ::pow(2.0f, logPitch);
    float volume = (100.0f - v.z) / 100.0f;

    std::lock_guard<std::mutex> lock(mMutex);
    mSynth.SetPan(pan);
    mSynth.SetPitchHz(pitch);
    mSynth.SetVolume(volume);
}

void AudioFeedback::AnnounceAbort()
{
    mAnnounceAbort.SetVolume(mVolume);
    mAnnounceAbort.Play();
}

void AudioFeedback::AnnounceHit()
{
    mAnnounceHit.SetVolume(mVolume);
    mAnnounceHit.Play();
}

void AudioFeedback::AnnounceMiss()
{
    mAnnounceMiss.SetVolume(mVolume);
    mAnnounceMiss.Play();
}

void AudioFeedback::OnSinkTick()
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

