////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
////////////////////////////////////////////////////////////////////////////////
#include "ReceivingAVDisplay.h"

#include "BCIStream.h"
#include "GraphDisplay.h"
#include "AudioSink.h"
#include "Runnable.h"
#include "Clock.h"
#include "TimeUtils.h"
#include "Debugging.h"

#include <mutex>
#include <vector>
#include <QImage>
#include <QPainter>

extern "C" {
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
}

static const Time::Interval cVideoTimeout = Time::Seconds(1);

struct ReceivingAVDisplay::Private
{
    Private(ReceivingAVDisplay*, GUI::GraphDisplay&);
    ~Private();

    ReceivingAVDisplay* mpSelf;

    // video
    void AdaptToVideoInput(int width, int height);
    void AdaptToVideoOutput(int width, int height);
    void AdjustVideoOutput();
    void AdjustVideoScalingContext();
    void ProcessVideoInput(const uint8_t*);
    void OnVideoTimeoutTick();

    GUI::GraphDisplay& mrDisplay;
    std::mutex mVideoMutex;
    std::vector<uint8_t> mVideoOutput;
    SwsContext* mpScalingContext = nullptr;
    int mInputWidth = 0, mInputHeight = 0, mOutputWidth = 0, mOutputHeight = 0, mOutputObjectWidth = 0, mOutputObjectHeight = 0;
    Time mLastVideoInput = Time::Never();
    Clock mVideoTimeoutClock;
    std::string mBackgroundText;

    MemberCall<void(Private*)> mOnVideoTimeoutTick;

    // audio
    void AdaptToAudioInput(int samplingrate, int channels, AVSampleFormat);
    void AdjustAudioResamplingContext();
    void ProcessAudioInput(int samples, const float*);
    void OnAudioSinkTick();

    std::mutex mAudioMutex;
    SwrContext* mpResamplingContext = nullptr;
    AudioSinkRef mpAudioSink;
    AudioSink::StreamRef mpStream;
    uint32_t mUserId = 0;
    float mAudioVolume = 1.0;
    float mAudioBufferSeconds = 40e-3;
    // limit compensation resampling to 1/24 of a semitone
    const float mAudioResamplingMaxCompensation = 0.25e-2;
    std::vector<float> mAudioOutput;
    int mAudioInputRate = 0, mAudioInputChannels = 0;
    AVSampleFormat mAudioInputFormat = AV_SAMPLE_FMT_FLT;
    int mAudioOutputRate = 0;
    const int mAudioOutputChannels = 2;

    MemberCall<void(Private*)> mOnAudioSinkTick;
};

ReceivingAVDisplay::Private::Private(ReceivingAVDisplay* pSelf, GUI::GraphDisplay& display)
:   mpSelf(pSelf), mrDisplay(display), 
    mOnAudioSinkTick(&ReceivingAVDisplay::Private::OnAudioSinkTick, this),
    mOnVideoTimeoutTick(&ReceivingAVDisplay::Private::OnVideoTimeoutTick, this)
{
    mVideoTimeoutClock.AddListener(Clock::OnTick, &mOnVideoTimeoutTick);
    mVideoTimeoutClock.SetInterval(cVideoTimeout);
    mVideoTimeoutClock.Start();
}

ReceivingAVDisplay::Private::~Private()
{
    mVideoTimeoutClock.Stop();

    std::unique_lock<std::mutex> audioLock(mAudioMutex);
    if (mpAudioSink)
        mpAudioSink->RemoveListener(AudioSink::OnTick, &mOnAudioSinkTick);
    audioLock.unlock();

    std::unique_lock<std::mutex> videoLock(mVideoMutex);
    if (mpScalingContext)
        ::sws_freeContext(mpScalingContext);
}

void ReceivingAVDisplay::Private::AdaptToVideoInput(int width, int height)
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    if (mInputWidth != width || mInputHeight != height) {
        mInputWidth = width;
        mInputHeight = height;
        AdjustVideoOutput();
        AdjustVideoScalingContext();
    }
}

void ReceivingAVDisplay::Private::AdjustVideoOutput()
{
    if (mInputWidth == 0 || mInputHeight == 0) {
        mOutputWidth = 0;
        mOutputHeight = 0;
        return;
    }
    if (mOutputObjectWidth * mInputHeight <= mOutputObjectHeight * mInputWidth) {
        mOutputHeight = mOutputObjectWidth * mInputHeight / mInputWidth;
        mOutputWidth = mOutputObjectWidth;
    }
    else {
        mOutputWidth = mOutputObjectHeight * mInputWidth / mInputHeight;
        mOutputHeight = mOutputObjectHeight;
    }
}

