////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: NeusenW4ADC header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_NEUSENW4ADC_H
#define INCLUDED_NEUSENW4ADC_H

#include "GenericADC.h"
#include "NeusenW4SDK.h"

#include <memory>
#include <vector>

// Exception-safe HostInfoV4 wrapper
void NeuracleCleanupDeviceInfoPtr(HostInfoV4*);
typedef std::unique_ptr<HostInfoV4, decltype(&NeuracleCleanupDeviceInfoPtr)> NeuracleDeviceInfoPtr;
NeuracleDeviceInfoPtr NeuracleCreateDeviceInfo();

// Exception-safe NeuracleV4Controller wrapper
typedef std::unique_ptr<NeuracleV4Controller, decltype(&NeuracleV4ControllerFinalize)> NeuracleV4ControllerPtr;

class NeusenW4ADC : public GenericADC
{
 public:
    NeusenW4ADC();
    ~NeusenW4ADC();
    void Publish() override;
    void AutoConfig(const SignalProperties&) override;
    void Preflight(const SignalProperties& Input, SignalProperties& Output) const override;
    void Initialize(const SignalProperties& Input, const SignalProperties& Output) override;
    void Process(const GenericSignal& Input, GenericSignal& Output) override;
    void Halt() override;

 private:
    struct SensorInfo { ModuleInfoV4* pModule; SensorDeviceType* pDeviceType; };
    bool DiscoverDevices(std::vector<NeuracleDeviceInfoPtr>&) const;
    NeuracleV4ControllerPtr ConnectDevices(const std::vector<NeuracleDeviceInfoPtr>&, unsigned int serialNumber, std::vector<SensorInfo>&) const;
    void AddNeuracleEEGHost(NeuracleV4Controller*, HostInfoV4*, std::vector<SensorInfo>&) const;
    void AddNeuracleTriggerBox(NeuracleV4Controller*, HostInfoV4*) const;

    int GetMaxDelayMs() const;
    int GetTotalChannels(const std::vector<SensorInfo>&) const;

    // this holds the entire device tree and may not be cleared during operation
    std::vector<NeuracleDeviceInfoPtr> mEEGDevices;

    // this holds pointers to data structures inside the controller and the device tree
    std::vector<SensorInfo> mEEGSensors;

    // this is a smart pointer to a NeurcacleV4Controller
    NeuracleV4ControllerPtr mpController;

    int mAbortOnBufferSamples = 0;
    int mAcquisitionSleepMs = 1;
    DataBlockV4* mpCurrentDataBlock = nullptr;
    int mCurrentDataSample = 0;
};

#endif // INCLUDED_NEUSENW4ADC_H
