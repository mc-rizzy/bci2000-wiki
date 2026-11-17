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
#include "AVBidirectionalChannel.h"

#include "Sockets.h"
#include "Thread.h"
#include "Runnable.h"
#include "BinaryData.h"
#include "AVError.h"
#include "AVLog.h"
#include "Debugging.h"
#include "BCIStream.h"
#include "AVMuxer.h"

// Some codecs insist on configuring thread properties
// in a way that is incompatible with other codecs
// (COM initialization). We use a worker thread to solve this.
// This problem is probably restricted
// to Windows, so we don't use a worker thread on other platforms.
#if _WIN32
# include "ReusableThread.h"
# include "Runnable.h"
# define USE_WORKER_THREAD 1
#endif

#include <sstream>
#include <vector>
#include <set>
#include <mutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}

static const char* h264_mf_presetNames[] = {
    "auto", "baseline", "main", "high"
};
static const char* libx264_presetNames[] = {
    "medium", "fast", "medium", "slow"
};
static const int numPresetNames = 4;

static const char* PresetName(const AVCodec* pCodec, int preset)
{
    if (preset >= numPresetNames)
        preset = numPresetNames - 1;
    if (preset < 0)
        preset = 0;
    if (pCodec->id == AV_CODEC_ID_H264) {
        if (!::stricmp(pCodec->name, "h264_mf")) { // Windows Media Foundation
            return h264_mf_presetNames[preset];
        }
        if (!::stricmp(pCodec->name, "libx264")) {
            return libx264_presetNames[preset];
        }
        return nullptr;
    }
    return nullptr;
}

static void PopulateOptions(const AVCodec* pCodec, AVDictionary** pOptions)
{
    if (!::stricmp(pCodec->name, "h264_mf")) {
        ::av_dict_set(pOptions, "delay", "0", 0);
        ::av_dict_set(pOptions, "scenario", "2", 0); // scenario 2 is video conferencing
    }
    else if (!::stricmp(pCodec->name, "libx264")) {
        ::av_dict_set(pOptions, "delay", "0", 0);
        ::av_dict_set(pOptions, "tune", "zerolatency", 0);
    }
}

struct AVBidirectionalChannel::Private
{
    AVPacket* mpPacket = nullptr;

    std::vector<AVMuxer::Stream> mSendingStreams, mReceivingStreams;

    std::string mAddress;
    ClientTCPSocket mSocket;
    AVMuxer mAVMuxer;
    std::ostringstream mError;
    std::mutex mMutex;

    std::atomic<int64_t> mReceivedVideoPacket = 0, mSentVideoPacket = 0;
    std::atomic<int64_t> mReceivedAudioFrame = 0, mSentAudioFrame = 0;

    std::atomic<bool> mWriting = false;

    std::set<RenderClient*> mRemoteRenderClients, mLocalRenderClients;

    Thread mReceivingThread;
    MemberCall<void(Private*)> mReceivingThreadCall;
#if USE_WORKER_THREAD
    WorkerThread mWorkerThread;
#endif

    Private()
        : mReceivingThreadCall(&Private::ReceivingThread, this),
        mReceivingThread(&mReceivingThreadCall, "AVBidirectionalChannel::ReceivingThread"),
        mAVMuxer(mSocket)
    {
    }

    ~Private()
    {
        Close();
    }

