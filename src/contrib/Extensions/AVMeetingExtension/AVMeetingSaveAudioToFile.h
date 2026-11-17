////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: AVMeetingSaveAudioToFile declaration
////////////////////////////////////////////////////////////////////////////////
#ifndef AVMEETING_SAVE_AUDIO_TO_FILE_H
#define AVMEETING_SAVE_AUDIO_TO_FILE_H

#include "AVMeetingSaveToFile.h"

class AVMeetingSaveAudioToFile : public AVMeetingSaveToFile
{
public:
    AVMeetingSaveAudioToFile();
    ~AVMeetingSaveAudioToFile() = default;

protected:
    void OnVideoFrame(int streamIdx, int width, int height, const uint8_t* pData, int64_t frame) override {}
    bool OnOpenFile(AVBufferedEncoder& encoder, const std::string& name) override;
    void OnFrameData(AVBufferedEncoder& encoder, const void*, int, Time::Interval) override;
};

#endif // AVMEETING_SAVE_AUDIO_TO_FILE_H
