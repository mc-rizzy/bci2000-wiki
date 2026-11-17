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
#include "JoystickLogger_macOS.h"

#include "BCIEvent.h"

#define MAXJOYSTICK 32768

Extension(JoystickLogger);

// **************************************************************************
// Function:   JoystickLogger
// Purpose:    This is the constructor for the JoystickLogger class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
JoystickLogger::JoystickLogger() : m_joystickenable(false)
{
    PublishEnabler("LogJoystick");
}

// **************************************************************************
// Function:   ~JoystickLogger
// Purpose:    This is the destructor for the JoystickLogger class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
JoystickLogger::~JoystickLogger()
{
}

// **************************************************************************
// Function:   Publish
// Purpose:    This function requests parameters by adding parameters to the
//             parameter list it also requests states by adding them to the
//             state list
// Returns:    N/A
// **************************************************************************
void JoystickLogger::Publish()
{
    if (OptionalParameter("LogJoystick") > 0)
    {
      BEGIN_PARAMETER_DEFINITIONS
          "Source:Log%20Input int LogJoystick= 1 0 0 1 "
               " // record joystick to states (boolean)",
      END_PARAMETER_DEFINITIONS

      BEGIN_EVENT_DEFINITIONS
          "JoystickXpos     16 0 0 0",
          "JoystickYpos     16 0 0 0",
          "JoystickZpos     16 0 0 0",
          "JoystickButtons1  1 0 0 0",
          "JoystickButtons2  1 0 0 0",
          "JoystickButtons3  1 0 0 0",
          "JoystickButtons4  1 0 0 0",
          "JoystickButtons9  1 0 0 0",
          "JoystickButtons10  1 0 0 0",
      END_EVENT_DEFINITIONS
    }
}

// **************************************************************************
// Function:   StartRun
// Purpose:    Starts a new Joystick thread at the beginning of a run
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void JoystickLogger::StartRun()
{
    if ( OptionalParameter( "LogJoystick" ) ) {
        mJoystickThread.StartIfNotRunning();
    }
}

// **************************************************************************
// Function:   StopRun
// Purpose:    Terminates the Joystick thread at the end of a run
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void JoystickLogger::StopRun()
{
    mJoystickThread.Terminate();
}

// **************************************************************************
// Function:   Halt
// Purpose:    Stops all asynchronous operation
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void JoystickLogger::Halt()
{
    StopRun();
}

static void OnInput( void* inC, IOReturn result, void* inS, IOHIDValueRef value ) {
	JoystickLogger* t = reinterpret_cast<JoystickLogger*>( inC );

	t->Joystick( IOHIDValueGetElement( value ), value );
}

void JoystickLogger::Joystick( IOHIDElementRef elem, IOHIDValueRef value ) {
	float axis = ( float )IOHIDValueGetIntegerValue( value );
	axis = axis / 255;
	axis = axis * MAXJOYSTICK;

	int usage = IOHIDElementGetUsage( elem );
	switch ( usage ) {
		case 48: // X Axis
			bcievent << "JoystickXpos " << axis;
			break;
		case 49: // Y
			bcievent << "JoystickYpos " << axis;
			break;
		case 50: // Z
			bcievent << "JoystickZpos " << axis;
			break;

		default:
			if ( ( usage > 0 && usage < 5 ) || ( usage > 8 && usage < 11 ) ) {
				bcievent << "JoystickButtons" << usage << " " << IOHIDValueGetIntegerValue( value );
			} else {
				bciout << IOHIDElementGetUsage( elem ) << ", " << IOHIDValueGetIntegerValue( value );
			}
			break;
	}
}


// **************************************************************************
// Function:   OnExecute
// Purpose:    This is the Joystick thread function
// Parameters: N/A
// Returns:    Always zero
// **************************************************************************
int JoystickLogger::JoystickThread::OnExecute()
{
	IOHIDManagerRef ioManager = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDManagerOptionNone );

	if ( CFGetTypeID( ioManager ) != IOHIDManagerGetTypeID() ) {
		std::cerr << "Wrong ID?" << std::endl;
	}

	static const unsigned kKeyCount = 2;

	int usagePage = kHIDPage_GenericDesktop;
	int usage = kHIDUsage_GD_Keyboard;

	CFStringRef keys[ kKeyCount ] = {
		CFSTR( kIOHIDDeviceUsagePageKey ),
		CFSTR( kIOHIDDeviceUsageKey ),
	};

	CFNumberRef values[ kKeyCount ] = {
		CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &usagePage ),
		CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &usage ),
	};

	CFDictionaryRef matchingDict = CFDictionaryCreate( kCFAllocatorDefault,
			( const void ** ) keys, ( const void ** ) values, kKeyCount,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );


	int usagePage1 = kHIDPage_GenericDesktop;
	int usage1 = kHIDUsage_GD_Joystick;

	CFStringRef keys1[ kKeyCount ] = {
		CFSTR( kIOHIDDeviceUsagePageKey ),
		CFSTR( kIOHIDDeviceUsageKey ),
	};

	CFNumberRef values1[ kKeyCount ] = {
		CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &usagePage1 ),
		CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &usage1 ),
	};

	CFDictionaryRef matchingDict1 = CFDictionaryCreate( kCFAllocatorDefault,
			( const void ** ) keys1, ( const void ** ) values1, kKeyCount,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

	CFDictionaryRef dicts[ 1 ] = { matchingDict1 };

	CFArrayRef cfdicts = CFArrayCreate( kCFAllocatorDefault, ( const void** ) ( ( CFDictionaryRef** )dicts ), 1, &kCFTypeArrayCallBacks );

	IOHIDManagerSetDeviceMatchingMultiple( ioManager, cfdicts  );

	for ( int i = 0; i < kKeyCount; i++ ) {
		CFRelease( keys[ i ] );
		CFRelease( values[ i ] );
	}
	for ( int i = 0; i < kKeyCount; i++ ) {
		CFRelease( keys1[ i ] );
		CFRelease( values1[ i ] );
	}
	CFRelease( matchingDict );
	CFRelease( matchingDict1 );

	IOHIDManagerRegisterInputValueCallback( ioManager, OnInput, this );

	IOHIDManagerOpen( ioManager, kIOHIDOptionsTypeNone );

	IOHIDManagerScheduleWithRunLoop( ioManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );

	while ( !Terminating() ) {
		CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
	}
	
	return 1;
}
