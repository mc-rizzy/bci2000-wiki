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
#ifndef KEY_LOGGER_H
#define KEY_LOGGER_H

#include "Environment.h"
#include "Thread.h"
#include <vector>
#include <string>
#include <linux/input.h>
#include "scancodes.h"

class KeyLogger final : public EnvironmentExtension, public Thread
{
  public:
    enum
    {
        KeyboardBits = 8,

        LButtonBit = 0,
        RButtonBit = 1,

        LButtonMask = 1 << LButtonBit,
        RButtonMask = 1 << RButtonBit,
    };

    KeyLogger();
    virtual ~KeyLogger();
    void Publish() override;
    void AutoConfig() override;
    void Preflight() const override;
    void Initialize() override;
    void StartRun() override;
    void StopRun() override;
    void Halt() override;

  protected:
    int OnExecute() override;

  private:
    void OnKeyboardEvent(const struct input_event&);
    void OnMouseEvent(const struct input_event&);

    bool mLogKeyboard;
    bool mLogMouse;

    std::vector<std::string> mKeyboardFiles, mMouseFiles;

    uint16_t mTranslationTable[NUM_SCANCODES] = {0};

    int mMouseKeysPressed = 0;
    int mMouseX = 0, mMouseY = 0;
};

#endif // KEY_LOGGER_H
