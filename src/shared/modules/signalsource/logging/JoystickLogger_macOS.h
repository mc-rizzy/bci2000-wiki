/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: pbrunner@wadsworth.org, Max Marcus
// Description: The joystick filter supports joysticks with up to 3 axes
//   and 4 buttons that are interfaced with windows via the USB port.
//
// Parameterization:
//   Joystick logging is enabled from the command line adding
//   --LogJoystick=1
//   as a command line option.
//
// State Variables:
//   Each position state is ranging from -16348 to +16347 with a resting
//   position at 0. Each button state is either 1 when pressed or 0 when
//   not pressed.
//   JoystickXpos
//   JoystickYpos
//   JoystickZpos
//   JoystickButtons1
//   JoystickButtons2
//   JoystickButtons3
//   JoystickButtons4
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
/////////////////////////////////////////////////////////////////////////////
#ifndef JOYSTICK_LOGGER_H
#define JOYSTICK_LOGGER_H

#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/hid/IOHIDManager.h>
#import <IOKit/hid/IOHIDUsageTables.h>

#include "Environment.h"
#include "Thread.h"

class JoystickLogger : public EnvironmentExtension
{
  public:
    JoystickLogger();
    ~JoystickLogger();
    void Publish() override;
    void Preflight() const override {}
    void Initialize() override {}
    void StartRun() override;
    void StopRun() override;
    void Halt() override;

    void Joystick(IOHIDElementRef, IOHIDValueRef);

private:
    bool m_joystickenable;

    struct JoystickThread : public Thread
    {
        int OnExecute() override;
    } mJoystickThread;
};

#endif // JOYSTICK_LOGGER_H
