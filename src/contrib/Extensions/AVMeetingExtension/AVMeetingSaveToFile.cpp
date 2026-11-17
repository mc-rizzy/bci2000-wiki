////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: A base class for saving audio or video or both to a file.
////////////////////////////////////////////////////////////////////////////////
#include "AVMeetingSaveToFile.h"
#include "BCIEvent.h"
#include "BCIStream.h"

AVMeetingSaveToFile::AVMeetingSaveToFile()
{
}

AVMeetingSaveToFile::~AVMeetingSaveToFile()
{
}

void AVMeetingSaveToFile::SetSamplingRate(const Rate& rate)
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    mSamplingRate = rate;
}

Rate AVMeetingSaveToFile::SamplingRate() const
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    return mSamplingRate;
}

void AVMeetingSaveToFile::SetEventPrefixFrame(const std::string& name)
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    mEventPrefixFrame = name;
}

std::string AVMeetingSaveToFile::EventPrefixFrame() const
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    return mEventPrefixFrame;
}

void AVMeetingSaveToFile::StartRecording(const std::string& name)
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    StopRecording();
    mFileBaseName = name;
    mFilePart = 1;
    if (mFileState == open)
        mFileState = closed;
}

void AVMeetingSaveToFile::StopRecording()
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    if (mFileState == open) {
        mFileState = broken;
        if (!mEncoder.Error().empty())
            return;
        mEncoder.CloseFile();
        if (!mEncoder.Error().empty())
            return;
        mFileBaseName.clear();
        mFileState = closed;
    }
}

void AVMeetingSaveToFile::IncrementFilePart()
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    StopRecording();
    ++mFilePart;
}

void AVMeetingSaveToFile::HandleFrameData(const uint8_t* pData, int size, int64_t frame)
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    if (mFileState == closed && !mFileBaseName.empty()) {
        mFileState = broken;
        std::string name = mFileBaseName;
        if (mFilePart > 1) {
            name += "_" + std::to_string(mFilePart);
        }
        if (OnOpenFile(mEncoder, name)) {
            mFirstFrame = frame;
            mFileState = open;
        }
    }
    if (mEncoder.Error().empty()) {

        int64_t frameInFile = frame - mFirstFrame;
        if (!mEventPrefixFrame.empty()) {
            lock.unlock();
            bcievent << mEventPrefixFrame << "Frame " << (frameInFile & 0xffffffff); // restrict frame number to 32 bit
            bcievent << mEventPrefixFrame << "TotalFrames " << (frame & 0xffffffff);
            lock.lock();
        }
        OnFrameData(mEncoder, pData, size, frameInFile / mSamplingRate);
    }
    if (mFileState == open && !mEncoder.Error().empty()) {
        mFileState = broken;
    }
    if (!mEncoder.Error().empty()) {
        bcierr << mEncoder.Error();
    }
}

void AVMeetingSaveToFile::OnVideoFrame(int, int width, int height, const uint8_t* pData, int64_t frame)
{
    mWidth = width;
    mHeight = height;
    HandleFrameData(pData, width * height * 4, frame);
}

void AVMeetingSaveToFile::OnAudioFrame(int, int samplingrate, int samples, int channels, const float* pData, int64_t frame)
{
    mSamplingRate = Rate::FromHertz(samplingrate);
    mSampleBlockSize = samples;
    mChannels = channels;
    HandleFrameData(reinterpret_cast<const uint8_t*>(pData), samples * channels * sizeof(float), frame);
}

std::string AVMeetingSaveToFile::Error() const
{
    std::unique_lock<std::recursive_mutex> lock(mMutex);
    return mEncoder.Error();
}
