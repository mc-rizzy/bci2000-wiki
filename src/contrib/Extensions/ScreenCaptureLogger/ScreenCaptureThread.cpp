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
#include "ScreenCaptureThread.h"

#include "Capture.h"
#include "FileUtils.h"
#include "BCIEvent.h"

ScreenCaptureThread::ScreenCaptureThread(int index)
: mThreadFunc(&ScreenCaptureThread::ThreadFunc, this),
  mThread(&mThreadFunc, "Screen Capture Thread #" + std::to_string(index)),
  mIndex(index),
  mVis("ScreenCapture" + std::to_string(index))
{
    mVis.Send(CfgID::WindowTitle, "Screen Capture " + std::to_string(index));
}

ScreenCaptureThread::~ScreenCaptureThread()
{
    mThread.Terminate();
}

void ScreenCaptureThread::Initialize(int left, int top, int width, int height, float decimation, float fps, int profile, float compression)
{
    mThread.Terminate();

    mCapture.SetRect(left, top, width, height).SetDecimation(decimation);

    mClock.SetInterval(Time::Seconds(1.0 / fps));

    mProfile = profile;
    mCompression = compression;

    mThread.StartIfNotRunning();
}

void ScreenCaptureThread::StartupCaptureStream()
{
    mCapture.StartupCaptureStream();
}

void ScreenCaptureThread::ShowWindow(bool b)
{
    if (mWindowVisible != b) {
        mWindowVisible = b;
        mVis.Send(CfgID::Visible, b);
    }
}

bool ScreenCaptureThread::IsDefunct() const
{
    return mState == defunct;
}

bool ScreenCaptureThread::StartRecording(const std::string& filebase)
{
    switch (mState) {
    case idle:
        {
            std::lock_guard<std::mutex> lock(mNextFileNameMutex);
            mFileBase = filebase + "_ScreenCapture" + std::to_string(mIndex);
            mNextFileName = mFileBase + ".mp4";
        }
        mState = startRecordingPending;
        return true;
    default:
        ;
    }
    return false;
}

bool ScreenCaptureThread::StopRecording()
{
    switch (mState) {
    case recording:
        {
            std::lock_guard<std::mutex> lock(mNextFileNameMutex);
            mFileBase = "";
        }
        mState = stopRecordingPending;
        return true;
    default:
        ;
    }
    return false;
}

bool ScreenCaptureThread::SwitchToFilePart(int part)
{
    std::string fileName;
    {
        std::lock_guard<std::mutex> lock(mNextFileNameMutex);
        fileName = mFileBase;
    }
    if (fileName.empty()) {
        mError = "SwitchToFilePart() requires previous StartRecording()";
        return false;
    }
    if (part < 2) {
        fileName += ".mp4";
    }
    else {
        fileName += ".part" + std::to_string(part) + ".mp4";
    }

    switch (mState) {
    case recording:
        {
            std::lock_guard<std::mutex> lock(mNextFileNameMutex);
            mNextFileName = fileName;
        }
        mState = startRecordingPending;
        return true;
    default:
        ;
    }
    return false;
}

const std::string& ScreenCaptureThread::Error() const
{
    return mError;
}

void ScreenCaptureThread::OnImageCaptured(const void* pData)
{
    switch (mState) {
        case startRecordingPending:
            {
                std::unique_lock<std::mutex> lock(mNextFileNameMutex);
                OnStartRecording(mNextFileName);
            }
            mState = recording;
            OnRecord(pData);
            break;
        case stopRecordingPending:
            OnRecord(pData);
            OnStopRecording();
            mState = idle;
            break;
        case recording:
            OnRecord(pData);
            break;
        default:
            ;
    }
    if (mWindowVisible) {
        mVisFrame.AssignData(mCapture.GetOutputWidth(), mCapture.GetOutputHeight(), pData);
        mVis.Send(mVisFrame);
    }
}

void ScreenCaptureThread::OnStartRecording(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mWriterMutex);
    CloseFile();
    OpenFile(name);
    mImageCount = 0;
    mEncoder.BeginWriting();
    if (!mEncoder.Error().empty()) {
        throw std::runtime_error(mEncoder.Error());
    }
}

void ScreenCaptureThread::OnStopRecording()
{
    std::lock_guard<std::mutex> lock(mWriterMutex);
    CloseFile();
}

void ScreenCaptureThread::OnRecord(const void* pData)
{
    if (mState == recording || mState == startRecordingPending)
        bcievent << "ScreenCaptureFrame" << mIndex << " " << mImageCount;
    std::lock_guard<std::mutex> lock(mWriterMutex);
    auto ts = mClock.Interval() * mImageCount++;
    int bytes = mCapture.GetImageBytes();
    mEncoder.WriteFrame(mStreamIdx, pData, bytes, ts);
}

void ScreenCaptureThread::OpenFile(const std::string& filename)
{
    std::string fullfile = FileUtils::StripExtension(filename) + ".mp4";
    if (FileUtils::Exists(fullfile)) {
        throw std::runtime_error("Output file \"" + fullfile + "\" exists, will not be touched.");
    }
    if (!FileUtils::IsWritableLocation(fullfile)) {
        throw std::runtime_error("Output file \"" + fullfile + "\" cannot be created at the given location.");
    }

    mEncoder.OpenFile(fullfile);
    if (!mEncoder.Error().empty()) {
        throw std::runtime_error(mEncoder.Error());
    }

    mEncoder.AddVideoStream(&mStreamIdx);
    if (!mEncoder.Error().empty()) {
        throw std::runtime_error(mEncoder.Error());
    }

    Rate fileFps(1, mClock.Interval());
    AVEncoder::Options options({
        {"tune", "animation"},
    });
    mEncoder.InitializeVideoStream(mStreamIdx, mCapture.GetOutputWidth(), mCapture.GetOutputHeight(), fileFps, mProfile, mCompression, options);
    if (!mEncoder.Error().empty()) {
        throw std::runtime_error(mEncoder.Error());
    }
}

void ScreenCaptureThread::CloseFile()
{
    if (mStreamIdx >= 0) {
        mEncoder.FinalizeStream(mStreamIdx);
        mStreamIdx = -1;
    }
    if (!mEncoder.Error().empty()) {
        throw std::runtime_error(mEncoder.Error());
    }

    mEncoder.CloseFile();
    if (!mEncoder.Error().empty()) {
        throw std::runtime_error(mEncoder.Error());
    }
}

void ScreenCaptureThread::ThreadFunc()
{
    try {
        std::vector<uint8_t> image(mCapture.GetImageBytes());
        mImageCount = 0;
        mState = idle;

        mClock.Start();
        while (mClock.Wait()) {
            mCapture.DoCapture(image.data());
            OnImageCaptured(image.data());
        }
    }
    catch (const std::exception& e) {
        mError = e.what();
    }
    mClock.Stop();
    mState = mError.empty() ? idle : defunct;
}
