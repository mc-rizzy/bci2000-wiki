//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: An AVBufferedEncoder wrapper that feeds data to the encoder
//   using a queue.
//   Unlike AVBufferedEncoder, this class only supports one video and one
//   audio stream.
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
///////////////////////////////////////////////////////////////////////
#include "AVBufferedEncoder.h"

#include <queue>
#include <mutex>
#include <string>
#include <variant>

#include "Thread.h"
#include "Runnable.h"
#include "WaitableEvent.h"

struct AVBufferedEncoder::Private
{
    struct OpenFile_args
    {
        std::string file;
    };
    struct BeginWriting_args
    {
    };
    struct CloseFile_args
    {
    };
    struct InitializeVideoStream_args
    {
        int width, height;
        Rate fps;
        int codec, preset, compressionFactor;
        AVEncoder::Options options;
    };
    struct WriteVideoFrame_args
    {
        void* pData;
        int bytes;
        Time::Interval timestamp;
    };

    struct InitializeAudioStream_args
    {
        int sampleblock, channels;
        Rate sps;
        int codec;
        AVEncoder::Options options;
    };
    struct WriteAudioFrame_args
    {
        void* pData;
        int bytes;
        Time::Interval timestamp;
    };

    typedef std::variant<
        OpenFile_args,
        BeginWriting_args,
        CloseFile_args,
        InitializeVideoStream_args,
        WriteVideoFrame_args,
        InitializeAudioStream_args,
        WriteAudioFrame_args
    > QueueElement;
    std::queue<QueueElement> mCommandQueue;
    std::mutex mCommandQueueMutex;

    int mVideoStreamIdx = -1, mAudioStreamIdx = -1;
    bool mIsFileOpen = false;
    AVEncoder mEncoder;

    std::string mError;
    std::mutex mErrorMutex;

    Tiny::MemberCall<void(Private*)> mThreadFunc = {&Private::ThreadFunc, this};
    Thread mEncoderThread = {&mThreadFunc, "AVBufferedEncoder Thread"};
    WaitableEvent mCommandAvailable = {true};

    Private();
    ~Private();

    void OpenFile(const OpenFile_args&);
    void BeginWriting(const BeginWriting_args&);
    void CloseFile(const CloseFile_args&);

    void InitializeVideoStream(const InitializeVideoStream_args&);
    void WriteVideoFrame(const WriteVideoFrame_args&);
    void InitializeAudioStream(const InitializeAudioStream_args&);
    void WriteAudioFrame(const WriteAudioFrame_args&);

    void DispatchCommand(const QueueElement&);
    void ClearCommandQueue();

    void ThreadFunc();
};

AVBufferedEncoder::Private::Private()
{
    mEncoderThread.Start();
}

AVBufferedEncoder::Private::~Private()
{
    mEncoderThread.Terminate();
    ClearCommandQueue();
}

void AVBufferedEncoder::Private::OpenFile(const OpenFile_args &args)
{
    if (mIsFileOpen) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "File already open, call CloseFile() first";
        return;
    }
    mEncoder.OpenFile(args.file);
    std::unique_lock<std::mutex> lock(mErrorMutex);
    mError = mEncoder.Error();
    if (mError.empty()) {
        mIsFileOpen = true;
    }
}

void AVBufferedEncoder::Private::BeginWriting(const BeginWriting_args &)
{
    if (!mIsFileOpen) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "File is not open, call OpenFile() first";
        return;
    }
    if (mVideoStreamIdx < 0 && mAudioStreamIdx < 0) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "File contains no streams";
        return;
    }
    mEncoder.BeginWriting();
    std::unique_lock<std::mutex> lock(mErrorMutex);
    mError = mEncoder.Error();
}

void AVBufferedEncoder::Private::CloseFile(const CloseFile_args &)
{
    if (!mIsFileOpen) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "File is not open, call OpenFile() first";
        return;
    }
    if (mVideoStreamIdx >= 0) {
        mEncoder.FinalizeStream(mVideoStreamIdx);
    }
    if (mAudioStreamIdx >= 00) {
        mEncoder.FinalizeStream(mAudioStreamIdx);
    }
    mEncoder.CloseFile();
    std::unique_lock<std::mutex> lock(mErrorMutex);
    mError = mEncoder.Error();
    mIsFileOpen = false;
    mVideoStreamIdx = -1;
    mAudioStreamIdx = -1;
}