void ReceivingAVDisplay::Private::AdaptToVideoOutput(int width, int height)
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    if (mOutputObjectWidth != width || mOutputObjectHeight != height) {
        mOutputObjectWidth = width;
        mOutputObjectHeight = height;
        AdjustVideoOutput();
        AdjustVideoScalingContext();
    }
}

void ReceivingAVDisplay::Private::AdjustVideoScalingContext()
{
    ::sws_freeContext(mpScalingContext);
    mpScalingContext = nullptr;
    if (mInputWidth > 0 && mInputHeight > 0 && mOutputWidth > 0 && mOutputHeight > 0) {
        mpScalingContext = ::sws_getContext(
            mInputWidth, mInputHeight, AVPixelFormat::AV_PIX_FMT_RGB32,
            mOutputWidth, mOutputHeight, AVPixelFormat::AV_PIX_FMT_RGB32,
            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
        );
    }
    size_t size = mOutputWidth * mOutputHeight * 4;
    mVideoOutput.resize(size, 0);
}

void ReceivingAVDisplay::Private::ProcessVideoInput(const uint8_t* pInputData)
{
    std::unique_lock<std::mutex> lock(mVideoMutex);

    if (mpScalingContext) {
        int inputBytesPerLine = mInputWidth * 4;
        int outputBytesPerLine = mVideoOutput.size() / mOutputHeight;
        uint8_t* pOutputdata = mVideoOutput.data();
        ::sws_scale(mpScalingContext, &pInputData, &inputBytesPerLine, 0, mInputHeight, &pOutputdata, &outputBytesPerLine);
    }
}

void ReceivingAVDisplay::Private::OnVideoTimeoutTick()
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    auto now = TimeUtils::MonotonicTime();
    if (now - mLastVideoInput >= cVideoTimeout)
        mpSelf->Invalidate();
}

void ReceivingAVDisplay::Private::AdaptToAudioInput(int samplingrate, int channels, AVSampleFormat format)
{
    if (mAudioInputRate != samplingrate || mAudioInputChannels != channels || mAudioInputFormat != format) {
        mAudioInputRate = samplingrate;
        mAudioInputChannels = channels;
        mAudioInputFormat = format;
        AdjustAudioResamplingContext();
    }
}

void ReceivingAVDisplay::Private::AdjustAudioResamplingContext()
{
    mpAudioSink = *mrDisplay.AssociatedAudio();
    if (!mpAudioSink)
        throw bcierr << "No audio sink is associated with the current display";
    mpAudioSink->AddListener(AudioSink::OnTick, &mOnAudioSinkTick);
    mpStream = mpAudioSink->Stream();

    std::unique_lock<std::mutex> lock(mAudioMutex);

    if (mpResamplingContext) {
        ::swr_free(&mpResamplingContext);
        mpResamplingContext = nullptr;
    }

    AVChannelLayout outChLayout = AV_CHANNEL_LAYOUT_STEREO,
                    inChLayout = AV_CHANNEL_LAYOUT_STEREO;
    if (this->mAudioInputChannels == 1)
        inChLayout = AV_CHANNEL_LAYOUT_MONO;
    else if (this->mAudioInputChannels != 2)
        throw bcierr << "Unsupported number of audio channels from inpt stream: " << this->mAudioInputChannels;

    mAudioOutputRate = mpStream->SampleRate().Hertz();
    if (mpStream->PlaybackChannels() != 2)
        throw bcierr << "Unsupported number of audio sink playback channels: " << mpStream->PlaybackChannels();

    ::swr_alloc_set_opts2(&mpResamplingContext,
        &outChLayout, AV_SAMPLE_FMT_FLT, mAudioOutputRate,
        &inChLayout, mAudioInputFormat, mAudioInputRate,
        0, nullptr
    );
    int err = ::swr_init(mpResamplingContext);
    if (err) {
        char buf[AV_ERROR_MAX_STRING_SIZE + 1];
        ::av_strerror(err, buf, sizeof(buf) - 1);
        bcierr << "swr_init(): " << buf;
    }
}

void ReceivingAVDisplay::Private::ProcessAudioInput(int samples, const float* pData)
{
    std::unique_lock<std::mutex> lock(mAudioMutex);
    // We use libswresample's internal buffer for intermediate storage, so we specify 0 for either input or output.
    union { const float* f; const uint8_t* i; } pInput = { pData };
    int err = ::swr_convert(mpResamplingContext, nullptr, 0, &pInput.i, samples);
    if (err < 0) {
        char buf[AV_ERROR_MAX_STRING_SIZE + 1];
        ::av_strerror(err, buf, sizeof(buf) - 1);
        bcierr << "swr_convert(): " << buf;
    }
}

