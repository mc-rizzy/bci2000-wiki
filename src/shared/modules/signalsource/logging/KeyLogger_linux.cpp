////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: An extension that logs keyboard and mouse keypresses into states.
//   Useful for experiments that require tracking of user responses.
//   A "Keyboard" state contains the key's "virtual key code" as defined by
//   the Win32 API (i.e., it is translated from HID key codes to Win32 virtual
//   key codes), and additionally a bit which is set when a key is released.
//   In the "MouseKeys" state, bit 0 represents left and bit 1 represents the
//   right mouse button.
//   In the MousePosX and MousePosY states, mouse cursor position is stored in
//   device coordinates (i.e. coordinates that are in units of screen pixels)
//   with an additional offset of 32768 to cover negative coordinates.
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
////////////////////////////////////////////////////////////////////////////////
#include "KeyLogger_linux.h"

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <sys/stat.h>
#include <grp.h>

#include <vector>
#include <sstream>

#include "BCIEvent.h"
#include "BCIStream.h"
#include "FileUtils.h"
#include "Files.h"

Extension(KeyLogger)

static const int cMouseOffset = 1 << 15;

namespace {
// https://github.com/cxnturi0n/keylogger/
bool HasEventTypes(int fd, unsigned long evbit_to_check)
{
    unsigned long evbit = 0;

    /* Get the bit field of available event types. */
    ::ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);

    /* Check if EV_* set in $evbit_to_check are set. */
    return ((evbit & evbit_to_check) == evbit_to_check);
}

bool HasKeys(int fd)
{
    return HasEventTypes(fd, (1 << EV_KEY));
}

bool HasRelativeMovement(int fd)
{
    return HasEventTypes(fd, (1 << EV_REL));
}

bool HasAbsoluteMovement(int fd)
{
    return HasEventTypes(fd, (1 << EV_ABS));
}

bool HasSpecificKeys(int fd, int *keys, size_t num_keys)
{
    size_t nchar = KEY_MAX / 8 + 1;
    unsigned char bits[nchar];

    /* Get the bit fields of available keys. */
    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), &bits);

    for (size_t i = 0; i < num_keys; ++i)
    {
        int key = keys[i];
        if (!(bits[key / 8] & (1 << (key % 8))))
            return false; /* At least one key is not supported */
    }
    return true; /* All keys are supported. */
}

void NoAccessWarning(const std::string& path)
{
    std::ostringstream oss;
    int groupID = -1;
    struct stat st;
    if (!::stat(path.c_str(), &st)) {
        groupID = st.st_gid;
    }
    auto pGroup = ::getgrgid(groupID);
    if (pGroup) {
        oss << pGroup->gr_name;
    }
    else {
        oss << "<" << groupID << ">";
    }
    bciwarn << "KeyLogger: Cannot access " << path << " device, make sure you are in the \""
            << oss.str() << "\" group";
}

bool IsMouseDevice(const std::string& path)
{
    int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0 && errno == EACCES) {
        NoAccessWarning(path);
        return false;
    }
    if (fd >= 0) {
        bool isMouse = true;
        isMouse = isMouse && HasKeys(fd);
        isMouse = isMouse && (HasRelativeMovement(fd) || HasAbsoluteMovement(fd));
        int keys[] = {BTN_LEFT, BTN_RIGHT};
        isMouse = isMouse && HasSpecificKeys(fd, keys, sizeof(keys) / sizeof(*keys));
        ::close(fd);
        return isMouse;
    }
    return false;
}

bool IsKeyboardDevice(const std::string& path)
{
    int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0 && errno == EACCES) {
        NoAccessWarning(path);
        return false;
    }
    if (fd >= 0) {
        bool isKeyboard = true;
        isKeyboard = isKeyboard && HasKeys(fd);
        isKeyboard = isKeyboard && !HasRelativeMovement(fd);
        isKeyboard = isKeyboard && !HasAbsoluteMovement(fd);
        int keys[] = {KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_BACKSPACE, KEY_ENTER, KEY_0, KEY_1, KEY_2, KEY_ESC};
        isKeyboard = isKeyboard && HasSpecificKeys(fd, keys, sizeof(keys) / sizeof(*keys));
        ::close(fd);
        return isKeyboard;
    }
    return false;
}