void AVBufferedEncoder::Private::InitializeVideoStream(const InitializeVideoStream_args &args)
{
    if (!mIsFileOpen) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "File is not open, call OpenFile() first";
        return;
    }
    if (mVideoStreamIdx >= 0) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "A video stream exists, can not have more than one video stream";
        return;
    }
    mEncoder.AddVideoStream(&mVideoStreamIdx, args.codec);
    if (mEncoder.Error().empty()) {
        mEncoder.InitializeVideoStream(mVideoStreamIdx, args.width, args.height, args.fps, args.preset, args.compressionFactor, args.options);
    }
    std::unique_lock<std::mutex> lock(mErrorMutex);
    mError = mEncoder.Error();
}

void AVBufferedEncoder::Private::WriteVideoFrame(const WriteVideoFrame_args &cmd)
{
    if (!mIsFileOpen) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "File is not open, call OpenFile() first";
    }
    else if (mVideoStreamIdx < 0) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "No video stream exists, call InitializeVideoStream() first";
    }
    else {
        mEncoder.WriteFrame(mVideoStreamIdx, cmd.pData, cmd.bytes, cmd.timestamp);
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = mEncoder.Error();
    }
    delete [] static_cast<uint8_t*>(cmd.pData);
}

void AVBufferedEncoder::Private::InitializeAudioStream(const InitializeAudioStream_args &args)
{
    if (!mIsFileOpen) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "File is not open, call OpenFile() first";
        return;
    }
    if (mAudioStreamIdx >= 0) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "An audio stream exists, can not have more than one audio stream";
        return;
    }
    mEncoder.AddAudioStream(&mAudioStreamIdx, args.codec);
    if (mEncoder.Error().empty()) {
        mEncoder.InitializeAudioStream(mAudioStreamIdx, args.sampleblock, args.channels, args.sps, args.options);
    }
    std::unique_lock<std::mutex> lock(mErrorMutex);
    mError = mEncoder.Error();
}

void AVBufferedEncoder::Private::WriteAudioFrame(const WriteAudioFrame_args &cmd)
{
    if (!mIsFileOpen) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "File is not open, call OpenFile() first";
    }
    else if (mAudioStreamIdx < 0) {
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = "No audio stream exists, call InitializeAudioStream() first";
    }
    else {
        mEncoder.WriteFrame(mAudioStreamIdx, cmd.pData, cmd.bytes, cmd.timestamp);
        std::unique_lock<std::mutex> lock(mErrorMutex);
        mError = mEncoder.Error();
    }
    delete [] static_cast<uint8_t*>(cmd.pData);
}

void AVBufferedEncoder::Private::DispatchCommand(const QueueElement& cmd)
{
    if (std::holds_alternative<OpenFile_args>(cmd)) {
        OpenFile(std::get<OpenFile_args>(cmd));
    }
    else if (std::holds_alternative<BeginWriting_args>(cmd)) {
        BeginWriting(std::get<BeginWriting_args>(cmd));
    }
    else if (std::holds_alternative<CloseFile_args>(cmd)) {
        CloseFile(std::get<CloseFile_args>(cmd));
    }
    else if (std::holds_alternative<InitializeVideoStream_args>(cmd)) {
        InitializeVideoStream(std::get<InitializeVideoStream_args>(cmd));
    }
    else if (std::holds_alternative<WriteVideoFrame_args>(cmd)) {
        WriteVideoFrame(std::get<WriteVideoFrame_args>(cmd));
    }
    else if (std::holds_alternative<InitializeAudioStream_args>(cmd)) {
        InitializeAudioStream(std::get<InitializeAudioStream_args>(cmd));
    }
    else if (std::holds_alternative<WriteAudioFrame_args>(cmd)) {
        WriteAudioFrame(std::get<WriteAudioFrame_args>(cmd));
    }
}

