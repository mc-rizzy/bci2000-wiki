////////////////////////////////////////////////////////////////////////////////
// $Id: AudioPlayer.h 9415 2026-06-17 17:12:10Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An object that plays audio content into an AudioSink.
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
#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include "AVPlayer.h"

struct AudioSinkRef;

class AudioControls
{
  public:
    enum
    {
        Volume,
        ControlCount
    };
    AudioControls(void *);
    AudioControls &Set(int, float);
    AudioControls &Set(int, int, float);
    float Get(int) const;
    float Get(int, int) const;

  private:
    void *p;
};

class AudioPlayer : public AVPlayer
{
  protected:
    AudioPlayer(bool autoInitialize);
    // Use autoInitialize = false from a derived class to make sure
    // the class is fully constructed before callbacks are being called.
    // In your derived class' constructor, call Initialize() to 
    // register callbacks so they are called by the sink.

  public:
    AudioPlayer();
    AudioPlayer(AudioSinkRef);
    virtual ~AudioPlayer();

    AudioSinkRef Sink() const;

    AudioPlayer &SetVolume(float);
    float Volume() const;

    AudioControls &Controls();
    const AudioControls &Controls() const;

    // Maximum relative count of samples to insert or remove
    // Default is 0.25e-2 which corresponds 1/24 musical semitone
    AudioPlayer &SetResamplingLimit(float);
    float ResamplingLimit() const;

    int Channels() const;
    int NativeChannels() const;

    int64_t Samples() const;
    double SamplesPerSecond() const;
    double NativeSamplesPerSecond() const;

    int SampleLag() const;

  protected:
    struct AudioSettings
    {
        AudioSettings();
        void Update();

        float mVolumeFactor, mResamplingLimit;
        AudioControls mControls;
        float mValues[AudioControls::ControlCount];
        int32_t mChanged;
    };
    AudioSettings& Settings();

    const char *OnSetFile(const std::string &);
    const char *OnRenderingTick(const ClockTick &, bool);
    const char *OnAdvance(int);
    const char *OnGetDeviceInfo(std::string &) const;

    // Use this in a derived class' constructor
    // after AudioPlayer has been instantiated 
    // with an autoInitialize = false argument.
    void Initialize(AudioSinkRef);

    void SetFramePositionTick(const ClockTick&);
    ClockTick FramePositionTick() const;
    void SetSampleLag(int);

  private:
    struct Data;
    Data *d;
};

#endif // AUDIO_PLAYER_H
