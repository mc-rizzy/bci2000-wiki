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
#ifndef AUDIO_FEEDBACK_H
#define AUDIO_FEEDBACK_H

#include "Environment.h"
#include "GraphDisplay.h"
#include "FeedbackScene.h"
#include "SimpleSynth.h"
#include "WavePlayer.h"

#include <vector>
#include <mutex>

class ApplicationWindow;

class AudioFeedback : private Environment
{
public:
    AudioFeedback(ApplicationWindow&);
    ~AudioFeedback();

    void Initialize();
    void AttachToAudioSink();
    void Halt();

    void Mute();
    void Unmute();

    void SetCursorPosition(const Vector3D&);
    void AnnounceAbort();
    void AnnounceHit();
    void AnnounceMiss();

private:
    void OnSinkTick();
    // mOnSinkTick wraps the OnSinkTick() function into a Runnable object
    // as required for the Broadcaster::AddListener() function.
    MemberCall<void(AudioFeedback*)> mOnSinkTick;

    float mVolume = 1.0, mPitch = 0, mRangeOctaves = 0;

    std::mutex mMutex;
    ApplicationWindow& mrDisplay;
    AudioSinkRef mpSink;
    AudioSink::StreamRef mpStream;
    std::vector<float> mSampleBuffer;
    SimpleSynth mSynth;
    WavePlayer mAnnounceAbort,
               mAnnounceHit,
               mAnnounceMiss;
};

#endif // AUDIO_FEEDBACK_H