std::vector<std::string> ListKeyboardDevices()
{
    const std::string path = "/dev/input/";
    std::vector<std::string> keyboardDevices;
    FileUtils::List devices;
    if (FileUtils::ListDirectory(path, devices)) {
        for (const auto& device : devices) {
            if (FileUtils::IsDirectory(device))
                continue;
            if (IsKeyboardDevice(path + device))
                keyboardDevices.push_back(path + device);
        }
    }
    return keyboardDevices;
}

std::vector<std::string> ListMouseDevices()
{
    const std::string path = "/dev/input/";
    std::vector<std::string> mouseDevices;
    FileUtils::List devices;
    if (FileUtils::ListDirectory(path, devices)) {
        for (const auto& device : devices) {
            if (FileUtils::IsDirectory(device))
                continue;
            if (IsMouseDevice(path + device))
                mouseDevices.push_back(path + device);
        }
    }
    return mouseDevices;
}

} // namespace

void KeyLogger::OnMouseEvent(const struct input_event& event)
{
    if (event.type == EV_KEY) {
        int mask = 0;
        switch (event.code) {
        case BTN_LEFT:
            mask = LButtonMask;
            break;
        case BTN_RIGHT:
            mask = RButtonMask;
        }
        if (event.value == 1) {
            // Key pressed
            mMouseKeysPressed |= mask;
        }
        else if (event.value == 0) {
            // Key released
            mMouseKeysPressed &= (~mask);
        }
        bcievent << "MouseKeys " << mMouseKeysPressed;
    }
    else if (event.type == EV_ABS) {
        if (event.code == ABS_X) {
            mMouseX = event.value;
            bcievent << "MousePosX " << mMouseX + cMouseOffset;
        }
        else if (event.code == ABS_Y) {
            mMouseY = -event.value;
            bcievent << "MousePosY " << mMouseY + cMouseOffset;
        }
    }
    else if (event.type == EV_REL) {
        // On Wayland, we cannot get the global pointer position
        // for security reasons.
        // Instead, we integrate up relative movement which is
        // not entirely accurate due to pointer acceleration in the
        // compositor.
        if (event.code == REL_X) {
            mMouseX += event.value;
            bcievent << "MousePosX " << mMouseX + cMouseOffset;
        }
        else if (event.code == REL_Y) {
            mMouseY -= event.value;
            bcievent << "MousePosY " << mMouseY + cMouseOffset;
        }
    }
}

void KeyLogger::OnKeyboardEvent(const struct input_event& event)
{
    if (event.type == EV_KEY) {
        int scancode = event.code;
        if (event.value == 1) {
            bcidbg << "Scancode: " << scancode << ", Mapped: " << mTranslationTable[scancode];
            bcievent << "KeyDown " << mTranslationTable[scancode] << " 0";
        } else if (event.value == 0) {
            bcievent << "KeyUp " << mTranslationTable[scancode] << " 0";
        }
    }
}