    void ReceivingThread()
    {
        AVMuxer::DataBuffer buf;
        bool configValid = false;
        while (!mReceivingThread.Terminating()) {
            while (!mReceivingThread.Terminating() && !configValid) {
                mAVMuxer.SendConfigRequest();
                if (mSocket.Wait(Time::Seconds(1)) && mAVMuxer.ReceiveData(buf)) {
                    if (buf.kind == AVMuxer::RequestConfig && mWriting) {
                        mAVMuxer.SendConfig(mSendingStreams);
                    }
                    else if (buf.kind == AVMuxer::ConfigData) {
                        mAVMuxer.ApplyConfigData(buf.payload, mReceivingStreams);
                        configValid = true;
                    }
                }
            }
            while (!mReceivingThread.Terminating() && configValid && mAVMuxer.ReceiveData(buf)) {
                if (buf.kind == AVMuxer::RequestConfig && mWriting) {
                    mAVMuxer.SendConfig(mSendingStreams);
                }
                else if (buf.kind == AVMuxer::InvalidateConfig) {
                    for (auto& stream : mReceivingStreams)
                        CloseStream(stream);
                    mReceivingStreams.clear();
                    configValid = false;
                }
                else if (buf.kind == AVMuxer::AVPacketData) {
                    int64_t packetCount = 0;
                    AVPacket* pPacket = mAVMuxer.UnpackAVPacket(buf.payload, packetCount);
                    if (pPacket->stream_index < mReceivingStreams.size()) {
                        AVMuxer::Stream& stream = GetReceivingStream(pPacket->stream_index);
                        switch (stream.kind) {
                        case AVMuxer::Stream::isAudio:
                            DecodeAudioPacket(stream, pPacket);
                            break;
                        case AVMuxer::Stream::isVideo:
                            DecodeVideoPacket(stream, pPacket);
                            mReceivedVideoPacket = packetCount;
                            break;
                        default:
                            ;
                        }
                    }
                    ::av_packet_free(&pPacket);
                }
            }
        }
    }

    void Open(const std::string& address)
    {
        Close();
        mAddress = address;
        mSocket.Open(address);
        if (!mSocket.IsOpen()) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Could not open address " << address << std::endl;
            return;
        }
        mReceivingThread.Start();

