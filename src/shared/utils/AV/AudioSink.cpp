////////////////////////////////////////////////////////////////////////////////
// $Id: AudioSink.cpp 9216 2026-01-01 14:20:40Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An object through which multiple AudioPlayer instances may
//   render audio into a single AudioStream.
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
#include "AudioSink.h"

#include "AudioDevice.h"
#include "Debugging.h"
#include "Runnable.h"
#include "SynchronizedObject.h"
#include <atomic>
#include <cmath>
#include <mutex>

struct AudioSink::Private
{
    Private(AudioSink *);

    void OnStreamTick();
    MemberCall<void(AudioSink::Private *)> mOnStreamTick;
    void OnStreamAbort();
    MemberCall<void(AudioSink::Private *)> mOnStreamAbort;

    void AddStreamRef();
    void ReleaseStream();
    std::atomic<int> mStreamRefCount{ 0 };

    AudioSink *mpSelf;
    AudioStream mStream;
    std::vector<float> mMixBuffer;
    std::mutex mMixBufferMutex;
    bool mSilent;
    int mSamplesInBuffer;
};

AudioSink::StreamRef::StreamRef(std::nullptr_t) : mp(nullptr)
{
}
AudioSink::StreamRef::StreamRef(AudioSink *p) : mp(p->p)
{
    if (mp)
        mp->AddStreamRef();
}
AudioSink::StreamRef::StreamRef(const StreamRef &other) : mp(other.mp)
{
    mp->AddStreamRef();
}
AudioSink::StreamRef &AudioSink::StreamRef::operator=(const StreamRef &other)
{
    if (mp != other.mp) {
        if (mp)
            mp->ReleaseStream();
        mp = other.mp;
        if (mp)
            mp->AddStreamRef();
    }
    return *this;
}
AudioSink::StreamRef::~StreamRef()
{
    if (mp)
        mp->ReleaseStream();
    mp = nullptr;
}
AudioSink::StreamRef::operator const AudioStream *() const
{
    return mp ? &mp->mStream : nullptr;
}
const AudioStream *AudioSink::StreamRef::operator->() const
{
    Assert(mp);
    return &mp->mStream;
}
const AudioStream &AudioSink::StreamRef::operator*() const
{
    Assert(mp);
    return mp->mStream;
}
AudioSink::StreamRef::operator AudioStream *()
{
    return mp ? &mp->mStream : nullptr;
}
AudioStream *AudioSink::StreamRef::operator->()
{
    Assert(mp);
    return &mp->mStream;
}
AudioStream &AudioSink::StreamRef::operator*()
{
    Assert(mp);
    return mp->mStream;
}
AudioSink &AudioSink::StreamRef::Sink()
{
    Assert(mp);
    return *mp->mpSelf;
}
const AudioSink &AudioSink::StreamRef::Sink() const
{
    Assert(mp);
    return *mp->mpSelf;
}

AudioSink::BufferRef::BufferRef(AudioSink *p) : mp(p->p), mpData(mp ? mp->mMixBuffer.data() : nullptr)
{
    if (mp)
        mp->AddStreamRef();
}
AudioSink::BufferRef::BufferRef(const BufferRef &other) : mp(other.mp), mpData(other.mpData)
{
    if (mp)
        mp->AddStreamRef();
}
AudioSink::BufferRef::~BufferRef()
{
    if (mp)
        mp->ReleaseStream();
}

int AudioSink::BufferRef::RemainingFrames() const
{
    if (!mp)
        return 0;
    return (mp->mMixBuffer.data() + mp->mMixBuffer.size() - mpData) / mp->mStream.PlaybackChannels();
}

void AudioSink::BufferRef::Put(const float* inpBegin, int inFrameCount, int inDupOrDrop, float inVolumeFactor)
{
    Assert(mp);
    int channels = mp->mStream.PlaybackChannels();
    int framesToWrite = inFrameCount + inDupOrDrop;
    Assert(framesToWrite <= RemainingFrames());
    if (framesToWrite == 1)
    {
        int rightIdx = inFrameCount - 1;
        for (int ch = 0; ch < channels; ++ch)
        {
            float leftValue = inpBegin[ch];
            float rightValue = inpBegin[rightIdx * channels + ch];
            mpData[ch] += inVolumeFactor * (leftValue + rightValue) / 2.0f;
        }
    }
    else for (int frame = 0; frame < framesToWrite; ++frame)
    {
        float inputPos = (frame * (inFrameCount - 1)) * 1.0f / (framesToWrite - 1);
        int leftIdx = ::floor(inputPos);
        int rightIdx = ::ceil(inputPos);
        float rightWeight = ::fmod(inputPos, 1.0f);
        float leftWeight = 1.0f - rightWeight;
        for (int ch = 0; ch < channels; ++ch)
        {
            float leftValue = inpBegin[leftIdx * channels + ch];
            float rightValue = inpBegin[rightIdx * channels + ch];
            mpData[frame * channels + ch] += inVolumeFactor * (leftWeight * leftValue + rightWeight * rightValue);
        }
    }
    mpData += channels * framesToWrite;
}

