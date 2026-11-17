////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: AVMeetingFilter implementation
////////////////////////////////////////////////////////////////////////////////
#include "AVMeetingFilter.h"
#include "BCIStream.h"
#include "AVBidirectionalChannel.h"
#include "Thread.h"
#include "AVSource.h"
#include "AVSourceEnumerator.h"
#include "AudioDevice.h"
#include "EnvVariable.h"
#include "Sockets.h"
#include "ReusableThread.h"
#include "Runnable.h"
#include "FileUtils.h"
#include "RunManager.h"
#include "ThreadUtils.h"

#if _WIN32
#include "AVSourceEnumeratorMF.h"
#else
#include "AVSourceEnumeratorLibAVDevice.h"
#endif

#include "ReceivingAVDisplay.h"
#include "MirrorDisplay.h"
#include "AVMeetingSaveVideoToFile.h"
#include "AVMeetingSaveAudioToFile.h"
#include "Expression.h"

#include <string>

static const int kDefaultCameraFrameDurationMs = 32;
static const int kAudioSamplingRateHz = 44100;
static const int kAudioBufferSize = 1024;
static const int kServerTimeoutMs = 1000;

static GUI::Rect RectFromParam(const ParamRef& param)
{
    float left = param(0), top = param(1), right = param(2), bottom = param(3);
    return GUI::Rect { left, top, right, bottom };
}

RegisterFilter(AVMeetingFilter, 3.001);

struct AVMeetingFilter::Private
{
    bool mEnabled = false;

    ReceivingAVDisplay* mpReceivingAVDisplay = nullptr;
    MirrorDisplay* mpMirrorDisplay = nullptr;
    AVMeetingSaveVideoToFile mSaveVideoLocal, mSaveVideoRemote;
    AVMeetingSaveAudioToFile mSaveAudioLocal, mSaveAudioRemote;
    AVMeetingSaveToFile* mSave[4] = { &mSaveVideoLocal, &mSaveVideoRemote, &mSaveAudioLocal, &mSaveAudioRemote };
    Expression mLocalVideoVisible, mRemoteVideoVisible;

    int mFilePart = 0;

    Time mStartTime = Time::Never();

    std::string mVideoEncoder;

    AVBidirectionalChannel mChannel;
    std::mutex mChannelMutex;

    int mCameraIndex = -1, mAudioIndex = -1;
    std::string mStreamingAdress;

    std::shared_ptr<AudioStream> mpAudioStream;
    int mAudioStreamIdx = -1;
    std::vector<float> mAudioBuffer;

    Thread mCameraThread;
    MemberCall<void(Private*)> mCameraThreadCall;
    void CameraThread();

    static std::shared_ptr<AVSource> OpenCamera(int index);
    static std::shared_ptr<AudioStream> OpenAudio(int index);

    MemberCall<void(Private*)> mOnAudioTickCall;
    void OnAudioTick();

    Private();
};

AVMeetingFilter::Private::Private()
    : mCameraThreadCall(&Private::CameraThread, this),
      mCameraThread(&mCameraThreadCall, "AVMeetingFilter::Camera Thread"),
      mOnAudioTickCall(&Private::OnAudioTick, this)
{
}

