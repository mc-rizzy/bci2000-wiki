//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: A simple muxer for AV data.
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
#include "AVMuxer.h"

#include "AVError.h"
#include "BinaryData.h"
#if _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

const std::string AVMuxer::RequestConfig = "RCfg";
const std::string AVMuxer::InvalidateConfig = "ICfg";
const std::string AVMuxer::ConfigData = "CfgD";
const std::string AVMuxer::AVPacketData = "AVPk";

AVMuxer::AVMuxer(Socket& s)
: mSocket(s)
{
}

std::ostream& AVMuxer::SerializeAVPacketSideData(const AVPacketSideData* pSideData, std::ostream& os)
{
    BinaryData<int, BigEndian>(pSideData->type).Put(os);
    BinaryData<int, BigEndian>(pSideData->size).Put(os);
    os.write(reinterpret_cast<const char*>(pSideData->data), pSideData->size);
    return os;
}

std::istream& AVMuxer::UnserializeAVPacketSideData(AVPacketSideData* pSideData, std::istream& is)
{
    pSideData->type = AVPacketSideDataType(int(BinaryData<int, BigEndian>(is)));
    pSideData->size = BinaryData<int, BigEndian>(is);
    pSideData->data = static_cast<uint8_t*>(::av_malloc(pSideData->size));
    is.read(reinterpret_cast<char*>(pSideData->data), pSideData->size);
    return is;
}

std::ostream& AVMuxer::SerializeAVPacket(const AVPacket* pPacket, std::ostream& os)
{
    BinaryData<int, BigEndian>(pPacket->stream_index).Put(os);
    BinaryData<int, BigEndian>(pPacket->size).Put(os);
    os.write(reinterpret_cast<const char*>(pPacket->data), pPacket->size);
    BinaryData<int, BigEndian>(pPacket->flags).Put(os);
    BinaryData<int, BigEndian>(pPacket->side_data_elems).Put(os);
    for (int i = 0; i < pPacket->side_data_elems; ++i) {
        SerializeAVPacketSideData(pPacket->side_data + i, os);
    }
    return os;
}

std::istream& AVMuxer::UnserializeAVPacket(AVPacket** pPacket, std::istream& is)
{
    int index = BinaryData<int, BigEndian>(is);
    int size = BinaryData<int, BigEndian>(is);
    *pPacket = ::av_packet_alloc();
    ::av_new_packet(*pPacket, size);
    (*pPacket)->stream_index = index;
    is.read(reinterpret_cast<char*>((*pPacket)->data), size);
    (*pPacket)->flags = BinaryData<int, BigEndian>(is);
    int elems = BinaryData<int, BigEndian>(is);
    for (int i = 0; i < elems; ++i) {
        AVPacketSideData data;
        UnserializeAVPacketSideData(&data, is);
        ::av_packet_add_side_data(*pPacket, data.type, data.data, data.size);
    }
    return is;
}

