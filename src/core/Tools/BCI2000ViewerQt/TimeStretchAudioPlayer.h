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
#ifndef TIME_STRETCH_AUDIO_PLAYER_H
#define TIME_STRETCH_AUDIO_PLAYER_H

#include "AudioPlayer.h"

class TimeStretchAudioPlayer : public AudioPlayer
{
  public:
    TimeStretchAudioPlayer();
    TimeStretchAudioPlayer(AudioSinkRef);
    virtual ~TimeStretchAudioPlayer();

    // 1.0: original pitch
    TimeStretchAudioPlayer& SetPitch(double);
    double Pitch() const;

    // 1.0: original tempo
    TimeStretchAudioPlayer& SetTempo(double);
    double Tempo() const;

  protected:
    const char *OnRenderingTick(const ClockTick &, bool) override;
    const char *OnAdvance(int) override;

    void OnPlay() override;

  private:
    using AudioPlayer::SampleLag;
    using AudioPlayer::SetResamplingLimit;
    using AudioPlayer::ResamplingLimit;

    struct Private;
    Private *p;
};

#endif // TIME_STRETCH_AUDIO_PLAYER_H
