////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Max Marcus
// Description: A filter that logs keyboard and mouse keypresses into states.
//   Useful for experiments that require tracking of user responses.
//   A "Keyboard" state contains the key's "virtual key code" as defined by
//   the Win32 API (i.e., it is translated from macOS key codes to Win32 virtual
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
#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/hid/IOHIDManager.h>
#import <IOKit/hid/IOHIDUsageTables.h>
#import <CoreGraphics/CoreGraphics.h>

class KeyLogger : public EnvironmentExtension, public Thread
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
    bool mLogKeyboard;
    bool mLogMouse;

  public:
    void Key(IOHIDElementRef, IOHIDValueRef, int);
    void Mouse(CGEventRef, CGEventType);

  private:
	CFMachPortRef eventTap;
	inline static uint8_t dialect[ 256 ] = { 0 };
	int mouseKeysPressed;
	int mouseX;
	int mouseY;
	bool reset;
};

#endif // KEY_LOGGER_H
