////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: NeusenW4ADC implementation
////////////////////////////////////////////////////////////////////////////////
#include "NeusenW4ADC.h"

#include "BCIStream.h"

#include <chrono>

RegisterFilter(NeusenW4ADC, 1);

namespace {
    static const auto kDeviceDiscoveryTimeGranularity = std::chrono::seconds(1);
    static const auto kDeviceDiscoveryTimeout = std::chrono::seconds(5);
    static const int kAbortOnBufferSeconds = 20;

    struct NeuracleSucceedOrThrow
    {
        NeuracleSucceedOrThrow(NeuracleV4ResultCode code)
        {
            switch (code) {
            case NeuracleV4Result_Succeed:
            case NeuracleV4Result_Information_ReconnectSucceeded:
            case NeuracleV4Result_Information_SyncSucceeded:
            case NeuracleV4Result_Information_TriggerArrived:
            case NeuracleV4Result_Information_ValueNoChanging:
                break;
            default:
                throw bcierr << "Unhandled Neuracle SDK error: " << NeuracleV4GetResultCodeString(code);
            }
        }
    };

static const char* NeuracleEEGElectrodeNames[] = {
#if 0 // channel names as communicated by Neuracle
    "TP8", "F4", "CP2", "Pz", "P3", "AF3", "TP7", "C3",
    "PO8", "P4", "Fp1", "F5", "F3",  "F6", "FC2", "F1",
    "PO5", "Oz", "C4", "HEOR", "AF7", "O1", "Fp2", "AF8",
    "F2", "CP6", "T8", "FC5", "Fpz", "CP1", "F7", "C5",
    "P7", "PO3", "FC1", "PO7", "FC6", "FCz", "C2", "Fz",
    "O2", "VEOL", "FC3", "FT8", "C6", "CP4", "C1", "CP5",
    "VEOU", "PO4", "F8", "ECG", "HEOL", "AF4", "P6", "P8",
    "P5", "FT7", "T7", "CP3", "Cz", "POZ", "PO6", "FC4",
#else // channel names as communicated by jswift
    "Cz", "CP1", "C4", "Pz", "O1", "Fp1", "CP6", "O2",
    "CP2", "P8", "C3", "P7", "Fp2", "F8", "CP5", "F7",
    "HEOL", "FC5", "HEOR", "P4", "T7", "F3", "T8", "FC2",
    "PO4", "FC6", "P3", "PO3", "Oz", "F4", "FC1", "Fz",
    "FC4", "Fpz", "P6", "FCz", "C6", "POz", "F6", "PO6",
    "PO8", "C2", "TP8", "F2", "FT8", "AF4", "AF8", "CP4",
    "AF7", "CP3", "FT7", "AF3", "TP7", "F1", "PO7", "C1",
    "F5", "PO5", "C5", "ECG", "P5", "VEOL", "FC3", "VEOU",
#endif
};
static const int NeuracleEEGElectrodeNamesCount = sizeof(NeuracleEEGElectrodeNames) / sizeof(*NeuracleEEGElectrodeNames);

} // namespace

// Exception-safe HostInfoV4 wrapper
NeuracleDeviceInfoPtr NeuracleCreateDeviceInfo()
{
    HostInfoV4* pInfo = new HostInfoV4{ 0 };
    return NeuracleDeviceInfoPtr(pInfo, NeuracleCleanupDeviceInfoPtr);
}

void NeuracleCleanupDeviceInfoPtr(HostInfoV4* pDeviceInfo)
{
    NeuracleV4FreeDevice(pDeviceInfo);
    delete pDeviceInfo;
}

NeusenW4ADC::NeusenW4ADC()
: mpController(nullptr, nullptr)
{
}

NeusenW4ADC::~NeusenW4ADC()
{
}

