#include "BCIStream.h"
#include "NetworkHID.h"

#include <vector>

NetworkHID::NetworkHID(std::string addr, int sampleBlockSize) {
    hidDestinationAddress = addr;
    mSampleBlockSize = sampleBlockSize;
}

NetworkHID::~NetworkHID() {
    mHIDSocket.Close();
}

void NetworkHID::Initialize() {
    mHIDSocket.Close();
    mHIDSocket.Open(hidDestinationAddress);
    if (!mHIDSocket.IsOpen()) {
        bciout << "Could not open " << hidDestinationAddress;
        return;
    }
    is_initialized = true;
}

/**
 * This code assumes little endian byte order!!
 * Writes little endian to big endian in a byte vec
 */
void write_int_le(uint32_t n, std::vector<char>& vec) {
    vec.push_back(n >> 24 & 0xFF);
    vec.push_back(n >> 16 & 0xFF);
    vec.push_back(n >> 8 & 0xFF);
    vec.push_back(n & 0xFF);
}

int MAGIC = 0x0BC12000;

void NetworkHID::write_message(const Message& message, std::vector<char>& data) {
    if (!this->is_initialized) {
        bcierr << "NetworkHID's Initialize() method must be called before use!";
    }
    write_int_le(MAGIC, data);
    write_int_le(message.type, data);
    write_int_le(message.arg1, data);
    write_int_le(message.arg2, data);
}

void NetworkHID::EnqueueKeyPress(uint8_t keycode) {
    messages.emplace_back(KeyboardPress, keycode, 0);
}
void NetworkHID::EnqueueKeyRelease(uint8_t keycode) {
    messages.emplace_back(KeyboardRelease, keycode, 0);
}
void NetworkHID::EnqueueMouseMovement(int32_t mouse_x, int32_t mouse_y) {
    messages.emplace_back(MouseMove, mouse_x, mouse_y);
}
void NetworkHID::EnqueueMouseButtons(bool mouse1, bool mouse2) {
    messages.emplace_back(MouseButton, static_cast<int32_t>(mouse1), static_cast<int32_t>(mouse2));
}

void NetworkHID::SendEnqueuedCommands() {
    int n_messages = messages.size();
    std::vector<char> messages_data;
    messages_data.reserve(n_messages * MESSAGE_SIZE_BYTES);
    for (const Message& message : messages) {
        write_message(message, messages_data);
    }

    mHIDSocket.Write(messages_data.data(), n_messages * MESSAGE_SIZE_BYTES);
    messages.clear();
}

void NetworkHID::SendSingle(const Message message) {
    std::vector<char> messages_data;
    write_message(message, messages_data);
    mHIDSocket.Write(messages_data.data(), MESSAGE_SIZE_BYTES);
}
void NetworkHID::KeyPress(uint8_t keycode) {
    Message m = Message(KeyboardPress, keycode, 0);
    SendSingle(m);
}
void NetworkHID::KeyRelease(uint8_t keycode) {
    Message m = Message(KeyboardRelease, keycode, 0);
    SendSingle(m);
}
void NetworkHID::MouseMovement(int32_t mouse_x, int32_t mouse_y) {
    Message m = Message(MouseMove, mouse_x, mouse_y);
    SendSingle(m);
}

void NetworkHID::MouseButtons(bool mouse1, bool mouse2) {
    Message m = Message(MouseButton, static_cast<int32_t>(mouse1), static_cast<int32_t>(mouse2));
    SendSingle(m);
}


