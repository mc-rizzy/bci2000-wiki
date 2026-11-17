////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
////////////////////////////////////////////////////////////////////////////////
#include "MirrorDisplay.h"

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

struct MirrorDisplay::Private
{
    Private(MirrorDisplay*, GUI::GraphDisplay&);
    ~Private();

    MirrorDisplay* mpSelf;

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
};

MirrorDisplay::Private::Private(MirrorDisplay* pSelf, GUI::GraphDisplay& display)
:   mpSelf(pSelf), mrDisplay(display), 
    mOnVideoTimeoutTick(&MirrorDisplay::Private::OnVideoTimeoutTick, this)
{
    mVideoTimeoutClock.AddListener(Clock::OnTick, &mOnVideoTimeoutTick);
    mVideoTimeoutClock.SetInterval(cVideoTimeout);
    mVideoTimeoutClock.Start();
}

MirrorDisplay::Private::~Private()
{
    mVideoTimeoutClock.Stop();

    std::unique_lock<std::mutex> videoLock(mVideoMutex);
    if (mpScalingContext)
        ::sws_freeContext(mpScalingContext);
}

void MirrorDisplay::Private::AdaptToVideoInput(int width, int height)
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    if (mInputWidth != width || mInputHeight != height) {
        mInputWidth = width;
        mInputHeight = height;
        AdjustVideoOutput();
        AdjustVideoScalingContext();
    }
}

void MirrorDisplay::Private::AdjustVideoOutput()
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

void MirrorDisplay::Private::AdaptToVideoOutput(int width, int height)
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    if (mOutputObjectWidth != width || mOutputObjectHeight != height) {
        mOutputObjectWidth = width;
        mOutputObjectHeight = height;
        AdjustVideoOutput();
        AdjustVideoScalingContext();
    }
}

void MirrorDisplay::Private::AdjustVideoScalingContext()
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

void MirrorDisplay::Private::ProcessVideoInput(const uint8_t* pData)
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    if (mpScalingContext) {
        int inputBytesPerLine = mInputWidth * 4;
        int outputBytesPerLine = mVideoOutput.size() / mOutputHeight;
        uint8_t* pOutputData = mVideoOutput.data();
        ::sws_scale(mpScalingContext, &pData, &inputBytesPerLine, 0, mInputHeight, &pOutputData, &outputBytesPerLine);
    }
}

void MirrorDisplay::Private::OnVideoTimeoutTick()
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    auto now = TimeUtils::MonotonicTime();
    if (now - mLastVideoInput >= cVideoTimeout)
        mpSelf->Invalidate();
}

MirrorDisplay::MirrorDisplay(GUI::GraphDisplay& display)
: GraphObject(display, AVStimulusZOrder), p(new Private(this, display))
{
}

MirrorDisplay::~MirrorDisplay()
{
    delete p;
}

void MirrorDisplay::SetBackgroundText(const std::string& s)
{
    p->mBackgroundText = s;
}

const std::string& MirrorDisplay::BackgroundText() const
{
    return p->mBackgroundText;
}

void MirrorDisplay::OnPaint(const GUI::DrawContext& dc)
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
        for (int i = 0; i < p->mOutputWidth * p->mOutputHeight; ++i) {
            uint8_t* q = p->mVideoOutput.data() + 4 * i;
            q[3] = 0xff; // QImage RGB32 needs BGRx byte set to 0xff (why?)
        }
        QImage image(p->mVideoOutput.data(), p->mOutputWidth, p->mOutputHeight, QImage::Format_RGB32);
        int left = dc.rect.left + (dc.rect.Width() - p->mOutputWidth) / 2;
        int top = dc.rect.top + (dc.rect.Height() - p->mOutputHeight) / 2;
        pPainter->drawImage(left, top, image.mirrored(true, false));
    }
}

void MirrorDisplay::OnResize(GUI::DrawContext& dc)
{
    p->AdaptToVideoOutput(dc.rect.Width(), dc.rect.Height());
}

void MirrorDisplay::OnVideoFrame(int, int width, int height, const uint8_t* pData, int64_t)
{
    p->AdaptToVideoInput(width, height);
    p->ProcessVideoInput(pData);
    p->mLastVideoInput = TimeUtils::MonotonicTime();
    Invalidate();
}

