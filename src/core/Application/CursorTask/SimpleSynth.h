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
#ifndef SIMPLE_SYNTH_H
#define SIMPLE_SYNTH_H

#include <vector>

class SimpleSynth
{
public:
    enum WaveformType {
        sine,
        sawtooth,
        square,
    };

    SimpleSynth& SetChannels(int);
    int Channels() const;

    SimpleSynth& SetSamplingRateHz(float);
    float SamplingRateHz() const;

    SimpleSynth& SetPitchHz(float);
    float PitchHz() const;

    SimpleSynth& SetWaveform(WaveformType);
    WaveformType Waveform() const;

    // Volume is between 0 and 1
    SimpleSynth& SetVolume(float);
    float Volume() const;

    // Pan is between -1 and 1
    SimpleSynth& SetPan(float);
    float Pan() const;

    SimpleSynth& ProduceSamples(std::vector<float>&);

private:
    float mChannels = 1,
          mSamplingRate = 44100,
          mPitch = 440,
          mVolume = 1,
          mPan = 0,
          mPhase = 0;
    WaveformType mWaveform = sine;
};

#endif // SIMPLE_SYNTH_H
