#include "NetworkHIDExtension.h"
#include "Environment.h"

Extension( NetworkHIDExtension );

NetworkHIDExtension::NetworkHIDExtension() {
    PublishEnabler("EnableNetworkHIDLogging");
}

void NetworkHIDExtension::Publish() {
    if (OptionalParameter("EnableNetworkHIDLogging")) {
        BEGIN_PARAMETER_DEFINITIONS
            "Source:NetworkHID int /EnableNetworkHIDLogging= 1 0 0 1 "
                " // Enable the network HID logger (boolean)",
            "Source:NetworkHID int EnableKeyboardLogging= 1 0 0 1 "
                " // Enable Logging of keystrokes to a network HID (boolean)",
            "Source:NetworkHID int EnableMouseMovementLogging= 1 0 0 1 "
                " // Enable Logging of mouse movement to a network HID (boolean)",
            "Source:NetworkHID int EnableMouseButtonLogging= 1 0 0 1 "
                " // Enable Logging of mouse button state to a network HID (boolean)",
            "Source:NetworkHID string NetworkHIDAddress= 192.168.4.1:1024 % % % "
                " // network address of the Network HID device in IP:port format",
            "Source:NetworkHID float MouseMovementLoggingScale= 1.0 % % % "
                " // Scaling factor for logging mouse movement",
        END_PARAMETER_DEFINITIONS
    }
}

void NetworkHIDExtension::Preflight() const {
    if (!OptionalParameter("EnableNetworkHIDLogging")) {
        return;
    }
    Parameter("SampleBlockSize");
    Parameter("EnableNetworkHIDLogging");
    if (Parameter("EnableKeyboardLogging")) {
        State("KeyUp");
        State("KeyDown");
    }
    if (Parameter("EnableMouseMovementLogging")) {
        State("MousePosX");
        State("MousePosY");
    }
    if (Parameter("EnableMouseButtonLogging")) {
        State("MouseKeys");
    }
    Parameter("SampleBlockSize");
    State("Running");
    std::string hidDestinationAddress = Parameter("NetworkHIDAddress");
    if (!hidDestinationAddress.empty()) {
        ClientTCPSocket socket;
        socket.Open(hidDestinationAddress);
        if (!socket.IsOpen())
            bcierr << "Could not connect to NetworkHID at " << hidDestinationAddress;
    }
}

void NetworkHIDExtension::Initialize() {
    if (!OptionalParameter("EnableNetworkHIDLogging")) return;
    mEnableNetworkHIDLogging = Parameter("EnableNetworkHIDLogging");
    std::string hidDestinationAddress = Parameter("NetworkHIDAddress");
    int sampleBlockSize = Parameter("SampleBlockSize");

    mpHid.reset();
    if (!hidDestinationAddress.empty()) {
        mpHid = std::make_unique<NetworkHID>(hidDestinationAddress, sampleBlockSize);
        mpHid->Initialize();
    }
}


void NetworkHIDExtension::StartRun() {
    if (!mEnableNetworkHIDLogging)
        return;

    mSampleBlockSize = Parameter("SampleBlockSize");
    mLogKeyboard = Parameter("EnableKeyboardLogging");
    mLogMouseMovement = Parameter("EnableMouseMovementLogging");
    mLogMouseButtons = Parameter("EnableMouseButtonLogging");
}
void NetworkHIDExtension::PostProcess() {
    if (!mEnableNetworkHIDLogging) return;
    if (!State("Running")) return;

    for (int sample_idx = 0; sample_idx < mSampleBlockSize; sample_idx++) {
        if (mLogKeyboard) {
            int keyDown = State("KeyDown")(sample_idx);
            int keyUp = State("KeyUp")(sample_idx);
            if (keyDown) {
                if (keyDown > UINT8_MAX) {
                    bciout << "detected keycode " << std::hex << keyDown << " outside ascii range, not sending to Network HID.\n";
                }
                mpHid->EnqueueKeyPress(static_cast<uint8_t>(keyDown));
            }
            if (keyUp) {
                if (keyUp > UINT8_MAX) {
                    bciout << "detected keycode " << std::hex << keyUp << " outside ascii range, not sending to Network HID.\n";
                }
                mpHid->EnqueueKeyRelease(static_cast<uint8_t>(keyUp));
            }
        }

        if (mLogMouseButtons) {
            uint32_t mouseButtons = State("MouseKeys")(sample_idx);
            if (mouseButtons != mMousePreviousButtons) {
                mMousePreviousButtons = mouseButtons;
                bool mouse1 = static_cast<bool>(mouseButtons & 1);
                bool mouse2 = static_cast<bool>((mouseButtons >> 1) & 1);
                mpHid->EnqueueMouseButtons(mouse1, mouse2);
            }
        }

        if (mLogMouseMovement) {
            // Mouse positions are stored with origin at 32768,32768 because states are stored unsigned,
            // This should not matter because we send relative mouse positions, but we translate anyway
            // for simplicity and because the previous mouse positions initialize to 0, so we avoid a massive
            // movement at startup
            uint32_t mouseX = State("MousePosX")(sample_idx) - 32768;
            uint32_t mouseY = State("MousePosY")(sample_idx) - 32768;
            uint32_t dMouseX = mouseX - mMousePreviousX;
            uint32_t dMouseY = mouseY - mMousePreviousY;
            mMousePreviousX = mouseX;
            mMousePreviousY = mouseY;

            if (dMouseX | dMouseY) {
                mpHid->EnqueueMouseMovement(dMouseX, dMouseY);
            }
        }
    }
    mpHid->SendEnqueuedCommands();
}