void AVBufferedEncoder::Private::ClearCommandQueue()
{
    std::unique_lock<std::mutex> lock(mCommandQueueMutex);
    while (!mCommandQueue.empty()) {
        const auto& cmd = mCommandQueue.front();
        if (std::holds_alternative<WriteVideoFrame_args>(cmd)) {
            delete[] static_cast<uint8_t*>(std::get<WriteVideoFrame_args>(cmd).pData);
        }
        else if (std::holds_alternative<WriteAudioFrame_args>(cmd)) {
            delete[] static_cast<uint8_t*>(std::get<WriteAudioFrame_args>(cmd).pData);
        }
        mCommandQueue.pop();
    }
}

void AVBufferedEncoder::Private::ThreadFunc()
{
    while (mCommandAvailable.Wait()) {
        std::unique_lock<std::mutex> lock(mCommandQueueMutex);
        while (!mCommandQueue.empty()) {
            auto cmd = mCommandQueue.front();
            mCommandQueue.pop();
            lock.unlock();
            DispatchCommand(cmd);
            lock.lock();
        }
    }
}

// Main AVBufferedEncoder class
AVBufferedEncoder::AVBufferedEncoder()
: p(new Private)
{
}

AVBufferedEncoder::~AVBufferedEncoder()
{
    delete p;
}

AVBufferedEncoder& AVBufferedEncoder::OpenFile(const std::string& file)
{
    Private::OpenFile_args args = { file };
    std::unique_lock<std::mutex> lock(p->mCommandQueueMutex);
    p->mCommandQueue.push(args);
    p->mCommandAvailable.Set();
    return *this;
}

AVBufferedEncoder &AVBufferedEncoder::BeginWriting()
{
    Private::BeginWriting_args args;
    std::unique_lock<std::mutex> lock(p->mCommandQueueMutex);
    p->mCommandQueue.push(args);
    p->mCommandAvailable.Set();
    return *this;
}

AVBufferedEncoder& AVBufferedEncoder::InitializeVideoStream(int width, int height, const Rate& fps, int codec, int preset, int compressionFactor, const AVEncoder::Options& options)
{
    Private::InitializeVideoStream_args args = { width, height, fps, codec, preset, compressionFactor, options };
    std::unique_lock<std::mutex> lock(p->mCommandQueueMutex);
    p->mCommandQueue.push(args);
    p->mCommandAvailable.Set();
    return *this;
}

AVBufferedEncoder& AVBufferedEncoder::WriteVideoFrame(const void* pData, int bytes, Time::Interval timestamp)
{
    Private::WriteVideoFrame_args args = { nullptr, bytes, timestamp };
    args.pData = new uint8_t[bytes];
    ::memcpy(args.pData, pData, bytes);

    std::unique_lock<std::mutex> lock(p->mCommandQueueMutex);
    p->mCommandQueue.push(args);
    p->mCommandAvailable.Set();
    return *this;
}

AVBufferedEncoder& AVBufferedEncoder::InitializeAudioStream(int sampleblock, int channels, const Rate& sps, int codec, const AVEncoder::Options& options)
{
    Private::InitializeAudioStream_args args = { sampleblock, channels, sps, codec, options };
    std::unique_lock<std::mutex> lock(p->mCommandQueueMutex);
    p->mCommandQueue.push(args);
    p->mCommandAvailable.Set();
    return *this;
}

AVBufferedEncoder& AVBufferedEncoder::WriteAudioFrame(const void* pData, int bytes, Time::Interval timestamp)
{
    Private::WriteAudioFrame_args args = { nullptr, bytes, timestamp };
    args.pData = new uint8_t[bytes];
    ::memcpy(args.pData, pData, bytes);

    std::unique_lock<std::mutex> lock(p->mCommandQueueMutex);
    p->mCommandQueue.push(args);
    p->mCommandAvailable.Set();
    return *this;
}

AVBufferedEncoder& AVBufferedEncoder::CloseFile()
{
    Private::CloseFile_args args;
    std::unique_lock<std::mutex> lock(p->mCommandQueueMutex);
    p->mCommandQueue.push(args);
    p->mCommandAvailable.Set();
    return *this;
}

std::string AVBufferedEncoder::Error() const
{
    std::lock_guard<std::mutex> lock(p->mErrorMutex);
    return p->mError;
}
