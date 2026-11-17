#ifndef NETWORK_HID_EXTENSION_H
#define NETWORK_HID_EXTENSION_H
#include "Environment.h"
#include "NetworkHID.h"

class NetworkHIDExtension : public EnvironmentExtension {
    public:
        NetworkHIDExtension();
        void Publish() override;
        void Preflight() const override;
        void Initialize() override;
        void StartRun() override;
        void PostProcess() override;

    private:
        bool mEnableNetworkHIDLogging = false;
        bool mLogKeyboard = false;
        bool mLogMouseMovement = false;
        bool mLogMouseButtons = false;

        std::unique_ptr<NetworkHID> mpHid;
        uint32_t mSampleBlockSize = 0;
        uint32_t mMousePreviousButtons = 0;
        uint32_t mMousePreviousX = 0;
        uint32_t mMousePreviousY = 0;

        float mMouseMovementScale = 1.0f;
};
#endif
