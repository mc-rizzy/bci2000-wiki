#ifndef NETHID_H
#define NETHID_H

#include "Sockets.h"

class NetworkHID {
public:
    NetworkHID(std::string addr, int sampleBlockSize);
    ~NetworkHID();

    /**
     * Attempt to connect to the device. You must call this before calling any of
     * the command methods.
     */
    void Initialize();

    /**
     * Documentation for the NetworkHID communications protocol, including the
     * description of the commands sent to the NetworkHID device, is located at []
     */


     /**
      * The following four methods enqueue their commands, but do not send them
      * over the network. This is useful for sending many commands in quick succession,
      * as network communication happens only when the enqueued commands are sent by
      * SendEnqueuedCommands, rather than once for each command.
      */
    void EnqueueKeyPress(uint8_t keycode);
    void EnqueueKeyRelease(uint8_t keycode);
    void EnqueueMouseMovement(int32_t mouse_x, int32_t mouse_y);
    void EnqueueMouseButtons(bool mouse1, bool mouse2);

    void SendEnqueuedCommands();

    void KeyPress(uint8_t keycode);
    void KeyRelease(uint8_t keycode);
    void MouseMovement(int32_t mouse_x, int32_t mouse_y);
    void MouseButtons(bool mouse1, bool mouse2);

private:

    std::string hidDestinationAddress;

    bool is_initialized = false;
    // Connection to the net HID
    ClientTCPSocket mHIDSocket;

    int mSampleBlockSize;

    static const uint32_t MAGIC = 0x0BC12000;

    const uint32_t MESSAGE_SIZE_BYTES = 4 * 4;

    struct Message {
        uint32_t magic;
        uint32_t type;
        int32_t arg1;
        int32_t arg2;

        Message(uint32_t type, int32_t arg1, int32_t arg2) : magic(MAGIC), type(type), arg1(arg1), arg2(arg2) {}
    };
    enum MessageType {
        KeyboardPress = 1,
        KeyboardRelease = 2,
        MouseMove = 3,
        MouseButton = 4
    };

    void write_message(const Message& message, std::vector<char>& data);
    void SendSingle(const Message message);
    std::vector<Message> messages;
};

#endif