void
NeusenW4ADC::Publish()
{
 BEGIN_PARAMETER_DEFINITIONS

    "Source:Signal%20Properties int SourceCh= auto "
       "% % % // number of digitized and stored channels",

    "Source:Signal%20Properties float SamplingRate= auto "
       "% % % // sampling rate",

    "Source:Signal%20Properties int SampleBlockSize= auto "
       "% % % / sampleblockSize",

    "Source:Signal%20Properties list SourceChGain= 1 auto "
       " // physical units per raw A/D unit",

    "Source:Signal%20Properties list SourceChOffset= 1 auto "
       " // raw A/D offset to subtract, typically 0",

    "Source:Signal%20Properties list ChannelNames= 1 auto "
       " // names of amplifier channels",

    "Source string DeviceSerialNumber= auto "
       " // serial number of device to connect to, "
       "use auto to connect to a single available device",

 END_PARAMETER_DEFINITIONS
}

void
NeusenW4ADC::AutoConfig(const SignalProperties&)
{
    // auto sampling rate must be set here to allow for proper device connection
    Parameter("SamplingRate") = "1000Hz";
    if (!DiscoverDevices(mEEGDevices)) {
        throw bcierr << "Did not discover any EEG devices";
    }
    if (ActualParameter("DeviceSerialNumber") == "auto" && mEEGDevices.size() > 1) {
        throw bcierr << "More than one EEG device found, must specify desired serial number in \"DeviceSerialNumber\"";
    }
    std::ostringstream oss;
    oss << std::hex << mEEGDevices.front()->SerialNumber;
    Parameter("DeviceSerialNumber") = oss.str();

    std::istringstream iss(ActualParameter("DeviceSerialNumber"));
    unsigned int actualSerialNumber = 0;
    iss >> std::hex >> actualSerialNumber;

    mpController = ConnectDevices(mEEGDevices, actualSerialNumber, mEEGSensors);
    if (!mpController) {
        throw bcierr << "Could not connect devices to controller";
    }
    if (mEEGSensors.empty()) {
        throw bcierr << "No EEG sensors were detected";
    }
    Parameter("SourceCh") = GetTotalChannels(mEEGSensors);
    int actualSourceCh = ActualParameter("SourceCh");
    Parameter("SourceChGain")->SetNumValues(actualSourceCh);
    Parameter("SourceChOffset")->SetNumValues(actualSourceCh);
    Parameter("ChannelNames")->SetNumValues(actualSourceCh);
    int j = 0;
    for (const auto& sensor : mEEGSensors) {
        for (int i = 0; i < sensor.pDeviceType->channelCount && j < actualSourceCh; ++i) {
            Parameter("SourceChGain")(j) << sensor.pDeviceType->gain << sensor.pDeviceType->units;
            Parameter("SourceChOffset")(j) = 0;
            if (actualSourceCh == NeuracleEEGElectrodeNamesCount) {
                Parameter("ChannelNames")(j) << NeuracleEEGElectrodeNames[j];
            }
            else {
                Parameter("ChannelNames")(j) << sensor.pDeviceType->sensorType << (j+1);
            }
             
            ++j;
        }
    }
    double samplingRate = ActualParameter("SamplingRate").InHertz();
    // the NeusenW4 API does not allow a sample block duration below 100ms
    Parameter("SampleBlockSize") = 100 * samplingRate / 1000;
}

void
NeusenW4ADC::Preflight(const SignalProperties& Input, SignalProperties& Output) const
{
    double samplingRate = Parameter("SamplingRate").InHertz();
    if (::floor(samplingRate) != samplingRate) {
        throw bcierr << "SamplingRate must be an integer value in Hz";
    }

    bool samplingRateValidForAllSensors = true;
    for (const auto& sensor : mEEGSensors) {
        bool samplingRateValidForSensor = false;
        for (int i = 0; i < sensor.pDeviceType->supportedSampleRatesCount; ++i) {
            if (samplingRate == sensor.pDeviceType->supportedSampleRates[i]) {
                samplingRateValidForSensor = true;
                break;
            }
        }
        if (!samplingRateValidForSensor) {
            bcierr << "Module " << sensor.pModule->SerialNumber << ", sensor " << sensor.pDeviceType->sensorType
                   << " does not support a sampling rate of " << samplingRate << "Hz";
            samplingRateValidForAllSensors = false;
        }
    }
    if (!samplingRateValidForAllSensors) {
        throw bcierr << "Invalid SamplingRate: " << samplingRate << "Hz";
    }

    int j = 0;
    for (const auto& sensor : mEEGSensors) {
        for (int i = 0; i < sensor.pDeviceType->channelCount; ++i) {
            std::ostringstream oss;
            oss << sensor.pDeviceType->gain << sensor.pDeviceType->units;
            if (Parameter("SourceChGain")(j) != oss.str())
                bcierr << "SourceChGain does not match sensor gain and units";
            if (Parameter("SourceChOffset")(j) != 0)
                bcierr << "SourceChOffset should be 0";
            ++j;
        }
    }

    int totalChannels = GetTotalChannels(mEEGSensors);
    int sourceCh = Parameter("SourceCh");
    if (sourceCh != totalChannels) {
        throw bcierr << "Number of channels is set to " << sourceCh << ", must match total number of EEG channels (" << totalChannels << ")";
    }

    int sampleBlockSize = Parameter("SampleBlockSize");
    if (std::ceil(sampleBlockSize / samplingRate) < 100e-3) {
        throw bcierr << "SampleBlockSize is too small, the NeusenW4 API does not allow a sample block duration below 100ms";
    }
    Output = SignalProperties(sourceCh, sampleBlockSize, SignalType::float32);
}