        mpPacket = ::av_packet_alloc();
    }

    void Close()
    {
        mReceivingThread.Terminate();
        mWriting = false;
        ::av_packet_free(&mpPacket);
        for (auto& stream : mSendingStreams)
            CloseStream(stream);
        mSendingStreams.clear();
    }

    void BeginWriting()
    {
        for (const auto& stream : mSendingStreams) {
            if (!stream.mpCodecContext) {
                std::lock_guard<std::mutex> lock(mMutex);
                mError << "BeginWriting() called with uninitialized streams";
                return;
            }
        }
        mAVMuxer.SendInvalidateConfig();
        mWriting = true;
    }

    void AddVideoStream(int* outStreamId, std::string encoder)
    {
        AVMuxer::Stream stream;
        if (encoder.empty()) {
            stream.mpCodec = ::avcodec_find_encoder(AV_CODEC_ID_H264);
        }
        else {
            stream.mpCodec = ::avcodec_find_encoder_by_name(encoder.c_str());
        }
        if (!stream.mpCodec) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Cannot find " << (encoder.empty() ? "H.264" : encoder) << " video encoder" << std::endl;
            mError << "Encoders are:\n";

            const AVCodec* pCodec = nullptr;
            void* i = 0;
            while ((pCodec = ::av_codec_iterate(&i))) {
                if (::av_codec_is_encoder(pCodec) && pCodec->type == AVMEDIA_TYPE_VIDEO) {
                    mError << " " << pCodec->name << " (" << pCodec->long_name << ")\n";
                }
            }
            return;
        }
        bciout << "Video encoder is " << stream.mpCodec->name << " (" << stream.mpCodec->long_name << ")";
        *outStreamId = mSendingStreams.size();
        stream.mIndex = *outStreamId;
        mSendingStreams.push_back(stream);
    }

    void AddAudioStream(int* outStreamId, std::string encoder)
    {
        AVMuxer::Stream stream;
        if (encoder.empty()) {
            stream.mpCodec = ::avcodec_find_encoder(AV_CODEC_ID_AAC);
        }
        else {
            stream.mpCodec = ::avcodec_find_encoder_by_name(encoder.c_str());
        }
        if (!stream.mpCodec) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Cannot find " << (encoder.empty() ? "AAC" : encoder) << " audio encoder" << std::endl;
            return;
        }
        bciout << "Audio encoder is " << stream.mpCodec->name << " (" << stream.mpCodec->long_name << ")";
        *outStreamId = mSendingStreams.size();
        stream.mIndex = *outStreamId;
        mSendingStreams.push_back(stream);
    }

    void CloseStream(AVMuxer::Stream& stream)
    {
        switch (stream.kind) {
            case AVMuxer::Stream::isVideo:
                ::sws_freeContext(stream.video.mpScaleContext);
                stream.video.mpScaleContext = nullptr;
                break;
            case AVMuxer::Stream::isAudio:
                ::swr_free(&stream.audio.mpResampleContext);
                break;
            default:
                ;
        }
        if (stream.mpCodecContext)
            ::avcodec_free_context(&stream.mpCodecContext);
        ::av_frame_free(&stream.mpFrame);
    }

    AVMuxer::Stream& GetSendingStream(int idx)
    {
        if (idx < 0 || idx >= mSendingStreams.size())
            throw std_runtime_error << "Invalid stream index: " << idx;
        return mSendingStreams[idx];
    }

    AVMuxer::Stream& GetReceivingStream(int idx)
    {
        if (idx < 0 || idx >= mReceivingStreams.size())
            throw std_runtime_error << "Invalid stream index: " << idx;
        return mReceivingStreams[idx];
    }

    void InitializeVideoStream(int idx, int width, int height, Rate fps, int preset, int compressionFactor)
    {
        AVMuxer::Stream& stream = GetSendingStream(idx);
        AVCodecContext* pCtx = ::avcodec_alloc_context3(stream.mpCodec);
        stream.mpCodecContext = pCtx;
        pCtx->thread_count = 0; // automatically choose number of threads
        pCtx->thread_type = 0; // default type
        pCtx->gop_size = 10;
        pCtx->max_b_frames = 1;
        pCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
        Ratio r = (fps * Time::OneSecond()).Simplified(1e-6);
        int n = r.Numerator(), d = r.Denominator();
        pCtx->framerate = AVRational{ n, d };
        pCtx->time_base = AVRational{ d, n * 10 }; // allow for small variations in framerate

        // Determine maximum bit rate
        pCtx->bit_rate = width * height * 2 /* YUV420 */ * sizeof(uint8_t) * r.To<double>();
        // Divide by compression factor
        pCtx->bit_rate /= compressionFactor;
        // Allow actual bit rate to be between 0 and twice the estimated compressed bit rate
        pCtx->bit_rate_tolerance = pCtx->bit_rate;

        pCtx->pix_fmt = AV_PIX_FMT_YUV420P;
#if 0
        pCtx->codec_id = AV_CODEC_ID_H264;
        pCtx->codec_type = AVMEDIA_TYPE_VIDEO;
#endif
        pCtx->width = width;
        pCtx->height = height;

        const char* pPreset = PresetName(stream.mpCodec, preset);
        if (pPreset)
            ::av_opt_set(pCtx->priv_data, "preset", pPreset, 0);

        AVDictionary* pOptions = nullptr;
        PopulateOptions(stream.mpCodec, &pOptions);
        int result = ::avcodec_open2(pCtx, stream.mpCodec, &pOptions);
        if (::av_dict_count(pOptions) > 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Unknown options:\n";
            for (const AVDictionaryEntry* pEntry = ::av_dict_iterate(pOptions, nullptr); 
                pEntry; 
                pEntry = ::av_dict_iterate(pOptions, pEntry)) {
                mError << pEntry->key << "\n";
            }
            return;
        }
        ::av_dict_free(&pOptions);
        if (result < 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Could not open codec: " << AVError(result).Message();
            return;
        }

        stream.mpFrame = ::av_frame_alloc();
        stream.mpFrame->format = pCtx->pix_fmt;
        stream.mpFrame->width = pCtx->width;
        stream.mpFrame->height = pCtx->height;
        result = ::av_frame_get_buffer(stream.mpFrame, 0);
        if (result < 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Could not get buffer: " << AVError(result).Message();
            return;
        }

        ::avcodec_parameters_free(&stream.mpCodecParameters);
        stream.mpCodecParameters = ::avcodec_parameters_alloc();
        result = ::avcodec_parameters_from_context(stream.mpCodecParameters, pCtx);
        if (result < 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Could not copy parameters from context: " << AVError(result).Message();
            return;
        }

        stream.video.mpScaleContext = ::sws_getContext(
            width, height, AV_PIX_FMT_BGR0,
            width, height, AV_PIX_FMT_YUV420P,
            0, nullptr, nullptr, nullptr
        );

        stream.kind = AVMuxer::Stream::isVideo;
    }

    void InitializeAudioStream(int idx, int blocksize, int channels, Rate fps)
    {
        AVMuxer::Stream& stream = GetSendingStream(idx);
        AVCodecContext* pCtx = ::avcodec_alloc_context3(stream.mpCodec);
        stream.mpCodecContext = pCtx;
        pCtx->thread_count = 0; // automatically choose number of threads
        pCtx->thread_type = 0; // default type
        Ratio r = (fps * Time::OneSecond()).Simplified(1e-6);
        int n = r.Numerator(), d = r.Denominator();
        pCtx->framerate = AVRational{ n, d };
        pCtx->time_base = AVRational{ d, n * 10 }; // allow for small variations in framerate

        pCtx->codec_type = AVMEDIA_TYPE_AUDIO;
        pCtx->codec_id = stream.mpCodec->id;
        pCtx->sample_rate = fps.Hertz();
        ::av_channel_layout_default(&pCtx->ch_layout, channels);
        pCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
        pCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        
        int result = ::avcodec_open2(pCtx, stream.mpCodec, nullptr);
        if (result < 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Could not open audio codec: " << AVError(result).Message();
            return;
        }

        stream.mpFrame = ::av_frame_alloc();
        stream.mpFrame->format = pCtx->sample_fmt;
        stream.mpFrame->ch_layout = pCtx->ch_layout;
        stream.mpFrame->nb_samples = blocksize;
        result = ::av_frame_get_buffer(stream.mpFrame, 0);
        if (result < 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Could not get buffer: " << AVError(result).Message();
            return;
        }

        ::avcodec_parameters_free(&stream.mpCodecParameters);
        stream.mpCodecParameters = ::avcodec_parameters_alloc();
        result = ::avcodec_parameters_from_context(stream.mpCodecParameters, pCtx);
        if (result < 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "Could not copy parameters from context: " << AVError(result).Message();
            return;
        }

        SwrContext* pSwrCtx = nullptr;
        ::swr_alloc_set_opts2(&pSwrCtx,
            &pCtx->ch_layout, pCtx->sample_fmt, pCtx->sample_rate,
            &pCtx->ch_layout, AV_SAMPLE_FMT_FLT, pCtx->sample_rate,
            0, nullptr
        );
        ::swr_init(pSwrCtx);
        stream.audio.mpResampleContext = pSwrCtx;

        stream.kind = AVMuxer::Stream::isAudio;
    }

    void WriteFrame(int streamIndex, const void* pData, Time::Interval timestamp)
    {
        auto& stream = GetSendingStream(streamIndex);
        switch (stream.kind) {
            case AVMuxer::Stream::isVideo:
                SetVideoFrame(stream, static_cast<const uint32_t*>(pData));
                break;
            case AVMuxer::Stream::isAudio:
                SetAudioFrame(stream, static_cast<const float*>(pData));
                break;
            default:
                ;
        }
        WriteFrame(stream, timestamp);
    }

    int64_t TimeIntervalToPts(AVMuxer::Stream& stream, Time::Interval interval)
    {
        AVRational tb = stream.mpCodecContext->time_base;
        Ratio r(tb.num, tb.den);
        return ((interval / Time::OneSecond()) / r).Round();
    }

    void SetVideoFrame(AVMuxer::Stream& stream, const uint32_t* pRGBData)
    {
        int rgblinesize = sizeof(*pRGBData) * stream.mpFrame->width;
        union { const uint32_t* p3; const uint8_t* p8; } rgbdata = { pRGBData };
        ::sws_scale(stream.video.mpScaleContext,
            &rgbdata.p8, &rgblinesize, 0, stream.mpFrame->height,
            stream.mpFrame->data, stream.mpFrame->linesize
        );
    }

    void SetAudioFrame(AVMuxer::Stream& stream, const float* pAudioData)
    {
        union { const float* f; const uint8_t* p8; } floatdata = { pAudioData };
        ::swr_convert(stream.audio.mpResampleContext,
            stream.mpFrame->data, stream.mpFrame->nb_samples,
            &floatdata.p8, stream.mpFrame->nb_samples
        );
    }

    void WriteFrame(AVMuxer::Stream& stream, Time::Interval timestamp)
    {
        int64_t prevPts = stream.mpFrame->pts;
        stream.mpFrame->pts = TimeIntervalToPts(stream, timestamp);
        if (stream.mpFrame->pts <= prevPts) {
            return;
        }
        int result = ::avcodec_send_frame(stream.mpCodecContext, stream.mpFrame);
        if (result < 0) {
            std::lock_guard<std::mutex> lock(mMutex);
            mError << "avcodec_send_frame() failed: " << AVError(result).Message() << std::endl;
            return;
        }

        while (result >= 0)
        {
            result = ::avcodec_receive_packet(stream.mpCodecContext, mpPacket);
            if (result < 0 && result != AVERROR(EAGAIN)) {
                std::lock_guard<std::mutex> lock(mMutex);
                mError << "avcodec_receive_packet() failed: " << AVError(result).Message() << std::endl;
                return;
            }
            if (result >= 0)
            {
                mpPacket->stream_index = stream.mIndex;
                int64_t frameCount = 0;
                switch (stream.kind) {
                    case AVMuxer::Stream::isVideo:
                        frameCount = mSentVideoPacket;
                        break;
                    case AVMuxer::Stream::isAudio:
                        frameCount = mSentAudioFrame;
                        break;
                    default:
                        ;
                }
                if (!mAVMuxer.SendAVPacket(mpPacket, frameCount)) {
                    std::lock_guard<std::mutex> lock(mMutex);
                    mError << "Could not serialize packet" << std::endl;
                    return;
                }
            }
        }
    }

    void FinalizeStream(int idx)
    {
        auto& stream = GetSendingStream(idx);
        if (stream.mpCodecContext && stream.mpFrame && stream.mpFrame->pts >= 0)
        {
            // Send a null frame to indicate End-Of-Stream.
            int result = ::avcodec_send_frame(stream.mpCodecContext, nullptr);
            if (result < 0) {
                std::lock_guard<std::mutex> lock(mMutex);
                mError << "avcodec_send_frame() failed: " << AVError(result).Message() << std::endl;
                return;
            }
            while (result >= 0)
            {
                result = ::avcodec_receive_packet(stream.mpCodecContext, mpPacket);
                if (result < 0 && result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
                    std::lock_guard<std::mutex> lock(mMutex);
                    mError << "avcodec_receive_packet() failed: " << AVError(result).Message() << std::endl;
                    return;
                }
                Assert(result != AVERROR(EAGAIN));
                if (result >= 0)
                {
                    mpPacket->stream_index = stream.mIndex;
                    int64_t packetCount = 0;
                    if (stream.kind == AVMuxer::Stream::isVideo) {
                        packetCount = mSentVideoPacket++;
                    }
                    if (!mAVMuxer.SendAVPacket(mpPacket, packetCount)) {
                        std::lock_guard<std::mutex> lock(mMutex);
                        mError << "Could not serialize packet" << std::endl;
                        return;
                    }
                }
            }
        }
    }

    void DecodeAudioPacket(AVMuxer::Stream& stream, AVPacket* pPacket)
    {
        int ret = ::avcodec_send_packet(stream.mpCodecContext, pPacket);
        if (ret < 0) {
            throw std_runtime_error << "Error sending audio packet to decoder: " << AVError(ret).Message();
            return;
        }

        while (ret >= 0) {
            AVFrame* pFrame = ::av_frame_alloc();
            ret = ::avcodec_receive_frame(stream.mpCodecContext, pFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                ::av_frame_free(&pFrame);
                throw std_runtime_error << "Error receiving audio frame from decoder: " << AVError(ret).Message();
            }
            std::unique_lock<std::mutex> lock(mMutex);
            if (!mRemoteRenderClients.empty()) {
                Assert(stream.audio.mpResampleContext);
                AVFrame* pFrame2 = ::av_frame_alloc();
                pFrame2->nb_samples = pFrame->nb_samples;
                pFrame2->format = AV_SAMPLE_FMT_FLT;
                pFrame2->ch_layout = pFrame->ch_layout;
                pFrame2->sample_rate = pFrame->sample_rate;
                ::av_frame_get_buffer(pFrame2, 1);
                ::swr_convert_frame(stream.audio.mpResampleContext, pFrame2, pFrame);
                for (auto pRenderClient : mRemoteRenderClients)
                    pRenderClient->OnAudioFrame(stream.mIndex, pFrame2->sample_rate, pFrame2->nb_samples, stream.audio.channels, reinterpret_cast<float*>(pFrame2->data[0]), mReceivedAudioFrame);
                mReceivedAudioFrame += pFrame2->nb_samples;
                ::av_frame_free(&pFrame2);
            }
            lock.unlock();
            ::av_frame_free(&pFrame);
        }
    }

    void DecodeVideoPacket(AVMuxer::Stream& stream, AVPacket* pPacket)
    {
        int ret = 0;
        {
            AVLog::Silence silence;
            ret = ::avcodec_send_packet(stream.mpCodecContext, pPacket);
        }
        if (ret < 0 && ret != AVERROR_INVALIDDATA /* missing P frame */) {
            throw std_runtime_error << "Error sending video packet to decoder: " << AVError(ret).Message();
        }

        while (ret >= 0) {
            AVFrame* pFrame = ::av_frame_alloc();
            ret = ::avcodec_receive_frame(stream.mpCodecContext, pFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                ::av_frame_free(&pFrame);
                break;
            }
            if (ret < 0) {
                ::av_frame_free(&pFrame);
                throw std_runtime_error << "Error receiving video frame from decoder: " << AVError(ret).Message();
            }

            std::unique_lock<std::mutex> lock(mMutex);
            if (!mRemoteRenderClients.empty()) {
                Assert(stream.video.mpScaleContext);
                AVFrame* pFrame2 = ::av_frame_alloc();
                ::sws_scale_frame(stream.video.mpScaleContext, pFrame2, pFrame);
                for (auto pRenderClient : mRemoteRenderClients)
                    pRenderClient->OnVideoFrame(stream.mIndex, stream.video.width, stream.video.height, pFrame2->data[0], mReceivedVideoPacket);
                ::av_frame_free(&pFrame2);
            }
            lock.unlock();
            ::av_frame_free(&pFrame);
        }
    }
};