std::ostream& AVMuxer::SerializeAVCodecParameters(const AVCodecParameters* pParam, std::ostream& os)
{
    BinaryData<int, BigEndian>(pParam->codec_type).Put(os);
    BinaryData<int, BigEndian>(pParam->codec_id).Put(os);
    BinaryData<int, BigEndian>(pParam->codec_tag).Put(os);

    BinaryData<int, BigEndian>(pParam->extradata_size).Put(os);
    os.write(reinterpret_cast<const char*>(pParam->extradata), pParam->extradata_size);

    BinaryData<int, BigEndian>(pParam->nb_coded_side_data).Put(os);
    for (int i = 0; i < pParam->nb_coded_side_data; ++i) {
        const AVPacketSideData* pData = pParam->coded_side_data + i;
        SerializeAVPacketSideData(pData, os);
    }

    BinaryData<int, BigEndian>(pParam->format).Put(os);
    BinaryData<int64_t, BigEndian>(pParam->bit_rate).Put(os);
    BinaryData<int, BigEndian>(pParam->bits_per_coded_sample).Put(os);
    BinaryData<int, BigEndian>(pParam->bits_per_raw_sample).Put(os);
    BinaryData<int, BigEndian>(pParam->profile).Put(os);
    BinaryData<int, BigEndian>(pParam->level).Put(os);
    BinaryData<int, BigEndian>(pParam->width).Put(os);
    BinaryData<int, BigEndian>(pParam->height).Put(os);
    BinaryData<int, BigEndian>(pParam->sample_aspect_ratio.num).Put(os);
    BinaryData<int, BigEndian>(pParam->sample_aspect_ratio.den).Put(os);
    BinaryData<int, BigEndian>(pParam->framerate.num).Put(os);
    BinaryData<int, BigEndian>(pParam->framerate.den).Put(os);
    BinaryData<int, BigEndian>(pParam->field_order).Put(os);
    BinaryData<int, BigEndian>(pParam->color_range).Put(os);
    BinaryData<int, BigEndian>(pParam->color_primaries).Put(os);
    BinaryData<int, BigEndian>(pParam->color_trc).Put(os);
    BinaryData<int, BigEndian>(pParam->color_space).Put(os);
    BinaryData<int, BigEndian>(pParam->chroma_location).Put(os);
    BinaryData<int, BigEndian>(pParam->video_delay).Put(os);
    BinaryData<int, BigEndian>(pParam->ch_layout.nb_channels).Put(os);
    BinaryData<int, BigEndian>(pParam->ch_layout.order).Put(os);
    BinaryData<uint64_t, BigEndian>(pParam->ch_layout.u.mask).Put(os);
    BinaryData<int, BigEndian>(pParam->sample_rate).Put(os);
    BinaryData<int, BigEndian>(pParam->block_align).Put(os);
    BinaryData<int, BigEndian>(pParam->frame_size).Put(os);
    BinaryData<int, BigEndian>(pParam->initial_padding).Put(os);
    BinaryData<int, BigEndian>(pParam->trailing_padding).Put(os);
    BinaryData<int, BigEndian>(pParam->seek_preroll).Put(os);
    return os;
}

std::istream& AVMuxer::UnserializeAVCodecParameters(AVCodecParameters* pParam, std::istream& is)
{
    pParam->codec_type = AVMediaType(int(BinaryData<int, BigEndian>(is)));
    pParam->codec_id = AVCodecID(int(BinaryData<int, BigEndian>(is)));
    pParam->codec_tag = BinaryData<int, BigEndian>(is);

    pParam->extradata_size = BinaryData<int, BigEndian>(is);
    pParam->extradata = nullptr;
    if (pParam->extradata_size) {
        pParam->extradata = static_cast<uint8_t*>(::av_malloc(pParam->extradata_size));
        is.read(reinterpret_cast<char*>(pParam->extradata), pParam->extradata_size);
    }

    pParam->nb_coded_side_data = BinaryData<int, BigEndian>(is);
    AVPacketSideData* pList = nullptr;
    if (pParam->nb_coded_side_data) {
        pList = reinterpret_cast<AVPacketSideData*>(::av_malloc(sizeof(*pList) * pParam->nb_coded_side_data));
        for (int i = 0; i < pParam->nb_coded_side_data; ++i) {
            AVPacketSideData* pData = pList + i;
            UnserializeAVPacketSideData(pData, is);
        }
    }
    pParam->coded_side_data = pList;

    pParam->format = BinaryData<int, BigEndian>(is);
    pParam->bit_rate = BinaryData<int64_t, BigEndian>(is);
    pParam->bits_per_coded_sample = BinaryData<int, BigEndian>(is);
    pParam->bits_per_raw_sample = BinaryData<int, BigEndian>(is);
    pParam->profile = BinaryData<int, BigEndian>(is);
    pParam->level = BinaryData<int, BigEndian>(is);
    pParam->width = BinaryData<int, BigEndian>(is);
    pParam->height = BinaryData<int, BigEndian>(is);
    pParam->sample_aspect_ratio.num = BinaryData<int, BigEndian>(is);
    pParam->sample_aspect_ratio.den = BinaryData<int, BigEndian>(is);
    pParam->framerate.num = BinaryData<int, BigEndian>(is);
    pParam->framerate.den = BinaryData<int, BigEndian>(is);
    pParam->field_order = AVFieldOrder(int(BinaryData<int, BigEndian>(is)));
    pParam->color_range = AVColorRange(int(BinaryData<int, BigEndian>(is)));
    pParam->color_primaries = AVColorPrimaries(int(BinaryData<int, BigEndian>(is)));
    pParam->color_trc = AVColorTransferCharacteristic(int(BinaryData<int, BigEndian>(is)));
    pParam->color_space = AVColorSpace(int(BinaryData<int, BigEndian>(is)));
    pParam->chroma_location = AVChromaLocation(int(BinaryData<int, BigEndian>(is)));
    pParam->video_delay = BinaryData<int, BigEndian>(is);
    pParam->ch_layout.nb_channels = BinaryData<int, BigEndian>(is);
    pParam->ch_layout.order = AVChannelOrder(int(BinaryData<int, BigEndian>(is)));
    pParam->ch_layout.u.mask = BinaryData<uint64_t, BigEndian>(is);
    pParam->sample_rate = BinaryData<int, BigEndian>(is);
    pParam->block_align = BinaryData<int, BigEndian>(is);
    pParam->frame_size = BinaryData<int, BigEndian>(is);
    pParam->initial_padding = BinaryData<int, BigEndian>(is);
    pParam->trailing_padding = BinaryData<int, BigEndian>(is);
    pParam->seek_preroll = BinaryData<int, BigEndian>(is);
    return is;
}