void AudioSink::BufferRef::Put(const AVDecoder::Chunk& inChunk, int inDupOrDrop, float inVolumeFactor)
{
    const float* pBegin, * pEnd;
    if (inChunk.GetData(pBegin, pEnd))
        Put(pBegin, inChunk.FrameCount(), inDupOrDrop, inVolumeFactor);
}

void AudioSink::BufferRef::GetContent(const float*& begin, const float*& end) const
{
    begin = nullptr;
    end = nullptr;
    if (mp)
    {
        begin = mp->mMixBuffer.data();
        end = begin + mp->mMixBuffer.size();
    }
}

AudioSink::Private::Private(AudioSink *p)
    : mpSelf(p), mOnStreamTick(&AudioSink::Private::OnStreamTick, this),
      mOnStreamAbort(&AudioSink::Private::OnStreamAbort, this), mSilent(true),
      mSamplesInBuffer(0)
{
}

void AudioSink::Private::AddStreamRef()
{
    if (++mStreamRefCount == 1)
    {
        int samples = mSamplesInBuffer;
        mStream.SetSamplesInBuffer(samples);
        mStream.Start();
        if (!mStream.Error().Empty())
            mStream.Destroy();
    }
}

void AudioSink::Private::ReleaseStream()
{
    if (--mStreamRefCount == 0)
    {
        mStream.Stop();
        mSilent = true;
    }
}

void AudioSink::Private::OnStreamTick()
{
    std::unique_lock<std::mutex> lock(mMixBufferMutex);
    mMixBuffer.resize(mStream.SamplesInBuffer() * mStream.PlaybackChannels());
    ::memset(mMixBuffer.data(), 0, mMixBuffer.size() * sizeof(*mMixBuffer.data()));

    int mixBufferSamples = mStream.SamplesInBuffer();
    ClockTick tick = mStream.PlaybackTick();
    mpSelf->Broadcaster::Emit(OnTick, &tick);

    Assert(mixBufferSamples == mStream.SamplesInBuffer());
    mStream.Write(mMixBuffer.data(), mixBufferSamples);
 
    if (mSilent)
    {
        const auto *p = mMixBuffer.data(), * pEnd = p + mMixBuffer.size();
        while (p < pEnd && *p == 0)
            ++p;
        if (p < pEnd)
        {
            mSilent = false;
            tick.time += tick.rate.Period() * Ratio(p - mMixBuffer.data(), mMixBuffer.size());
            mpSelf->Broadcaster::Emit(OnSound, &tick);
        }
    }
    else
    {
        const auto *pBegin = mMixBuffer.data(), * pEnd = pBegin + mMixBuffer.size(), * p = pEnd;
        while (p > pBegin && *(p - 1) == 0)
            --p;
        if (p < pEnd)
        {
            mSilent = true;
            tick.time += tick.rate.Period() * Ratio(p - mMixBuffer.data(), mMixBuffer.size());
            mpSelf->Broadcaster::Emit(OnSilence, &tick);
        }
    }
}

void AudioSink::Private::OnStreamAbort()
{
    std::cerr << "Audio stream aborted with error: " << mStream.Error() << std::endl;
}

AudioSink::AudioSink(const Rate &r, int ch, int s) : p(new Private(this))
{
    p->mSamplesInBuffer = s;
    p->mStream.SetSampleRate(r).SetPlaybackChannels(ch).SetRecordingChannels(0);
    p->mStream.AddListener(AudioStream::OnTick, &p->mOnStreamTick);
    p->mStream.AddListener(AudioStream::OnAbort, &p->mOnStreamAbort);
}

AudioSink::~AudioSink()
{
    p->mStream.RemoveListener(AudioStream::OnAbort, &p->mOnStreamAbort);
    p->mStream.RemoveListener(AudioStream::OnTick, &p->mOnStreamTick);
    p->mStream.Stop();
    SyncMemory<BeforeRead>();
    delete p;
}

void AudioSink::AddStreamListener(int msg, Tiny::Runnable *pRunnable)
{
    p->mStream.AddListener(msg, pRunnable);
}

void AudioSink::RemoveStreamListener(int msg, Tiny::Runnable *pRunnable)
{
    p->mStream.RemoveListener(msg, pRunnable);
}

const StringUtils::String &AudioSink::Error() const
{
    return p->mStream.Error();
}