KeyLogger::KeyLogger() : mLogKeyboard( false ), mLogMouse( false )
{
    PublishEnabler("LogKeyboard");
	PublishEnabler("LogMouse");

    mTranslationTable[SCANCODE_ESCAPE] = KeyEscape;
    mTranslationTable[SCANCODE_F1] = KeyF1;
    mTranslationTable[SCANCODE_F2] = KeyF2;
    mTranslationTable[SCANCODE_F3] = KeyF3;
    mTranslationTable[SCANCODE_F4] = KeyF4;
    mTranslationTable[SCANCODE_F5] = KeyF5;
    mTranslationTable[SCANCODE_F6] = KeyF6;
    mTranslationTable[SCANCODE_F7] = KeyF7;
    mTranslationTable[SCANCODE_F8] = KeyF8;
    mTranslationTable[SCANCODE_F9] = KeyF9;
    mTranslationTable[SCANCODE_F10] = KeyF10;
    mTranslationTable[SCANCODE_F11] = KeyF11;
    mTranslationTable[SCANCODE_F12] = KeyF12;
    mTranslationTable[SCANCODE_F13] = KeyF13;
    mTranslationTable[SCANCODE_F14] = KeyF14;
    mTranslationTable[SCANCODE_F15] = KeyF15;
    mTranslationTable[SCANCODE_F16] = KeyF16;
    mTranslationTable[SCANCODE_F17] = KeyF17;
    mTranslationTable[SCANCODE_F18] = KeyF18;
    mTranslationTable[SCANCODE_F19] = KeyF19;
    mTranslationTable[SCANCODE_PRINTSCREEN] = KeyPrint;
    mTranslationTable[SCANCODE_SCROLLLOCK] = KeyScrollLock;
    mTranslationTable[SCANCODE_PAUSE] = KeyBreak;
    mTranslationTable[SCANCODE_SPACE] = KeySpace;
    mTranslationTable[SCANCODE_COMMA] = KeyComma;
    mTranslationTable[SCANCODE_MINUS] = KeyMinus;
    mTranslationTable[SCANCODE_PERIOD] = KeyPeriod;
    mTranslationTable[SCANCODE_APOSTROPHE] = KeyApostrophe;
    mTranslationTable[SCANCODE_0] = Key0;
    mTranslationTable[SCANCODE_1] = Key1;
    mTranslationTable[SCANCODE_2] = Key2;
    mTranslationTable[SCANCODE_3] = Key3;
    mTranslationTable[SCANCODE_4] = Key4;
    mTranslationTable[SCANCODE_5] = Key5;
    mTranslationTable[SCANCODE_6] = Key6;
    mTranslationTable[SCANCODE_7] = Key7;
    mTranslationTable[SCANCODE_8] = Key8;
    mTranslationTable[SCANCODE_9] = Key9;
    mTranslationTable[SCANCODE_SEMICOLON] = KeySemicolon;
    mTranslationTable[SCANCODE_EQUALS] = KeyEqual;
    mTranslationTable[SCANCODE_A] = 'A';
    mTranslationTable[SCANCODE_B] = 'B';
    mTranslationTable[SCANCODE_C] = 'C';
    mTranslationTable[SCANCODE_D] = 'D';
    mTranslationTable[SCANCODE_E] = 'E';
    mTranslationTable[SCANCODE_F] = 'F';
    mTranslationTable[SCANCODE_G] = 'G';
    mTranslationTable[SCANCODE_H] = 'H';
    mTranslationTable[SCANCODE_I] = 'I';
    mTranslationTable[SCANCODE_J] = 'J';
    mTranslationTable[SCANCODE_K] = 'K';
    mTranslationTable[SCANCODE_L] = 'L';
    mTranslationTable[SCANCODE_M] = 'M';
    mTranslationTable[SCANCODE_N] = 'N';
    mTranslationTable[SCANCODE_O] = 'O';
    mTranslationTable[SCANCODE_P] = 'P';
    mTranslationTable[SCANCODE_Q] = 'Q';
    mTranslationTable[SCANCODE_R] = 'R';
    mTranslationTable[SCANCODE_S] = 'S';
    mTranslationTable[SCANCODE_T] = 'T';
    mTranslationTable[SCANCODE_U] = 'U';
    mTranslationTable[SCANCODE_V] = 'V';
    mTranslationTable[SCANCODE_W] = 'W';
    mTranslationTable[SCANCODE_X] = 'X';
    mTranslationTable[SCANCODE_Y] = 'Y';
    mTranslationTable[SCANCODE_Z] = 'Z';
    mTranslationTable[SCANCODE_LEFTBRACKET] = KeyBracketLeft;
    mTranslationTable[SCANCODE_BACKSLASH] = KeyBackslash;
    mTranslationTable[SCANCODE_RIGHTBRACKET] = KeyBracketRight;
    mTranslationTable[SCANCODE_GRAVE] = KeyGrave;
    mTranslationTable[SCANCODE_LEFT] = KeyLeft;
    mTranslationTable[SCANCODE_RIGHT] = KeyRight;
    mTranslationTable[SCANCODE_UP] = KeyUp;
    mTranslationTable[SCANCODE_DOWN] = KeyDown;
    mTranslationTable[SCANCODE_INSERT] = KeyInsert;
    mTranslationTable[SCANCODE_HOME] = KeyHome;
    mTranslationTable[SCANCODE_DELETE] = KeyDelete;
    mTranslationTable[SCANCODE_END] = KeyEnd;
    mTranslationTable[SCANCODE_PAGEUP] = KeyPageUp;
    mTranslationTable[SCANCODE_PAGEDOWN] = KeyPageDown;
    mTranslationTable[SCANCODE_NUMLOCKCLEAR] = KeyNumLock;
    mTranslationTable[SCANCODE_KP_EQUALS] = KeyKpEqual;
    mTranslationTable[SCANCODE_KP_DIVIDE] = KeyKpDivide;
    mTranslationTable[SCANCODE_KP_MULTIPLY] = KeyKpMultiply;
    mTranslationTable[SCANCODE_KP_MINUS] = KeyKpSubtract;
    mTranslationTable[SCANCODE_KP_PLUS] = KeyKpAdd;
    mTranslationTable[SCANCODE_KP_ENTER] = KeyKpEnter;
    mTranslationTable[SCANCODE_KP_0] = KeyKpInsert;
    mTranslationTable[SCANCODE_KP_1] = KeyKpEnd;
    mTranslationTable[SCANCODE_KP_2] = KeyKpDown;
    mTranslationTable[SCANCODE_KP_3] = KeyKpPageDown;
    mTranslationTable[SCANCODE_KP_4] = KeyKpLeft;
    mTranslationTable[SCANCODE_KP_5] = KeyKpBegin;
    mTranslationTable[SCANCODE_KP_6] = KeyKpRight;
    mTranslationTable[SCANCODE_KP_7] = KeyKpHome;
    mTranslationTable[SCANCODE_KP_8] = KeyKpUp;
    mTranslationTable[SCANCODE_KP_9] = KeyKpPageUp;
    mTranslationTable[SCANCODE_KP_PERIOD] = KeyKpDelete;
    mTranslationTable[SCANCODE_BACKSPACE] = KeyBackSpace;
    mTranslationTable[SCANCODE_TAB] = KeyTab;
    mTranslationTable[SCANCODE_RETURN] = KeyReturn;
    mTranslationTable[SCANCODE_CAPSLOCK] = KeyCapsLock;
    mTranslationTable[SCANCODE_LSHIFT] = KeyShiftL;
    mTranslationTable[SCANCODE_LCTRL] = KeyCtrlL;
    mTranslationTable[SCANCODE_LGUI] = KeySuperL;
    mTranslationTable[SCANCODE_LALT] = KeyAltL;
    mTranslationTable[SCANCODE_RALT] = KeyAltR;
    mTranslationTable[SCANCODE_RGUI] = KeySuperR;
    mTranslationTable[SCANCODE_RCTRL] = KeyCtrlR;
    mTranslationTable[SCANCODE_RSHIFT] = KeyShiftR;
}


