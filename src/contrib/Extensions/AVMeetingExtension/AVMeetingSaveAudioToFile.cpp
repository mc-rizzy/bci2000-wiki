////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: AVMeetingSaveAudioToFile implementation
////////////////////////////////////////////////////////////////////////////////
#include "AVMeetingSaveAudioToFile.h"

AVMeetingSaveAudioToFile::AVMeetingSaveAudioToFile()
{
    SetSamplingRate(Rate::FromHertz(44100));
}

bool AVMeetingSaveAudioToFile::OnOpenFile(AVBufferedEncoder& encoder, const std::string& filename)
{
    if (SampleBlockSize() * Channels() <= 0)
        return false;

    encoder.OpenFile(filename + ".aac");
    if (!encoder.Error().empty())
        return false;
    encoder.InitializeAudioStream(SampleBlockSize(), Channels(), SamplingRate(), AVEncoder::aac);
    if (!encoder.Error().empty())
        return false;
    encoder.BeginWriting();
    if (!encoder.Error().empty())
        return false;
    return true;
}

void AVMeetingSaveAudioToFile::OnFrameData(AVBufferedEncoder &encoder, const void * pData, int bytes, Time::Interval ts)
{
    encoder.WriteAudioFrame(pData, bytes, ts);
}

