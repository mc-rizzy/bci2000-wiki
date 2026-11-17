////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: AVMeetingSaveVideoToFile implementation
////////////////////////////////////////////////////////////////////////////////
#include "AVMeetingSaveVideoToFile.h"

AVMeetingSaveVideoToFile::AVMeetingSaveVideoToFile()
{
    SetSamplingRate(Rate::FromHertz(15));
}

bool AVMeetingSaveVideoToFile::OnOpenFile(AVBufferedEncoder& encoder, const std::string& filename)
{
    if (Width() * Height() <= 0)
        return false;

    encoder.OpenFile(filename + ".mp4");
    if (!encoder.Error().empty())
        return false;
    encoder.InitializeVideoStream(Width(), Height(), SamplingRate(), AVEncoder::h264);
    if (!encoder.Error().empty())
        return false;
    encoder.BeginWriting();
    if (!encoder.Error().empty())
        return false;
    return true;
}

void AVMeetingSaveVideoToFile::OnFrameData(AVBufferedEncoder &encoder, const void * pData, int bytes, Time::Interval ts)
{
    encoder.WriteVideoFrame(pData, bytes, ts);
}

