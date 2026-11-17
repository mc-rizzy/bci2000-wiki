/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A helper class representing a single screen capture, writing
//   into a file, and visualizing into the Operator module
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
/////////////////////////////////////////////////////////////////////////////
#ifndef SCREENCAPTURE_THREAD_H
#define SCREENCAPTURE_THREAD_H

#include "AVEncoder.h"
#include "Thread.h"
#include "Runnable.h"
#include "Clock.h"
#include "gui/GUI.h"
#include "GenericVisualization.h"
#include "VideoFrame.h"
#include "Capture.h"

#include <atomic>
#include <mutex>

class ScreenCaptureThread
{
public:
    ScreenCaptureThread(int index);
    ~ScreenCaptureThread();

    ScreenCaptureThread(const ScreenCaptureThread&) = delete;
    ScreenCaptureThread& operator=(const ScreenCaptureThread&) = delete;

    void Initialize(int left, int top, int width, int height, float decimation, float fps, int profile, float compression);
    // This function exists purely for Wayland reasons
    void StartupCaptureStream();

    void ShowWindow(bool = true);
    bool IsDefunct() const;

    bool StartRecording(const std::string& filebase);
    bool SwitchToFilePart(int);
    bool StopRecording();

    const std::string& Error() const;

protected:
    void OnImageCaptured(const void*);
    void OnStartRecording(const std::string& name);
    void OnStopRecording();
    void OnRecord(const void*);

private:
    void OpenFile(const std::string&);
    void CloseFile();

    void ThreadFunc();
    MemberCall<void (ScreenCaptureThread*)> mThreadFunc;
    Thread mThread;

    enum { 
        idle,
        startRecordingPending,
        recording,
        stopRecordingPending,
        defunct,
    };
    std::atomic<int> mState = defunct;

    int mIndex = 0;

    Capture mCapture;

    GenericVisualization mVis;
    bool mWindowVisible = false;
    VideoFrame mVisFrame;

    int64_t mImageCount = 0;

    std::string mFileBase;
    std::string mNextFileName;
    std::mutex mNextFileNameMutex;

    std::mutex mWriterMutex;
    int mProfile = 0;
    float mCompression = 8;
    GUI::Rect mRect = {0};
    int mDecimation = 1;
    Clock mClock;

    int mStreamIdx = -1;
    AVEncoder mEncoder;

    std::string mError;
};

#endif // SCREENCAPTURE_THREAD_H