void AVMeetingFilter::Private::CameraThread()
{
    std::shared_ptr<AVSource> pCamera = OpenCamera(mCameraIndex);
    int width = 0, height = 0;
    Rate fps;
    if (pCamera) {
        width = pCamera->Properties().width;
        height = pCamera->Properties().height;
        fps = pCamera->Properties().fps;
    }
    std::vector<uint8_t> data(width * height * 4);

    mpAudioStream = OpenAudio(mAudioIndex);
    int channels = 0, samples = 0;
    Rate samplerate;
    if (mpAudioStream) {
        channels = mpAudioStream->RecordingChannels();
        samples = mpAudioStream->SamplesInBuffer();
        samplerate = mpAudioStream->SampleRate();
        mAudioBuffer.resize(channels * samples);
    }

    std::unique_lock<std::mutex> lock(mChannelMutex);
    mChannel.Open(mStreamingAdress);
    int videoStreamIdx = -1;
    if (pCamera) {
        mChannel.AddVideoStream(&videoStreamIdx, mVideoEncoder);
        if (!mChannel.Error().empty())
            throw bcierr << "AddVideoStream: " << mChannel.Error();
        mChannel.InitializeVideoStream(videoStreamIdx, width, height, fps);
        if (!mChannel.Error().empty())
            throw bcierr << "InitializeVideoStream: " << mChannel.Error();
    }
    if (mpAudioStream) {
        mChannel.AddAudioStream(&mAudioStreamIdx);
        if (!mChannel.Error().empty())
            throw bcierr << "AddAudioStream: " << mChannel.Error();
        mChannel.InitializeAudioStream(mAudioStreamIdx, samples, channels, samplerate);
        if (!mChannel.Error().empty())
            throw bcierr << "InitializeAudioStream: " << mChannel.Error();
    }
    mChannel.BeginWriting();
    if (!mChannel.Error().empty()) {
        bcierr << "When writing to " << mStreamingAdress << ": " << mChannel.Error();
        return;
    }
    lock.unlock();

    mStartTime = TimeUtils::MonotonicTime();
    if (mpAudioStream) {
        mpAudioStream->AddListener(AudioStream::OnTick, &mOnAudioTickCall);
        mpAudioStream->Start();
        if (!mpAudioStream->Error().Empty())
            bcierr << "Could not start audio input stream: " << mpAudioStream->Error();
    }

    Time ts = Time::Never();
    while (!mCameraThread.Terminating()) {
        if (pCamera) {
            if (pCamera->WaitForData(data, ts)) {
                lock.lock();
                mChannel.WriteFrame(videoStreamIdx, data.data(), data.size(), ts - mStartTime);
                if (!mChannel.Error().empty())
                    throw bcierr << mChannel.Error();
                lock.unlock();
            }
        }
        else {
            ThreadUtils::SleepForMs(kDefaultCameraFrameDurationMs);
        }
    }
    if (mpAudioStream) {
        mpAudioStream->Stop();
    }
    lock.lock();
    if (mpAudioStream) {
        mpAudioStream.reset();
        mChannel.FinalizeStream(mAudioStreamIdx);
    }
    if (pCamera) {
        mChannel.FinalizeStream(videoStreamIdx);
    }
    mChannel.Close();
}

std::shared_ptr<AVSource> AVMeetingFilter::Private::OpenCamera(int cameraIndex)
{
    if (cameraIndex < 0)
        return nullptr;

#if _WIN32
    std::unique_ptr<AVSourceEnumerator> pEnumerator = std::make_unique<AVSourceEnumeratorMF>();
#else
    std::unique_ptr<AVSourceEnumerator> pEnumerator = std::make_unique<AVSourceEnumeratorLibAVDevice>();
#endif
    auto devices = pEnumerator->ListDevices();
    if (cameraIndex >= devices.size())
        throw std_runtime_error << "Camera index " << cameraIndex << " is out of range";

    auto mediaTypes = pEnumerator->ListVideoMediaTypes(devices[cameraIndex]);
    VideoMediaType videoType;
    videoType.framerateN = 15;
    videoType.framerateD = 1;
    videoType.width = 640;
    videoType.height = 480;
    videoType = pEnumerator->MatchVideoMediaType(mediaTypes, videoType);

    return std::shared_ptr<AVSource>(pEnumerator->CreateSource(devices[cameraIndex], videoType));
}

