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
#include "KeyLogger_macOS.h"

#include "BCIEvent.h"
#include "BCIStream.h"

#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/hid/IOHIDManager.h>
#import <IOKit/hid/IOHIDUsageTables.h>
#import <IOKit/hidsystem/IOHIDLib.h>
#import <CoreGraphics/CoreGraphics.h>

#include "scancodes.h"

Extension(KeyLogger);


static const int cMouseOffset = 1 << 15;

static void OnInput( void* inC, IOReturn result, void* inS, IOHIDValueRef value )
{
	KeyLogger* t = ( KeyLogger* )inC;
	t->Key( IOHIDValueGetElement( value ), value, IOHIDValueGetIntegerValue( value ) );
}

CGEventRef MouseCallback( CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* user )
{
	KeyLogger* t = reinterpret_cast<KeyLogger*>( user );
	t->Mouse( event, type );
	return event;
}

void KeyLogger::Mouse( CGEventRef event, CGEventType type )
{
	if ( type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput ) {
		CGEventTapEnable( eventTap, true );
	} else if ( type == kCGEventMouseMoved ){

		CGPoint point = CGEventGetLocation( event );
		if ( mouseX != point.x || mouseY != point.y ) {
			mouseX = point.x;
			mouseY = point.y;
			bcievent << "MousePosX " << ( int ) point.x + cMouseOffset;
			bcievent << "MousePosY " << ( int ) point.y + cMouseOffset;
		}

	} else {
		if ( type == kCGEventLeftMouseDown )
			mouseKeysPressed = mouseKeysPressed | LButtonMask;
		if ( type == kCGEventLeftMouseUp )
			mouseKeysPressed = mouseKeysPressed & ( ~LButtonMask );
		if ( type == kCGEventRightMouseDown )
			mouseKeysPressed = mouseKeysPressed | RButtonMask;
		if ( type == kCGEventRightMouseUp )
			mouseKeysPressed = mouseKeysPressed & ( ~RButtonMask );

		bcievent << "MouseKeys " << mouseKeysPressed;
	}

}

void KeyLogger::Key( IOHIDElementRef elem, IOHIDValueRef value, int pressed )
{
	if ( IOHIDElementGetUsagePage( elem ) == 7 ) { // Keyboard
		int scancode = IOHIDElementGetUsage( elem );
		if ( scancode < 2 || scancode > 255 )
			return;
		reset = true;
		if ( pressed ) {
			bcidbg << "Scancode: " << ( int )scancode << ", Mapped: " << ( int )dialect[ scancode ];
			bcievent << "KeyDown " << ( int )dialect[ scancode ] << " 0";
		} else {
			bcievent << "KeyUp " << ( int )dialect[ scancode ] << " 0";
		}
	} else {
		bciwarn << "Unknown Device Usage: " << IOHIDElementGetUsagePage( elem );
		bcidbg << IOHIDElementGetUsage( elem ) << ", " << IOHIDValueGetIntegerValue( value ) << ", " << pressed;
	}
}