void
NeusenW4ADC::Initialize(const SignalProperties& Input, const SignalProperties& Output)
{
    mAcquisitionSleepMs = GetMaxDelayMs() / 2;
    mCurrentDataSample = 0;
    if (mpCurrentDataBlock) {
        ::NeuracleV4FreeDataBlock(mpCurrentDataBlock);
        mpCurrentDataBlock = nullptr;
    }
    double samplingRate = Parameter("SamplingRate").InHertz();
    mAbortOnBufferSamples = ::ceil(kAbortOnBufferSeconds * samplingRate);

    NeuracleV4ResultCode result = NeuracleV4Result_Succeed;

    //NOTE:
    //   NeuracleV4ControllerSetEEGParameter() && NeuracleV4ControllerConnect() && NeuracleV4ControllerStart()
    //   can be invoked only once for one test

    for (const auto& sensor : mEEGSensors) {
        result = ::NeuracleV4ControllerSetEEGParameter(mpController.get(),
            sensor.pModule->SerialNumber,
            sensor.pModule->BoardFlag,
            sensor.pModule->CpuFlag,
            sensor.pDeviceType,
            sensor.pDeviceType->sensorTypeForProtocol,
            sensor.pDeviceType->gain,
            WorkModeV4_Signal,
            sensor.pDeviceType->channelCount,
            samplingRate,
            0);
        if (result != NeuracleV4Result_Succeed)
        {
            throw bcierr << "NeuracleV4ControllerSetEEGParameter() failed " << ::NeuracleV4GetResultCodeString(result);
        }
    }
    
    result = ::NeuracleV4ControllerConnect(mpController.get());
    if (result != NeuracleV4Result_Succeed)
    {
        throw bcierr << "NeuracleV4ControllerConnect() failed " << ::NeuracleV4GetResultCodeString(result);
    }

    result = ::NeuracleV4ControllerStart(mpController.get());
    if (result != NeuracleV4Result_Succeed)
    {
        throw bcierr << "NeuracleV4ControllerStart() failed " << ::NeuracleV4GetResultCodeString(result);
    }
}

