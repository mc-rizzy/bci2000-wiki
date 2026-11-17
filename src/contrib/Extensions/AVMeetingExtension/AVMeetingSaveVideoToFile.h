////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: AVMeetingSaveVideoToFile declaration
////////////////////////////////////////////////////////////////////////////////
#ifndef AVMEETING_SAVE_VIDEO_TO_FILE_H
#define AVMEETING_SAVE_VIDEO_TO_FILE_H

#include "AVMeetingSaveToFile.h"

class AVMeetingSaveVideoToFile : public AVMeetingSaveToFile
{
public:
    AVMeetingSaveVideoToFile();
    ~AVMeetingSaveVideoToFile() = default;

protected:
    void OnAudioFrame(int streamIdx, int samplingrate, int samples, int channels, const float* pData, int64_t frame) override {}
    bool OnOpenFile(AVBufferedEncoder& encoder, const std::string& name) override;
    void OnFrameData(AVBufferedEncoder&, const void*, int, Time::Interval) override;
};

#endif // AVMEETING_SAVE_VIDEO_TO_FILE_H