KeyLogger::KeyLogger() : mLogKeyboard( false ), mLogMouse( false )
{
	PublishEnabler("LogKeyboard");
	PublishEnabler("LogMouse");

	dialect[kHIDUsage_KeyboardEscape] = KeyEscape;
	dialect[kHIDUsage_KeyboardF1] = KeyF1;
	dialect[kHIDUsage_KeyboardF2] = KeyF2;
	dialect[kHIDUsage_KeyboardF3] = KeyF3;
	dialect[kHIDUsage_KeyboardF4] = KeyF4;
	dialect[kHIDUsage_KeyboardF5] = KeyF5;
	dialect[kHIDUsage_KeyboardF6] = KeyF6;
	dialect[kHIDUsage_KeyboardF7] = KeyF7;
	dialect[kHIDUsage_KeyboardF8] = KeyF8;
	dialect[kHIDUsage_KeyboardF9] = KeyF9;
	dialect[kHIDUsage_KeyboardF10] = KeyF10;
	dialect[kHIDUsage_KeyboardF11] = KeyF11;
	dialect[kHIDUsage_KeyboardF12] = KeyF12;
	dialect[kHIDUsage_KeyboardF13] = KeyF13;
	dialect[kHIDUsage_KeyboardF14] = KeyF14;
	dialect[kHIDUsage_KeyboardF15] = KeyF15;
	dialect[kHIDUsage_KeyboardF16] = KeyF16;
	dialect[kHIDUsage_KeyboardF17] = KeyF17;
	dialect[kHIDUsage_KeyboardF18] = KeyF18;
	dialect[kHIDUsage_KeyboardF19] = KeyF19;
	dialect[kHIDUsage_KeyboardPrintScreen] = KeyPrint;
	dialect[kHIDUsage_KeyboardScrollLock] = KeyScrollLock;
	dialect[kHIDUsage_KeyboardPause] = KeyBreak;
	dialect[kHIDUsage_KeyboardSpacebar] = KeySpace;
	dialect[kHIDUsage_KeyboardComma] = KeyComma;
	dialect[kHIDUsage_KeyboardHyphen] = KeyMinus;
	dialect[kHIDUsage_KeyboardPeriod] = KeyPeriod;
	dialect[kHIDUsage_KeyboardQuote] = KeyApostrophe;
	dialect[kHIDUsage_Keyboard0] = Key0;
	dialect[kHIDUsage_Keyboard1] = Key1;
	dialect[kHIDUsage_Keyboard2] = Key2;
	dialect[kHIDUsage_Keyboard3] = Key3;
	dialect[kHIDUsage_Keyboard4] = Key4;
	dialect[kHIDUsage_Keyboard5] = Key5;
	dialect[kHIDUsage_Keyboard6] = Key6;
	dialect[kHIDUsage_Keyboard7] = Key7;
	dialect[kHIDUsage_Keyboard8] = Key8;
	dialect[kHIDUsage_Keyboard9] = Key9;
	dialect[kHIDUsage_KeyboardSemicolon] = KeySemicolon;
	dialect[kHIDUsage_KeyboardEqualSign] = KeyEqual;
	dialect[kHIDUsage_KeyboardA] = 'A';
	dialect[kHIDUsage_KeyboardB] = 'B';
	dialect[kHIDUsage_KeyboardC] = 'C';
	dialect[kHIDUsage_KeyboardD] = 'D';
	dialect[kHIDUsage_KeyboardE] = 'E';
	dialect[kHIDUsage_KeyboardF] = 'F';
	dialect[kHIDUsage_KeyboardG] = 'G';
	dialect[kHIDUsage_KeyboardH] = 'H';
	dialect[kHIDUsage_KeyboardI] = 'I';
	dialect[kHIDUsage_KeyboardJ] = 'J';
	dialect[kHIDUsage_KeyboardK] = 'K';
	dialect[kHIDUsage_KeyboardL] = 'L';
	dialect[kHIDUsage_KeyboardM] = 'M';
	dialect[kHIDUsage_KeyboardN] = 'N';
	dialect[kHIDUsage_KeyboardO] = 'O';
	dialect[kHIDUsage_KeyboardP] = 'P';
	dialect[kHIDUsage_KeyboardQ] = 'Q';
	dialect[kHIDUsage_KeyboardR] = 'R';
	dialect[kHIDUsage_KeyboardS] = 'S';
	dialect[kHIDUsage_KeyboardT] = 'T';
	dialect[kHIDUsage_KeyboardU] = 'U';
	dialect[kHIDUsage_KeyboardV] = 'V';
	dialect[kHIDUsage_KeyboardW] = 'W';
	dialect[kHIDUsage_KeyboardX] = 'X';
	dialect[kHIDUsage_KeyboardY] = 'Y';
	dialect[kHIDUsage_KeyboardZ] = 'Z';
	dialect[kHIDUsage_KeyboardOpenBracket] = KeyBracketLeft;
	dialect[kHIDUsage_KeyboardBackslash] = KeyBackslash;
	dialect[kHIDUsage_KeyboardCloseBracket] = KeyBracketRight;
	dialect[kHIDUsage_KeyboardGraveAccentAndTilde] = KeyGrave;
	dialect[kHIDUsage_KeyboardLeftArrow] = KeyLeft;
	dialect[kHIDUsage_KeyboardRightArrow] = KeyRight;
	dialect[kHIDUsage_KeyboardUpArrow] = KeyUp;
	dialect[kHIDUsage_KeyboardDownArrow] = KeyDown;
	dialect[kHIDUsage_KeyboardInsert] = KeyInsert;
	dialect[kHIDUsage_KeyboardHome] = KeyHome;
	dialect[kHIDUsage_KeyboardDeleteForward] = KeyDelete;
	dialect[kHIDUsage_KeyboardEnd] = KeyEnd;
	dialect[kHIDUsage_KeyboardPageUp] = KeyPageUp;
	dialect[kHIDUsage_KeyboardPageDown] = KeyPageDown;
	dialect[kHIDUsage_KeypadNumLock] = KeyNumLock;
	dialect[kHIDUsage_KeypadEqualSign] = KeyKpEqual;
	dialect[kHIDUsage_KeypadSlash] = KeyKpDivide;
	dialect[kHIDUsage_KeypadAsterisk] = KeyKpMultiply;
	dialect[kHIDUsage_KeypadHyphen] = KeyKpSubtract;
	dialect[kHIDUsage_KeypadPlus] = KeyKpAdd;
	dialect[kHIDUsage_KeypadEnter] = KeyKpEnter;
	dialect[kHIDUsage_Keypad0] = KeyKpInsert;
	dialect[kHIDUsage_Keypad1] = KeyKpEnd;
	dialect[kHIDUsage_Keypad2] = KeyKpDown;
	dialect[kHIDUsage_Keypad3] = KeyKpPageDown;
	dialect[kHIDUsage_Keypad4] = KeyKpLeft;
	dialect[kHIDUsage_Keypad5] = KeyKpBegin;
	dialect[kHIDUsage_Keypad6] = KeyKpRight;
	dialect[kHIDUsage_Keypad7] = KeyKpHome;
	dialect[kHIDUsage_Keypad8] = KeyKpUp;
	dialect[kHIDUsage_Keypad9] = KeyKpPageUp;
	dialect[kHIDUsage_KeypadPeriod] = KeyKpDelete;
	dialect[kHIDUsage_KeyboardDeleteOrBackspace] = KeyBackSpace;
	dialect[kHIDUsage_KeyboardTab] = KeyTab;
	dialect[kHIDUsage_KeyboardReturnOrEnter] = KeyReturn;
	dialect[kHIDUsage_KeyboardCapsLock] = KeyCapsLock;
	dialect[kHIDUsage_KeyboardLeftShift] = KeyShiftL;
	dialect[kHIDUsage_KeyboardLeftControl] = KeyCtrlL;
	dialect[kHIDUsage_KeyboardLeftGUI] = KeySuperL;
	dialect[kHIDUsage_KeyboardLeftAlt] = KeyAltL;
	dialect[kHIDUsage_KeyboardRightAlt] = KeyAltR;
	dialect[kHIDUsage_KeyboardRightGUI] = KeySuperR;
	dialect[kHIDUsage_KeyboardRightControl] = KeyCtrlR;
	dialect[kHIDUsage_KeyboardRightShift] = KeyShiftR;
	//dialect[kHIDUsage_KeyboardNonUSPound] = KeyNumbersign;
}


