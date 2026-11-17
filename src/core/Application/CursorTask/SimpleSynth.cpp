////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A simple sound synthesizer.
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
#include "SimpleSynth.h"
#include "Numeric.h"

SimpleSynth& SimpleSynth::SetChannels(int ch)
{
    mChannels = ch;
    return *this;
}

int SimpleSynth::Channels() const
{
    return mChannels;
}

SimpleSynth& SimpleSynth::SetSamplingRateHz(float sr)
{
    mSamplingRate = sr;
    return *this;
}

float SimpleSynth::SamplingRateHz() const
{
    return mSamplingRate;
}

SimpleSynth& SimpleSynth::SetPitchHz(float pitch)
{
    mPitch = pitch;
    return *this;
}

float SimpleSynth::PitchHz() const
{
    return mPitch;
}

SimpleSynth& SimpleSynth::SetWaveform(WaveformType waveform)
{
    mWaveform = waveform;
    return *this;
}

SimpleSynth::WaveformType SimpleSynth::Waveform() const
{
    return mWaveform;
}

SimpleSynth& SimpleSynth::SetVolume(float vol)
{
    if (vol < 0)
        vol = 0;
    if (vol > 1)
        vol = 1;
    mVolume = vol;
    return *this;
}

float SimpleSynth::Volume() const
{
    return mVolume;
}

SimpleSynth& SimpleSynth::SetPan(float pan)
{
    if (pan < -1)
        pan = -1;
    if (pan > 1)
        pan = 1;
    mPan = pan;
    return *this;
}

float SimpleSynth::Pan() const
{
    return mPan;
}

SimpleSynth& SimpleSynth::ProduceSamples(std::vector<float>& buffer)
{
    float delta = mPitch / mSamplingRate;
    int sampleCount = buffer.size() / mChannels;
    std::vector<float> channelWeights(mChannels, 1);
    if (mChannels > 1) {
        for (int i = 0; i < mChannels; ++i) {
            // arrange channels on left-right axis, left is -1, right is 1
            float channelPos = -1 + i * 2.0 / (mChannels - 1);
            float channelWeight = ::sqrt(1 + mPan * channelPos) * ::sqrt(0.5);
            channelWeights[i] = channelWeight;
        }
    }
    for (int i = 0; i < sampleCount; ++i) {
        float val = 0.0;
        switch (mWaveform) {
            case sine:
                val = std::sin(mPhase * 2 * Pi<float>());
                break;
            case sawtooth:
                val = mPhase - 0.5;
                break;
            case square:
                val = mPhase < 0.5 ? -1 : 1;
                break;
        }
        mPhase += delta;
        mPhase = std::fmod(mPhase, 1);
        val *= mVolume;
        for (int j = 0; j < mChannels; ++j) {
            buffer[i * mChannels + j] = channelWeights[j] * val;
        }
    }
    return *this;
}