void NeusenW4ADC::Process(const GenericSignal& Input, GenericSignal& Output)
{
    int outputSample = 0;
    auto pOutput = Output.MutableData(GenericSignal::intentOverwriteContents);

    // use data from existing data block first
    while (mpCurrentDataBlock && mCurrentDataSample < mpCurrentDataBlock->DataCountPerChannel && outputSample < Output.Elements()) {
        Assert(mpCurrentDataBlock->ChannelCount == Output.Channels());
        for (int channel = 0; channel < mpCurrentDataBlock->ChannelCount; ++channel) {
            pOutput[Output.LinearIndex(channel, outputSample)]
                = mpCurrentDataBlock->Datas[mCurrentDataSample + channel * mpCurrentDataBlock->DataCountPerChannel];
        }
        ++mCurrentDataSample;
        ++outputSample;
    }

    NeuracleV4ResultCode result = NeuracleV4Result_Succeed;
    // still not enough data to fill output, read more data
    while (outputSample < Output.Elements()) {
        if (mpCurrentDataBlock) {
            ::NeuracleV4FreeDataBlock(mpCurrentDataBlock);
            mpCurrentDataBlock = nullptr;
        }
        mCurrentDataSample = 0;

        //NOTE: NeuracleV4ControllerReadData() should NOT be blocked, it's better working in a separated thread
        int remainingDataLen = 0;
        do {
            result = ::NeuracleV4ControllerReadData(mpController.get(), &mpCurrentDataBlock, 0, &remainingDataLen);
            if (remainingDataLen > mAbortOnBufferSamples) {
                result = ::NeuracleV4ControllerStop(mpController.get());
                throw bcierr << "Buffer reading too slow, internal queue exceeds a size of " << kAbortOnBufferSeconds << " seconds"
                             << " (NeuracleV4ControllerStop() returned " << ::NeuracleV4GetResultCodeString(result) << ")";
            }
            if (result == NeuracleV4Result_Warning_NoEnoughData)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else if (result != NeuracleV4Result_Succeed &&
                result != NeuracleV4Result_Information_ReconnectSucceeded &&
                result != NeuracleV4Result_Warning_DeviceDisconnected)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } while (result == NeuracleV4Result_Warning_NoEnoughData);

        // copy data from acquired block to output
        while (mpCurrentDataBlock && mCurrentDataSample < mpCurrentDataBlock->DataCountPerChannel && outputSample < Output.Elements())
        {
            Assert(mpCurrentDataBlock->ChannelCount == Output.Channels());
            for (int channel = 0; channel < mpCurrentDataBlock->ChannelCount; ++channel) {
                pOutput[Output.LinearIndex(channel, outputSample)]
                    = mpCurrentDataBlock->Datas[mCurrentDataSample + channel * mpCurrentDataBlock->DataCountPerChannel];
            }
            ++mCurrentDataSample;
            ++outputSample;
        }
    }
}

void NeusenW4ADC::Halt()
{
    ::NeuracleV4ControllerStop(mpController.get());
    if (mpCurrentDataBlock) {
        ::NeuracleV4FreeDataBlock(mpCurrentDataBlock);
        mpCurrentDataBlock = nullptr;
    }
    mpController.reset();
}

bool NeusenW4ADC::DiscoverDevices(std::vector<NeuracleDeviceInfoPtr>& devices) const
{
    devices.clear();

    NeuracleV4DiscoveryController* pDiscoveryControllerTemp = nullptr;
    NeuracleSucceedOrThrow err = NeuracleV4CreateDeviceDiscovery(&pDiscoveryControllerTemp);
    // use unique_ptr<> with custom deleter for exception safety
    std::unique_ptr<NeuracleV4DiscoveryController, decltype(&NeuracleV4DeleteDeviceDiscovery)>
        pDiscoveryController(pDiscoveryControllerTemp, &NeuracleV4DeleteDeviceDiscovery);

    err = NeuracleV4StartDeviceDiscovery(pDiscoveryController.get());
    bool timeout = false;
    int deviceCount = 0;
    auto begin = std::chrono::steady_clock::now();
    while (!timeout && deviceCount < 1) {
        std::this_thread::sleep_for(kDeviceDiscoveryTimeGranularity);
        err = NeuracleV4GetFoundedDevicesCount(pDiscoveryController.get(), &deviceCount);
        timeout = (std::chrono::steady_clock::now() - begin >= kDeviceDiscoveryTimeout);
    }
    NeuracleV4StopDeviceDiscovery(pDiscoveryController.get());

    std::ostringstream oss;
    for (int i = 0; i < deviceCount; ++i) {
        NeuracleDeviceInfoPtr pDevice = NeuracleCreateDeviceInfo();
        err = NeuracleV4GetFoundedDevicesByIndex(pDiscoveryController.get(), pDevice.get(), i);
        if (NeuracleV4IsValidEEGHost(pDevice->DeviceType)) {
            oss << "\t" << std::hex << pDevice->SerialNumber << "\n";
            devices.push_back(std::move(pDevice));
        }
    }
    if (!devices.empty()) {
        bciout << "Serial numbers of NeusenW EEG devices present:\n" << oss.str() << "\n";
    }
    return !devices.empty();
}

