////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: A base class for saving audio or video or both to a file.
////////////////////////////////////////////////////////////////////////////////
#ifndef AVMEETING_SAVE_TO_FILE_H
#define AVMEETING_SAVE_TO_FILE_H

#include "AVBidirectionalChannel.h"
#include "AVBufferedEncoder.h"

#include <string>
#include <mutex>

class AVMeetingSaveToFile : public AVBidirectionalChannel::RenderClient
{
public:
    AVMeetingSaveToFile();
    virtual ~AVMeetingSaveToFile();

    void SetSamplingRate(const Rate&);
    Rate SamplingRate() const;

    // Prefix for bcievent recording frame number.
    // Will be appended with "Video" resp. "Audio" and "Frame" resp. "TotalFrames".
    // E.g., if prefix is "Local" and video is recorded, the following two bcievents
    // will be generated:
    //   LocalVideoFrame
    //   LocalVideoTotalFrames
    void SetEventPrefixFrame(const std::string&);
    std::string EventPrefixFrame() const;

    void StartRecording(const std::string&);
    void StopRecording();
    void IncrementFilePart();

    std::string Error() const;

protected:
    void OnVideoFrame(int streamIdx, int width, int height, const uint8_t* pData, int64_t frame) override;
    void OnAudioFrame(int streamIdx, int samplingrate, int samples, int channels, const float* pData, int64_t frame) override;

    virtual bool OnOpenFile(AVBufferedEncoder& encoder, const std::string& name) = 0;
    virtual void OnFrameData(AVBufferedEncoder& encoder, const void* data, int bytes, Time::Interval) = 0;

    int Width() const { return mWidth; }
    int Height() const { return mHeight; }
    int SampleBlockSize() const { return mSampleBlockSize; }
    int Channels() const { return mChannels; }

private:
    void HandleFrameData(const uint8_t* data, int size, int64_t frame);

    mutable std::recursive_mutex mMutex;

    int mFilePart = 0;
    std::string mFileBaseName, mEventPrefixFrame;

    enum { closed, open, broken } mFileState = closed;

    int64_t mFirstFrame = 0;
    class Rate mSamplingRate;

    int mWidth = 0, mHeight = 0;
    int mSampleBlockSize = 0, mChannels = 0;

    AVBufferedEncoder mEncoder;
};

#endif // AVMEETING_SAVE_TO_FILE_H
