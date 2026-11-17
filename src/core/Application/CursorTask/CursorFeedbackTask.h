////////////////////////////////////////////////////////////////////////////////
// $Id: CursorFeedbackTask.h 9555 2026-09-15 13:16:09Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The CursorFeedback Application's Task filter.
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
#ifndef CURSOR_FEEDBACK_TASK_H
#define CURSOR_FEEDBACK_TASK_H

#include "ApplicationWindow.h"
#include "Color.h"
#include "FeedbackTask.h"
#include "TextField.h"
#include "TrialStatistics.h"
#include "NetworkHID.h"

#include "FeedbackScene.h"

class AudioFeedback;

class CursorFeedbackTask : public FeedbackTask
{
  public:
    CursorFeedbackTask();
    virtual ~CursorFeedbackTask();

  private:
    // Events to be handled by FeedbackTask descendants.
    //  Events triggered by the GenericFilter interface
    void OnPreflight(const SignalProperties &Input) const override;
    void OnInitialize(const SignalProperties &Input) override;
    void OnStartRun() override;
    void OnStopRun() override;
    void OnHalt() override;
    //  Events triggered during the course of a trial
    void OnTrialBegin() override;
    void OnTrialEnd() override;
    void OnFeedbackBegin() override;
    void OnFeedbackEnd() override;
    //  Dispatching of the input signal.
    //  Each call to GenericSignal::Process() is dispatched to one of these
    //  events, depending on the phase in the sequence.
    //  There, each handler function corresponds to a phase.
    //  If a handler sets the "progress" argument to true, the application's
    //  state will switch to the next phase.
    void DoPreRun(const GenericSignal &, bool &doProgress) override;
    void DoPreFeedback(const GenericSignal &, bool &doProgress) override;
    void DoFeedback(const GenericSignal &, bool &doProgress) override;
    void DoPostFeedback(const GenericSignal &, bool &doProgress) override;
    void DoITI(const GenericSignal &, bool &doProgress) override;

    std::unique_ptr<NetworkHID> mpNetworkHID;

  private:
    void MoveCursorTo(float x, float y, float z);
    void DisplayMessage(const std::string &);

    // Graphic objects
    ApplicationWindow &mrWindow;
    FeedbackScene *mpFeedbackScene;
    int mRenderingQuality;
    TextField *mpMessage;

    AudioFeedback *mpAudioFeedback;

    RGBColor mCursorColorFront, mCursorColorBack;
    int mRunCount, mTrialCount, mCurFeedbackDuration, mMaxFeedbackDuration;
    float mCursorSpeedX, mCursorSpeedY, mCursorSpeedZ;

    TrialStatistics mTrialStatistics;
};

#endif // CURSOR_FEEDBACK_TASK_H