std::shared_ptr<AudioStream> AVMeetingFilter::Private::OpenAudio(int audioIndex)
{
    if (audioIndex < 0)
        return nullptr;

    if (!AudioDevice(audioIndex).Exists())
        throw std_runtime_error << "Audio device index " << audioIndex << " is out of range";
    if (!AudioDevice(audioIndex).SupportsRecording())
        throw std_runtime_error << "Audio device " << audioIndex << " does not support recording";

    AudioDevice device(audioIndex);
    auto pStream = std::make_shared<AudioStream>(device);
    pStream->SetSampleRate(Rate::FromHertz(kAudioSamplingRateHz));
    pStream->SetSamplesInBuffer(kAudioBufferSize); // required for AAC codec
    return pStream;
}

void AVMeetingFilter::Private::OnAudioTick()
{
    Assert(!!mpAudioStream);
    mpAudioStream->Read(mAudioBuffer.data(), mpAudioStream->SamplesInBuffer());
    std::unique_lock<std::mutex> lock(mChannelMutex);
    Time::Interval ts = mpAudioStream->RecordingTick().time - mStartTime;
    mChannel.WriteFrame(mAudioStreamIdx, mAudioBuffer.data(), mAudioBuffer.size() * sizeof(float), ts);
    if (!mChannel.Error().empty())
        throw bcierr << mChannel.Error();
}

AVMeetingFilter::AVMeetingFilter()
    : p(new Private)
{
    EnvVariable("BCI2000_AUDIO_MODE").Set("portaudio");
    p->mpReceivingAVDisplay = new ReceivingAVDisplay(Window());
    p->mpReceivingAVDisplay->SetVisible(false);
    p->mpMirrorDisplay = new MirrorDisplay(Window());
    p->mpMirrorDisplay->SetVisible(false);

    p->mChannel.AddRenderClient(AVBidirectionalChannel::local, p->mpMirrorDisplay);
    p->mChannel.AddRenderClient(AVBidirectionalChannel::remote, p->mpReceivingAVDisplay);
}

AVMeetingFilter::~AVMeetingFilter()
{
    Halt();
    delete p->mpReceivingAVDisplay;
    delete p->mpMirrorDisplay;
    delete p;
}

void
AVMeetingFilter::Publish()
{
    BEGIN_PARAMETER_DEFINITIONS
        "Application:AVMeeting int /EnableAVMeeting= 0 0 0 1 // Enable AVMeeting (boolean)",

        "Application:AVMeeting string StreamingServer= upload.bci2000.org:1915 % % % // Streaming server address",
        "Application:AVMeeting int CameraIndex= 0 % % % // Streaming camera input index (0-based, -1 for no video)",
        "Application:AVMeeting int AudioDeviceIndex= -1 % % % // Streaming audio input index (0-based, -1 for no audio)",
        "Application:AVMeeting string VideoEncoder= auto % % % // Name of desired video encoder, e.g. h264_mf or libx264",

        "Application:AVMeeting list LocalVideoRect= 4 0.75 0 1 0.25 % 0 1 // Size and position of local video, in relative coordinates",
        "Application:AVMeeting float LocalVideoZ= 1 % % % // Z coordinate of local video display",
        "Application:AVMeeting string LocalVideoVisibility= 1 % % % // Expression controlling local video visibility",

        "Application:AVMeeting list RemoteVideoRect= 4 0 0 1 1 % 0 1 // Size and position of remote video, in relative coordinates",
        "Application:AVMeeting float RemoteVideoZ= 2 % % % // Z coordinate of remote video display",
        "Application:AVMeeting string RemoteVideoVisibility= 1 % % % // Expression controlling remote video visibility",
    END_PARAMETER_DEFINITIONS

    BEGIN_EVENT_DEFINITIONS
        "VideoStreamingLocalTotalFrames 32 0 0 0",
        "VideoStreamingRemoteTotalFrames 32 0 0 0",
        "VideoStreamingLocalFrame 32 0 0 0",
        "VideoStreamingRemoteFrame 32 0 0 0",
        "AudioStreamingLocalTotalFrames 32 0 0 0",
        "AudioStreamingRemoteTotalFrames 32 0 0 0",
        "AudioStreamingLocalFrame 32 0 0 0",
        "AudioStreamingRemoteFrame 32 0 0 0",
        END_EVENT_DEFINITIONS
}