int KeyLogger::OnExecute()
{
    IOHIDManagerRef ioManager = nullptr;

    if ( mLogKeyboard ) {

        ioManager = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDManagerOptionNone );

		if ( CFGetTypeID( ioManager ) != IOHIDManagerGetTypeID() ) {
            bcierr << "Wrong ID?" << std::endl;
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

		CFDictionaryRef dicts[ 2 ] = { matchingDict, matchingDict1 };

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
	}

    CFMachPortRef eventTap = nullptr;
	if ( mLogMouse ) {

		CGEventMask eventMask = CGEventMaskBit( kCGEventLeftMouseDown ) |
								CGEventMaskBit( kCGEventLeftMouseUp )   |
								CGEventMaskBit( kCGEventRightMouseDown )|
								CGEventMaskBit( kCGEventRightMouseUp )  |
								CGEventMaskBit( kCGEventMouseMoved );


        eventTap = CGEventTapCreate( kCGSessionEventTap,
				kCGHeadInsertEventTap,
				kCGEventTapOptionListenOnly,
				eventMask,
				MouseCallback,
				this );

		if ( !eventTap ) {
			bcierr << "Error Instantiating Mouse Tap";
			return -1;
		}

		CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource( kCFAllocatorDefault, eventTap, 0 );
		CFRunLoopAddSource( CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes );

		CGEventTapEnable( eventTap, true );

	}

	while ( !Terminating() ) {
		CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
	}

    if (eventTap)
        CFRelease(eventTap);

    if (ioManager)
        CFRelease(ioManager);

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

    if (OptionalParameter("LogKeyboard") || OptionalParameter("LogMouse")) {

        if (!IOHIDRequestAccess(kIOHIDRequestTypeListenEvent)){
            bciwarn << "Cannot access keyboard/mouse input -- check your system settings:\n"
                       "\"Privacy and security\"->\"Input Monitoring\"";
        }

    }
}

void KeyLogger::AutoConfig()
{
    if ((OptionalParameter("LogMouse", 0) != 0))
    {
        int left = 0, top = 0, width = 0, height = 0;
        
        CGDirectDisplayID displays[16];
        uint32_t displayCount = sizeof(displays)/sizeof(*displays);
        CGError err = ::CGGetActiveDisplayList(displayCount, displays, &displayCount);
        if (err != kCGErrorSuccess)
            return;
        
        // calculate the enclosing rectangle of all monitors
        int right = 0, bottom = 0;
        
        for (uint32_t i = 0; i < displayCount; ++i)
        {
            CGRect rect = ::CGDisplayBounds(displays[i]);
            if (rect.origin.x < left)
                left = rect.origin.x;
            if (rect.origin.y < top)
                top = rect.origin.y;
            int r = rect.origin.x + rect.size.width;
            if (r > right)
                right = r;
            int b = rect.origin.y + rect.size.height;
            if (b > bottom)
                bottom = b;
        }
        width = right - left;
        height = bottom - top;
        
        MutableParamRef DesktopRect = Parameter("DesktopRect");
        DesktopRect->SetNumValues(4);
        DesktopRect(0) = left;
        DesktopRect(1) = top;
        DesktopRect(2) = width;
        DesktopRect(3) = height;
    }
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
    mLogKeyboard = (OptionalParameter("LogKeyboard") != 0);
    mLogMouse = (OptionalParameter("LogMouse") != 0);
}

void KeyLogger::StartRun() {

	Start();
}

void KeyLogger::Halt()
{
	TerminateAndWait();
}