AVBidirectionalChannel::AVBidirectionalChannel()
: p(new Private)
{
    AVLog::AddClient();
}

AVBidirectionalChannel::~AVBidirectionalChannel()
{
    AVLog::RemoveClient();
    delete p;
}

AVBidirectionalChannel& AVBidirectionalChannel::AddRenderClient(Position pos, RenderClient* pClient)
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    switch (pos) {
    case local:
        p->mLocalRenderClients.insert(pClient);
        break;
    case remote:
        p->mRemoteRenderClients.insert(pClient);
        break;
    }
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::RemoveRenderClient(Position pos, RenderClient* pClient)
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    switch (pos) {
    case local:
        p->mLocalRenderClients.erase(pClient);
        break;
    case remote:
        p->mRemoteRenderClients.erase(pClient);
        break;
    }
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::Open(const std::string& address)
{
#if USE_WORKER_THREAD
    MemberCall<void(Private*, const std::string&)> call(&Private::Open, p, address);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->Open(address);
#endif
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::AddVideoStream(int* pStreamIndex, const std::string& encoder)
{
#if USE_WORKER_THREAD
    MemberCall<void(Private*, int*, std::string)> call(&Private::AddVideoStream, p, pStreamIndex, encoder);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->AddVideoStream(pStreamIndex, encoder);
#endif
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::InitializeVideoStream(int streamIndex, int width, int height, const Rate& fps, int preset, int compressionFactor)
{
    auto& stream = p->GetSendingStream(streamIndex);
    stream.kind = stream.isVideo;
    stream.video.width = width;
    stream.video.height = height;
#if USE_WORKER_THREAD
    MemberCall<void(Private*, int, int, int, Rate, int, int)> 
        call(&Private::InitializeVideoStream, p, streamIndex, width, height, fps, preset, compressionFactor);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->InitializeVideoStream(streamIndex, width, height, fps, preset, compressionFactor);
#endif
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::AddAudioStream(int* pStreamIndex, const std::string& encoder)
{
#if USE_WORKER_THREAD
    MemberCall<void(Private*, int*, std::string)> call(&Private::AddAudioStream, p, pStreamIndex, encoder);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->AddAudioStream(pStreamIndex, encoder);
#endif
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::InitializeAudioStream(int streamIndex, int sampleblock, int channels, const Rate& sps)
{
    auto& stream = p->GetSendingStream(streamIndex);
    stream.kind = stream.isAudio;
    stream.audio.channels = channels;
    stream.audio.samplingrate = sps.Hertz();
#if USE_WORKER_THREAD
    MemberCall<void(Private*, int, int, int, Rate)>
        call(&Private::InitializeAudioStream, p, streamIndex, sampleblock, channels, sps);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->InitializeAudioStream(streamIndex, sampleblock, channels, sps);
#endif
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::BeginWriting()
{
#if USE_WORKER_THREAD
    MemberCall<void(Private*)> call(&Private::BeginWriting, p);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->BeginWriting();
#endif
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::WriteFrame(int streamIndex, const void* pData, int bytes, Time::Interval timestamp)
{
    std::unique_lock<std::mutex> lock(p->mMutex);
    auto& stream = p->GetSendingStream(streamIndex);
    if (stream.kind == stream.isAudio) {
        int samples = bytes / stream.audio.channels / sizeof(float);
        for (auto pRenderer : p->mLocalRenderClients)
            pRenderer->OnAudioFrame(streamIndex, stream.audio.samplingrate, samples, stream.audio.channels, static_cast<const float*>(pData), p->mSentAudioFrame);
        p->mSentAudioFrame += samples;
    }
    else if (stream.kind == stream.isVideo) {
        for (auto pRenderer : p->mLocalRenderClients)
            pRenderer->OnVideoFrame(streamIndex, stream.video.width, stream.video.height, static_cast<const uint8_t*>(pData), p->mSentVideoPacket);
        p->mSentVideoPacket += 1;
    }
    lock.unlock();
#if USE_WORKER_THREAD
    MemberCall<void(Private*, int, const void*, Time::Interval)> 
        call(&Private::WriteFrame, p, streamIndex, pData, timestamp);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->WriteFrame(streamIndex, pData, timestamp);
#endif
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::FinalizeStream(int streamIndex)
{
#if USE_WORKER_THREAD
    MemberCall<void(Private*, int)> call(&Private::FinalizeStream, p, streamIndex);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->FinalizeStream(streamIndex);
#endif
    return *this;
}

AVBidirectionalChannel& AVBidirectionalChannel::Close()
{
#if USE_WORKER_THREAD
    MemberCall<void(Private*)> call(&Private::Close, p);
    p->mWorkerThread.RunSynchronously(call);
#else
    p->Close();
#endif
    return *this;
}

std::string AVBidirectionalChannel::Error() const
{
    std::lock_guard<std::mutex> lock(p->mMutex);
    return p->mError.str();
}