void
AVMeetingFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
    if (!Parameter("/EnableAVMeeting"))
        return;

    AudioDevice::Rescan(AudioDevice::portaudio);
    std::ostringstream oss;
    for (int i = 0; AudioDevice(i).Exists(); ++i) {
        if (AudioDevice(i).SupportsRecording())
            oss << i << ": " << AudioDevice(i).Name() << "\n";
    }
    bciout << "Audio recording devices:\n" << oss.str();

    std::string address = Parameter("StreamingServer");
    client_tcpsocket socket;
    socket.create();
    socket.set_blocking(false);
    socket.open(address);
    if (!socket.wait_for_write(kServerTimeoutMs))
        throw bcierr << "Cannot connect to " << address;
    socket.close();

    ReusableThread rt; // a separate thread is necessary because of incompatible COM threading models on Windows
    FunctionCall<std::shared_ptr<AVSource>(int)> openCamera(&Private::OpenCamera, Parameter("CameraIndex").ToNumber());
    FunctionCall<std::shared_ptr<AudioStream>(int)> openAudio(&Private::OpenAudio, Parameter("AudioDeviceIndex").ToNumber());
    rt.RunSynchronously(openCamera);
    rt.RunSynchronously(openAudio);

    std::string encoder = Parameter("VideoEncoder");
    if (encoder == "auto")
        encoder = "";
    AVBidirectionalChannel channel;
    int idx = -1;
    channel.AddVideoStream(&idx, encoder);
    if (!channel.Error().empty())
        throw bcierr << channel.Error();
    channel.AddAudioStream(&idx);
    if (!channel.Error().empty())
        throw bcierr << channel.Error();

    Parameter("LocalVideoZ");
    Parameter("RemoteVideoZ");
    Expression(Parameter("LocalVideoVisibility")).Compile();
    Expression(Parameter("RemoteVideoVisibility")).Compile();

    State("FilePart");

    Output = Input;
}


void
AVMeetingFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
    p->mpReceivingAVDisplay->SetVisible(false);
    p->mpMirrorDisplay->SetVisible(false);

    p->mEnabled = Parameter("/EnableAVMeeting");
    if (!p->mEnabled)
        return;

    p->mpReceivingAVDisplay->SetObjectRect(RectFromParam(Parameter("RemoteVideoRect")));
    p->mpReceivingAVDisplay->SetBackgroundText("Remote");
    p->mpReceivingAVDisplay->SetZOrder(Parameter("RemoteVideoZ"));
    p->mRemoteVideoVisible = Parameter("RemoteVideoVisibility").ToString();
    p->mpReceivingAVDisplay->SetVisible(p->mRemoteVideoVisible.Evaluate());

    p->mpMirrorDisplay->SetObjectRect(RectFromParam(Parameter("LocalVideoRect")));
    p->mpMirrorDisplay->SetBackgroundText("Local");
    p->mpMirrorDisplay->SetZOrder(Parameter("LocalVideoZ"));
    p->mLocalVideoVisible = Parameter("LocalVideoVisibility").ToString();
    p->mpMirrorDisplay->SetVisible(p->mLocalVideoVisible.Evaluate());

    p->mChannel.Close();

    p->mCameraThread.Terminate();

    p->mChannel.AddRenderClient(AVBidirectionalChannel::remote, p->mpReceivingAVDisplay);
    p->mChannel.AddRenderClient(AVBidirectionalChannel::local, p->mpMirrorDisplay);

    p->mCameraIndex = Parameter("CameraIndex");
    p->mAudioIndex = Parameter("AudioDeviceIndex");
    p->mVideoEncoder = Parameter("VideoEncoder").ToString();
    if (p->mVideoEncoder == "auto")
        p->mVideoEncoder = "";
    p->mStreamingAdress = Parameter("StreamingServer").ToString();
    if (!p->mStreamingAdress.empty())
        p->mCameraThread.Start();

}