bool AVMuxer::SendData(const DataBuffer& buf)
{
    uint32_t size = buf.kind.length() + 1;
    size += buf.payload.size();
    size = htonl(size);
    std::unique_lock<Socket> lock(mSocket);
    int count = mSocket.Write(&size, sizeof(size));
    count += mSocket.Write(buf.kind.c_str(), buf.kind.length() + 1);
    count += mSocket.Write(buf.payload.data(), buf.payload.size());
    lock.unlock();
    size = ntohl(size);
    return count == size + sizeof(size);
}

bool AVMuxer::ReceiveData(DataBuffer& buf)
{
    if (!mSocket.Wait()) {
        return false;
    }
    uint32_t size = 0;
    int count = mSocket.Read(&size, sizeof(size));
    if (count != sizeof(size))
        return false;
    size = ntohl(size);
    buf.kind.clear();
    count = 0;
    char c = 0;
    do {
        count += mSocket.Read(&c, 1);
        if (c != 0)
            buf.kind += c;
    } while (c != 0 && count <= size);
    if (c != 0)
        return false;

    uint32_t payloadSize = size - count;
    if (payloadSize > 0) {
        if (!mSocket.Wait()) {
            return false;
        }
        buf.payload.resize(payloadSize);
        count = mSocket.Read(buf.payload.data(), buf.payload.size());
        if (count != payloadSize) {
            return false;
        }
    }
    return true;
}

bool AVMuxer::SendAVPacket(AVPacket* pPacket, int64_t packetCount)
{
    DataBuffer buf;
    buf.kind = AVPacketData;
    PackAVPacket(pPacket, packetCount, buf.payload);
    return SendData(buf);
}

bool AVMuxer::SendConfigRequest()
{
    DataBuffer buf;
    buf.kind = RequestConfig;
    return SendData(buf);
}

bool AVMuxer::SendInvalidateConfig()
{
    DataBuffer buf;
    buf.kind = InvalidateConfig;
    return SendData(buf);
}

bool AVMuxer::SendConfig(const std::vector<AVMuxer::Stream>& streams)
{
    AVMuxer::DataBuffer buf;
    buf.kind = ConfigData;
    AVMuxer::EncodeConfigData(streams, buf.payload);
    return SendData(buf);
}
void AVMuxer::PackAVPacket(const AVPacket* pPacket, int64_t packetCount, std::vector<char>& buf)
{
    std::ostringstream oss;
    BinaryData<int64_t, BigEndian>(packetCount).Put(oss);
    AVMuxer::SerializeAVPacket(pPacket, oss);
    std::string data = oss.str();
    buf.resize(data.length());
    std::memcpy(buf.data(), data.data(), data.length());
}

AVPacket* AVMuxer::UnpackAVPacket(const std::vector<char>& buf, int64_t& packetCount)
{
    std::string data(buf.data(), buf.size());
    std::istringstream iss(data);
    packetCount = BinaryData<int64_t, BigEndian>(iss);
    AVPacket* pPacket = nullptr;
    AVMuxer::UnserializeAVPacket(&pPacket, iss);
    return pPacket;
}

