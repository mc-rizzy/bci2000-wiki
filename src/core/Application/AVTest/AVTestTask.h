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
#ifndef INCLUDED_AVTESTTASK_H
#define INCLUDED_AVTESTTASK_H

#include "ApplicationBase.h"
#include "AudioSink.h"
#include "SimpleSynth.h"

class AVTestTask : public ApplicationBase
{
public:
    AVTestTask();
    ~AVTestTask();
    void Publish() override;
    void Preflight(const SignalProperties& Input, SignalProperties& Output) const override;
    void Initialize(const SignalProperties& Input, const SignalProperties& Output) override;
    void StartRun() override;
    void Process(const GenericSignal& Input, GenericSignal& Output) override;
    void StopRun() override;
    void Halt() override;

private:
    void OnSinkTick();
    // mOnSinkTick wraps the OnSinkTick() function into a Runnable object
    // as required for the Broadcaster::AddListener() function.
    MemberCall<void(AVTestTask*)> mOnSinkTick;

    ApplicationWindow& mrDisplay;

    std::mutex mMutex;
    AudioSinkRef mpSink;
    AudioSink::StreamRef mpStream;
    std::vector<float> mSampleBuffer;
    SimpleSynth mSynth;
    float mVolume = 0;
};

#endif // INCLUDED_AVTESTTASK_H