void
AVMeetingFilter::StartRun()
{
    if (!p->mEnabled)
        return;

    p->mpMirrorDisplay->SetVisible(p->mLocalVideoVisible.Evaluate());
    p->mpReceivingAVDisplay->SetVisible(p->mRemoteVideoVisible.Evaluate());

    p->mFilePart = 1;
    std::string baseName = FileUtils::StripExtension(RunManager()->CurrentRun());
    p->mSaveVideoLocal.StartRecording(baseName + "_VideoStreamingLocal");
    p->mSaveVideoLocal.SetEventPrefixFrame("VideoStreamingLocal");
    if (!p->mSaveVideoLocal.Error().empty())
        bcierr << p->mSaveVideoLocal.Error();
    p->mSaveVideoRemote.StartRecording(baseName + "_VideoStreamingRemote");
    p->mSaveVideoRemote.SetEventPrefixFrame("VideoStreamingRemote");
    if (!p->mSaveVideoRemote.Error().empty())
        bcierr << p->mSaveVideoRemote.Error();

    p->mSaveAudioLocal.StartRecording(baseName + "_AudioStreamingLocal");
    p->mSaveAudioLocal.SetEventPrefixFrame("AudioStreamingLocal");
    if (!p->mSaveAudioLocal.Error().empty())
        bcierr << p->mSaveAudioLocal.Error();
    p->mSaveAudioRemote.StartRecording(baseName + "_AudioStreamingRemote");
    p->mSaveAudioRemote.SetEventPrefixFrame("AudioStreamingRemote");
    if (!p->mSaveAudioRemote.Error().empty())
        bcierr << p->mSaveAudioRemote.Error();

    p->mChannel.AddRenderClient(AVBidirectionalChannel::local, &p->mSaveVideoLocal);
    p->mChannel.AddRenderClient(AVBidirectionalChannel::remote, &p->mSaveVideoRemote);
    p->mChannel.AddRenderClient(AVBidirectionalChannel::local, &p->mSaveAudioLocal);
    p->mChannel.AddRenderClient(AVBidirectionalChannel::remote, &p->mSaveAudioRemote);
}


void
AVMeetingFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
    Output = Input;

    if (!p->mEnabled)
        return;

    p->mpMirrorDisplay->SetVisible(p->mLocalVideoVisible.Evaluate());
    p->mpReceivingAVDisplay->SetVisible(p->mRemoteVideoVisible.Evaluate());

    int nextFilePart = State("FilePart")(Statevector->Samples() - 1);
    if (nextFilePart > p->mFilePart)
    {
        for (auto pSave : p->mSave) {
            pSave->IncrementFilePart();
            if (!pSave->Error().empty())
                bcierr << pSave->Error();
        }
        p->mFilePart = nextFilePart;
    }

}

void
AVMeetingFilter::StopRun()
{
    if (!p->mEnabled)
        return;

    p->mpMirrorDisplay->SetVisible(p->mLocalVideoVisible.Evaluate());
    p->mpReceivingAVDisplay->SetVisible(p->mRemoteVideoVisible.Evaluate());

    for (auto pSave : p->mSave) {
        pSave->StopRecording();
        if (!pSave->Error().empty())
            bcierr << pSave->Error();
        pSave->SetEventPrefixFrame("");
    }
    p->mChannel.RemoveRenderClient(AVBidirectionalChannel::local, &p->mSaveVideoLocal);
    p->mChannel.RemoveRenderClient(AVBidirectionalChannel::remote, &p->mSaveVideoRemote);
    p->mChannel.RemoveRenderClient(AVBidirectionalChannel::local, &p->mSaveAudioLocal);
    p->mChannel.RemoveRenderClient(AVBidirectionalChannel::remote, &p->mSaveAudioRemote);
}

void
AVMeetingFilter::Halt()
{
    p->mCameraThread.Terminate();
    p->mChannel.Close();
}