void AVMuxer::EncodeConfigData(const std::vector<Stream>& streams, std::vector<char>& buf)
{
    std::ostringstream oss;
    for (const auto& stream : streams) {
        AVMuxer::SerializeAVCodecParameters(stream.mpCodecParameters, oss);
    }
    std::string data = oss.str();
    buf.resize(data.length());
    std::memcpy(buf.data(), data.data(), data.length());
}

void AVMuxer::ApplyConfigData(const std::vector<char>& buf, std::vector<AVMuxer::Stream>& streams)
{
    std::string data(buf.data(), buf.size());
    std::istringstream iss(data);
    for (auto& stream : streams) {
        CleanupStream(stream);
    }
    streams.clear();
    AVCodecParameters* pCodecParameters = ::avcodec_parameters_alloc();
    while (AVMuxer::UnserializeAVCodecParameters(pCodecParameters, iss)) {
        AVMuxer::Stream stream;
        stream.mIndex = streams.size();
        stream.mpCodecParameters = pCodecParameters;
        switch (pCodecParameters->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
            ConfigureVideoStream(stream, pCodecParameters);
            break;
        case AVMEDIA_TYPE_AUDIO:
            ConfigureAudioStream(stream, pCodecParameters);
            break;
        default:
            ;
        }
        streams.push_back(stream);
        pCodecParameters = ::avcodec_parameters_alloc();
    }
    ::avcodec_parameters_free(&pCodecParameters);
}

void AVMuxer::ConfigureVideoStream(Stream& stream, const AVCodecParameters* pParams)
{
    stream.kind = Stream::isVideo;
    stream.video.height = pParams->height;
    stream.video.width = pParams->width;
    stream.mpCodec = ::avcodec_find_decoder(pParams->codec_id);
    stream.mpCodecContext = ::avcodec_alloc_context3(stream.mpCodec);
    ::avcodec_parameters_to_context(stream.mpCodecContext, pParams);
    ::avcodec_open2(stream.mpCodecContext, stream.mpCodec, nullptr);

    SwsContext* pContext = ::sws_getContext(
        pParams->width, pParams->height, AVPixelFormat(pParams->format),
        pParams->width, pParams->height, AVPixelFormat::AV_PIX_FMT_RGB32,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
    );
    if (!pContext) {
        throw std_runtime_error << "Could not initialize scaling context";
    }
    stream.video.mpScaleContext = pContext;
}

void AVMuxer::ConfigureAudioStream(Stream& stream, const AVCodecParameters* pParams)
{
    stream.kind = Stream::isAudio;
    stream.audio.samplingrate = pParams->sample_rate;
    stream.audio.channels = pParams->ch_layout.nb_channels;
    stream.mpCodec = ::avcodec_find_decoder(pParams->codec_id);
    stream.mpCodecContext = ::avcodec_alloc_context3(stream.mpCodec);
    ::avcodec_parameters_to_context(stream.mpCodecContext, pParams);
    ::avcodec_open2(stream.mpCodecContext, stream.mpCodec, nullptr);

    SwrContext* pContext = nullptr;
    int err = ::swr_alloc_set_opts2(&pContext,
        &pParams->ch_layout, AV_SAMPLE_FMT_FLT, pParams->sample_rate,
        &pParams->ch_layout, AVSampleFormat(pParams->format), pParams->sample_rate,
        0, nullptr
    );
    if (err) {
        throw std_runtime_error << "Could not allocate swr context: " << AVError(err).Message();
    }
    err = ::swr_init(pContext);
    if (err) {
        throw std_runtime_error << "Could not initialize swr context: " << AVError(err).Message();
    }
    stream.audio.mpResampleContext = pContext;
}

void AVMuxer::CleanupStream(Stream& stream)
{
    ::avcodec_free_context(&stream.mpCodecContext);
    if (stream.kind == Stream::isVideo) {
        ::sws_freeContext(stream.video.mpScaleContext);
    }
    else if (stream.kind == Stream::isAudio) {
        ::swr_free(&stream.audio.mpResampleContext);
    }
}
