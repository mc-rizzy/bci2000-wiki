////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: An audio player variant that allows time-stretching.
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
#include "TimeStretchAudioPlayer.h"

#include "AVDecoder.h"
#include "AudioSink.h"
#include "SoundTouch.h"

#include <vector>
#include <mutex>

struct TimeStretchAudioPlayer::Private
{
    void Initialize(TimeStretchAudioPlayer*);

    TimeStretchAudioPlayer* mpSelf = nullptr;
    soundtouch::SoundTouch mSoundTouch;
    std::vector<float> mBuffer;
    std::mutex mMutex;

    double mTempo = 1.0, mPitch = 1.0;
};

// TimeStretchAudioPlayer
TimeStretchAudioPlayer::TimeStretchAudioPlayer()
: p(new Private)
{
    Initialize(nullptr);
    p->Initialize(this);
}

TimeStretchAudioPlayer::TimeStretchAudioPlayer(AudioSinkRef pSink)
: AudioPlayer(false)
, p(new Private)
{
    Initialize(pSink);
    p->Initialize(this);
}

TimeStretchAudioPlayer::~TimeStretchAudioPlayer()
{
    delete p;
}

TimeStretchAudioPlayer& TimeStretchAudioPlayer::SetPitch(double pitch)
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    p->mSoundTouch.setPitch(pitch);
    p->mPitch = pitch;
    return *this;
}

double TimeStretchAudioPlayer::Pitch() const
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    return p->mPitch;
}

TimeStretchAudioPlayer& TimeStretchAudioPlayer::SetTempo(double tempo)
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    p->mSoundTouch.setTempo(tempo);
    p->mTempo = tempo;
    return *this;
}

double TimeStretchAudioPlayer::Tempo() const
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    return p->mTempo;
}

const char *TimeStretchAudioPlayer::OnRenderingTick(const ClockTick &, bool)
{
    auto pStream = p->mpSelf->Sink()->Stream();
    if (!pStream->Error().Empty())
        return pStream->Error().c_str();

    if (IsPlaying())
    {
        auto mixBuffer = p->mpSelf->Sink()->MixBuffer();
        int remaining = mixBuffer.RemainingFrames();
        while (IsPlaying() && remaining > 0 && !Decoder().EndOfData())
        {
            while (!Decoder().AreFramesAvailable() && !Decoder().EndOfData())
                SpendDecodingEffort();

            std::unique_lock<std::mutex> lock(p->mMutex);

            int inputFrames = std::round(remaining / p->mSoundTouch.getInputOutputSampleRatio());
            inputFrames = std::max(inputFrames, 1);
            AVDecoder::Chunk chunk = Decoder().GetFrames(inputFrames);
            const float* pBegin = nullptr, *pEnd = nullptr;
            chunk.GetData(pBegin, pEnd);
            int frames = chunk.FrameCount();

            p->mSoundTouch.putSamples(pBegin, frames);
            chunk.Release();

            int maxSamples = p->mBuffer.size() / p->mSoundTouch.numChannels();
            maxSamples = std::min(maxSamples, remaining);
            int receivedSamples = p->mSoundTouch.receiveSamples(p->mBuffer.data(), maxSamples);

            lock.unlock();

            Settings().Update();
            mixBuffer.Put(p->mBuffer.data(), receivedSamples, 0, Settings().mVolumeFactor);
            remaining = mixBuffer.RemainingFrames();

            while (frames > 0 && !Decoder().EndOfData())
            {
                if (!Decoder().AreFramesAvailable())
                    SpendDecodingEffort();
                frames -= Decoder().AdvanceByFrames(frames);
            }
        }
        auto tick = FramePositionTick();
        tick.count = Decoder().HeadFrameTick();
        SetFramePositionTick(tick);
    }
    if (!IsPlaying())
    {
        auto tick = FramePositionTick();
        tick.time = Time::Never();
        SetFramePositionTick(tick);
    }
    return Decoder().Error().Empty() ? nullptr : Decoder().Error().c_str();
}

const char *TimeStretchAudioPlayer::OnAdvance(int)
{
    return nullptr;
}

void TimeStretchAudioPlayer::OnPlay()
{
    p->mSoundTouch.clear();
}

void TimeStretchAudioPlayer::Private::Initialize(TimeStretchAudioPlayer* pSelf)
{
    mpSelf = pSelf;
    auto pStream = mpSelf->Sink()->Stream();

    std::unique_lock<std::mutex> lock(mMutex);
    mBuffer.resize(pStream->SamplesInBuffer() * pStream->PlaybackChannels());

    mSoundTouch.setSampleRate(pStream->SampleRate().Hertz());
    mSoundTouch.setChannels(pStream->PlaybackChannels());
    mSoundTouch.setRate(1.0);
    mSoundTouch.setPitch(mPitch);
    mSoundTouch.setTempo(mTempo);
}