int KeyLogger::OnExecute()
{
    struct TaggedFile : File
    {
        enum { unknown, keyboard, mouse } tag = unknown;
    };
    std::vector<std::unique_ptr<TaggedFile>> devices;
    for (const auto& file : mKeyboardFiles) {
        devices.push_back(std::make_unique<TaggedFile>());
        devices.back()->tag = TaggedFile::keyboard;
        devices.back()->Open(file);
    }
    for (const auto& file : mMouseFiles) {
        devices.push_back(std::make_unique<TaggedFile>());
        devices.back()->tag = TaggedFile::mouse;
        devices.back()->Open(file);
    }
    Waitables deviceSet;
    for (auto& device : devices) {
        deviceSet.Add(*device);
    }
    const Waitable* pDevice = nullptr;
    while (pDevice = deviceSet.Wait()) {
        for (auto& device : devices) {
            if (pDevice == &*device) {
                struct input_event event = {0};
                if (device->Read(&event, sizeof(event)) == sizeof(event)) {
                    switch (device->tag) {
                    case TaggedFile::keyboard:
                        OnKeyboardEvent(event);
                        break;
                    case TaggedFile::mouse:
                        OnMouseEvent(event);
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

KeyLogger::~KeyLogger()
{
    Halt();
}

void KeyLogger::Publish()
{
    if ((OptionalParameter("LogKeyboard") != 0))
    {
      BEGIN_PARAMETER_DEFINITIONS
          "Source:Log%20Input int /LogKeyboard= 1 0 0 1 "
              " // record keyboard to states (boolean)",
      END_PARAMETER_DEFINITIONS

      BEGIN_EVENT_DEFINITIONS
          "KeyDown 8 0 0 0",
          "KeyUp   8 0 0 0",
      END_EVENT_DEFINITIONS
    }

    if ((OptionalParameter("LogMouse", 0) != 0))
    {
      BEGIN_PARAMETER_DEFINITIONS
          "Source:Log%20Input int /LogMouse= 1 0 0 1 "
              " // record mouse to states (boolean)",
          "Source:Log%20Input list DesktopRect= 1 auto % % "
             " // desktop rect: left, top, width, height (noedit)",
      END_PARAMETER_DEFINITIONS

      BEGIN_EVENT_DEFINITIONS
          "MouseKeys  2 0 0 0",
          "MousePosX 16 0 0 0",
          "MousePosY 16 0 0 0",
      END_EVENT_DEFINITIONS
    }

    if (OptionalParameter("LogKeyboard")) {
        ListKeyboardDevices();
    }
}

void KeyLogger::AutoConfig()
{
    // On Wayland, global desktop properties are not accessible for security reasons.
}

void KeyLogger::Preflight() const
{
    OptionalParameter("LogKeyboard");
    OptionalParameter("LogMouse");
}

void KeyLogger::StopRun()
{
    Halt();
}

void KeyLogger::Initialize()
{
    Halt();
    mLogKeyboard = OptionalParameter("LogKeyboard");
    mLogMouse = OptionalParameter("LogMouse");

    std::ostringstream oss;

    mKeyboardFiles.clear();
    if (mLogKeyboard) {
        mKeyboardFiles = ListKeyboardDevices();
        if (mKeyboardFiles.empty()) {
            bcierr << "Could not find any keyboard device";
        }
        else {
            oss << "Keyboard device(s):\n";
            for (const auto& file : mKeyboardFiles)
               oss << "\t" << file << "\n";
        }
    }

    mMouseFiles.clear();
    if (mLogMouse) {
        mMouseFiles = ListMouseDevices();
        if (mMouseFiles.empty()) {
            bcierr << "Could not find any mouse device";
        }
        else {
            oss << "Mouse device(s):\n";
            for (const auto& file : mMouseFiles)
               oss << "\t" << file << "\n";
        }
    }
    if (!oss.str().empty()) {
        bciout << "Using input devices:\n" << oss.str();
    }
}

void KeyLogger::StartRun()
{
    if (mLogKeyboard || mLogMouse)
        Start();
}

void KeyLogger::Halt()
{
	TerminateAndWait();
}