NeuracleV4ControllerPtr NeusenW4ADC::ConnectDevices(const std::vector<NeuracleDeviceInfoPtr>& devices, unsigned int serialNumber, std::vector<SensorInfo>& sensors) const
{
    sensors.clear();

    NeuracleV4Controller* pDeviceControllerTemp = nullptr;
    NeuracleSucceedOrThrow err = NeuracleV4ControllerInitialize(&pDeviceControllerTemp, true);
    NeuracleV4ControllerPtr pDeviceController(pDeviceControllerTemp, &NeuracleV4ControllerFinalize);
    err = NeuracleV4ControllerSkipMergeTriggerToEEGPacket(pDeviceController.get());

    bool found = false;
    for (const auto& pDevice : devices) {
        if (pDevice->SerialNumber == serialNumber) {
            AddNeuracleEEGHost(pDeviceController.get(), pDevice.get(), sensors);
            bciout << "Connected to device with serial number " << std::hex << serialNumber;
            found = true;
        }
    }
    if (!found) {
        bciwarn << "Could not find device with serial number " << std::hex << serialNumber;
    }

    return pDeviceController;
}

void NeusenW4ADC::AddNeuracleEEGHost(NeuracleV4Controller* pController, HostInfoV4* pDevice, std::vector<SensorInfo>& sensors) const
{
    if (pDevice->ModuleCount < 1) {
        throw bcierr << "EEG host does not contain any modules";
    }
    int maxDelayMs = 5;//GetMaxDelayMs();
    
    NeuracleSucceedOrThrow err = NeuracleV4ControllerAddEEGHost(pController,
        pDevice->IPAddress, pDevice->TcpListeningPort,
        pDevice->DeviceType, maxDelayMs
    );

    for (int i = 0; i < pDevice->ModuleCount; ++i) {
        auto& module = pDevice->ModuleInfos[i];

        if (!NeuracleV4IsValidEEGModule(module.DeviceType)) {
            bciwarn << "Invalid module: " << module.DeviceType;
            continue;
        }
        err = NeuracleV4ControllerAddModule(pController, module.SerialNumber, reinterpret_cast<char*>(module.DeviceType));

        for (int j = 0; j < module.SensorCount; ++j) {
            SensorDeviceType* pSensorDeviceType = NeuracleV4GetEEGSensorDeviceTypeByName(reinterpret_cast<char*>(module.SensorInfos[j].DeviceType));
            if (pSensorDeviceType) {
                if (pSensorDeviceType->sensorTypeForProtocol == SensorTypeV4_EEG) {
                    SensorInfo sensorInfo = { &module, pSensorDeviceType };
                    sensors.push_back(sensorInfo);
                }
                else {
                    bciwarn << "Unsupported sensor type: " << pSensorDeviceType->sensorType;
                }
            }
        }
    }
}

void NeusenW4ADC::AddNeuracleTriggerBox(NeuracleV4Controller* pController, HostInfoV4* pDevice) const
{
    NeuracleSucceedOrThrow err = NeuracleV4ControllerAddTriggerBox(pController,
        pDevice->IPAddress, pDevice->TcpListeningPort, 
        pDevice->DeviceType, pDevice->FirmwareVersion
    );
}

int NeusenW4ADC::GetMaxDelayMs() const 
{
    double samplingRate = ActualParameter("SamplingRate").InHertz();
    int sampleBlockSize = ActualParameter("SampleBlockSize");

    int maxDelayMs = ::floor(sampleBlockSize * 1000 / samplingRate);
    maxDelayMs = (maxDelayMs / 2) * 2;
    if (maxDelayMs > 1000)
        maxDelayMs = 1000;
    else if (maxDelayMs < 100)
        maxDelayMs = 100;

    return maxDelayMs;
}

int NeusenW4ADC::GetTotalChannels(const std::vector<SensorInfo>& sensors) const
{
    int totalChannels = 0;
    for (const auto& sensor : sensors) {
        totalChannels += sensor.pDeviceType->channelCount;
    }
    return totalChannels;
}
