//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: A class for bidirectional streaming of AV data in
//   real time.
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
#ifndef AV_MUXER_H
#define AV_MUXER_H

#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#include "Sockets.h"

class AVMuxer
{
public:
    struct DataBuffer
    {
        std::string kind;
        std::vector<char> payload;
    };

    struct Stream {
        Stream() {}
        const AVCodec* mpCodec = nullptr;
        AVCodecContext* mpCodecContext = nullptr;
        AVFrame* mpFrame = nullptr;
        AVCodecParameters* mpCodecParameters = nullptr;
        int mIndex = -1;
        enum { isNull, isVideo, isAudio } kind = isNull;
        union {
            struct {
                SwsContext* mpScaleContext = nullptr;
                int width = 0, height = 0;
            } video;
            struct {
                SwrContext* mpResampleContext = nullptr;
                int samplingrate = 0, channels = 0;
            } audio;
        };
    };

    static const std::string RequestConfig;
    static const std::string InvalidateConfig;
    static const std::string ConfigData;
    static const std::string AVPacketData;

    explicit AVMuxer(Socket&);

    bool ReceiveData(DataBuffer&);
    bool SendAVPacket(AVPacket*, int64_t packetCount);
    bool SendConfigRequest();
    bool SendInvalidateConfig();
    bool SendConfig(const std::vector<AVMuxer::Stream>&);

    static void EncodeConfigData(const std::vector<Stream>&, std::vector<char>&);
    static void ApplyConfigData(const std::vector<char>&, std::vector<Stream>&);

private:
    bool SendData(const DataBuffer&);

    static void ConfigureVideoStream(Stream& stream, const AVCodecParameters*);
    static void ConfigureAudioStream(Stream& stream, const AVCodecParameters*);
    static void CleanupStream(Stream& stream);

    static std::ostream& SerializeAVPacketSideData(const AVPacketSideData*, std::ostream&);
    static std::istream& UnserializeAVPacketSideData(AVPacketSideData*, std::istream&);
    static std::ostream& SerializeAVPacket(const AVPacket*, std::ostream&);
    static std::istream& UnserializeAVPacket(AVPacket**, std::istream&);
    static std::ostream& SerializeAVCodecParameters(const AVCodecParameters*, std::ostream&);
    static std::istream& UnserializeAVCodecParameters(AVCodecParameters*, std::istream&);

    void PackAVPacket(const AVPacket*, int64_t count, std::vector<char>&);

public:
    AVPacket* UnpackAVPacket(const std::vector<char>&, int64_t& count);

private:
    Socket& mSocket;
};

#endif // AV_MUXER_H