void ReceivingAVDisplay::Private::OnAudioSinkTick()
{
    std::unique_lock<std::mutex> lock(mAudioMutex);
    if (mAudioInputRate <= 0)
        return;

    int actualSamplesBuffered = ::swr_get_out_samples(mpResamplingContext, 0);
    int targetSamplesBuffered = mAudioBufferSeconds * mAudioOutputRate;
    int dupOrDrop = targetSamplesBuffered - actualSamplesBuffered;

    auto buf = mpAudioSink->MixBuffer();
    int bufFrames = buf.RemainingFrames();

    if (bufFrames - dupOrDrop < 1)
        dupOrDrop = 0;

    int maxdiff = ::floor(bufFrames * mAudioResamplingMaxCompensation);
    if (maxdiff < 1)
        maxdiff = 1;
    if (dupOrDrop < -maxdiff)
        dupOrDrop = -maxdiff;
    if (dupOrDrop > maxdiff)
        dupOrDrop = maxdiff;

    int actualFrames = bufFrames - dupOrDrop;
    if (mAudioOutput.size() < actualFrames * mAudioOutputChannels)
        mAudioOutput.resize(actualFrames * mAudioOutputChannels, 0);
    uint8_t* pOutput = reinterpret_cast<uint8_t*>(mAudioOutput.data());
    int outputLen = actualFrames;

    // We use libswresample's internal buffer for intermediate storage, so we don't specify an input.
    int count = ::swr_convert(mpResamplingContext, &pOutput, outputLen, nullptr, 0);
    if (count < 0) {
        char buf[AV_ERROR_MAX_STRING_SIZE + 1];
        ::av_strerror(count, buf, sizeof(buf) - 1);
        bcierr << "swr_convert(): " << buf;
    }
    lock.unlock();

    if (count == actualFrames) {
        buf.Put(mAudioOutput.data(), actualFrames, dupOrDrop, mAudioVolume);
    }
}

ReceivingAVDisplay::ReceivingAVDisplay(GUI::GraphDisplay& display)
: GraphObject(display, AVStimulusZOrder), p(new Private(this, display))
{
}

ReceivingAVDisplay::~ReceivingAVDisplay()
{
    delete p;
}

void ReceivingAVDisplay::SetAudioVolume(float volume)
{
    p->mAudioVolume = volume;
}

float ReceivingAVDisplay::AudioVolume() const
{
    return p->mAudioVolume;
}

void ReceivingAVDisplay::SetAudioBufferSeconds(float duration)
{
    p->mAudioBufferSeconds = duration;
}

float ReceivingAVDisplay::AudioBufferSeconds() const
{
    return p->mAudioBufferSeconds;
}

void ReceivingAVDisplay::SetBackgroundText(const std::string& s)
{
    p->mBackgroundText = s;
}

const std::string& ReceivingAVDisplay::BackgroundText() const
{
    return p->mBackgroundText;
}

void ReceivingAVDisplay::OnPaint(const GUI::DrawContext& dc)
{
    QPainter* pPainter = dc.handle.dc;
    QRect rect(dc.rect.left, dc.rect.top, dc.rect.Width(), dc.rect.Height());
    pPainter->fillRect(rect, Qt::black);
    auto now = TimeUtils::MonotonicTime();
    if (now - p->mLastVideoInput > cVideoTimeout) {
        QString text = QString::fromUtf8(p->mBackgroundText.c_str());
        auto font = pPainter->font();
        font.setBold(true);
        float width = QFontMetricsF(font).horizontalAdvance(text);
        font.setPointSizeF(0.8 * dc.rect.Width() * font.pointSizeF() / width);
        pPainter->setFont(font);
        pPainter->setPen(Qt::white);
        pPainter->drawText(rect, Qt::AlignCenter, text);
    }
    else {
        std::unique_lock<std::mutex> lock(p->mVideoMutex);
        QImage image(p->mVideoOutput.data(), p->mOutputWidth, p->mOutputHeight, QImage::Format_RGB32);
        int left = dc.rect.left + (dc.rect.Width() - p->mOutputWidth) / 2;
        int top = dc.rect.top + (dc.rect.Height() - p->mOutputHeight) / 2;
        pPainter->drawImage(left, top, image);
    }
}

void ReceivingAVDisplay::OnResize(GUI::DrawContext& dc)
{
    p->AdaptToVideoOutput(dc.rect.Width(), dc.rect.Height());
}

void ReceivingAVDisplay::OnVideoFrame(int, int width, int height, const uint8_t* pData, int64_t frame)
{
    p->AdaptToVideoInput(width, height);
    p->ProcessVideoInput(pData);
    p->mLastVideoInput = TimeUtils::MonotonicTime();
    Invalidate();
}

void ReceivingAVDisplay::OnAudioFrame(int, int samplingrate, int samples, int channels, const float* pData, int64_t frame)
{
    p->AdaptToAudioInput(samplingrate, channels, AV_SAMPLE_FMT_FLT);
    p->ProcessAudioInput(samples, pData);
}


