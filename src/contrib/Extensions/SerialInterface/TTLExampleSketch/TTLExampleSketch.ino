// $Id: TTLExampleSketch.ino 9365 2026-05-11 22:36:42Z jhill $

#include "Keyhole.h" // install the Keyhole library using the library manager of your Arduino IDE

/*

 * To receive, over the serial port, a report of all exposed variables, send the string "?"
 * via the serial port monitor  (or "?\n" if using your own program - all commands will need
 * to be newline-terminated).
 * 
 * To turn on the LED and set the voltage high on the output pin, send::
 * 
 *     output = 1
 * 
 * To turn off the LED and set the voltage low on the output pin, send::
 * 
 *     output = 0
 * 
 * You can also change the values of `TTLInputPin` and `TTLOutputPin` on-the-fly in the
 * same way (but bear in mind that settings reconfigured this way will not be remembered
 * once power is lost).
 * 
 * When the voltage goes high on the *input* pin, the microcontroller will send the
 * string `TTLInput 1\n` over the serial port. When it goes low, it will send
 * `TTLInput 0\n`.
 * 
 * If you send the command `publish\n` then the sketch will reply with BCI2000 parameter
 * and/or event definitions.

*/

#ifndef   LED_BUILTIN
# define  LED_BUILTIN 2  // works for ItsyBitsy, whose headers for some reason don't define this
#endif // LED_BUILTIN

int gInputPin  = 16; // default value (on the Pico, GP16 is the bottom right corner)
int gOutputPin = 15; // default value (on the Pico, GP15 is the bottom left corner)

int gInput  = 0;
int gOutput = 0;
int gMute = 0;

#if __has_include("_SyncGenie.h") // only exists if compiled with the genie script using the -r flag. Otherwise, nothing to see here.
# include "_SyncGenie.h"
  SyncGenie genie;
#endif

void setup()
{
# ifdef SYNCGENIE_BOARD_REVISION
    genie.begin("SerialInterface\nTTLExampleSketch"); // if we're using a SyncGenie, it's nice to be able to see which sketch is loaded (to enable this, compile with 'genie -r.')
    gInputPin = genie.BUTTON;
    gOutputPin = genie.D0_OUT;
#   define INPUT_PIN_MODE ( ( gInputPin == genie.BUTTON ) ? INPUT_PULLUP : INPUT )
# else
    Serial.begin(115200); // note that Teensy, Pico and others ignore this rate and use faster USB rates anyway;  but for actual Arduinos: 115200 is the max BAUD rate supported by the Arduino IDE's Serial Monitor 
#   define INPUT_PIN_MODE INPUT
# endif
# define INPUT_RESTING_VALUE ( INPUT_PIN_MODE == INPUT_PULLUP ? HIGH : LOW )
  
  if( gInputPin >= 0 )
    pinMode( gInputPin, INPUT_PIN_MODE );
  
  if( gOutputPin >= 0 )
  {
    pinMode( gOutputPin, OUTPUT );
    digitalWrite( gOutputPin, LOW );
  }

  // Flicker the LED for a second to say hello:
  Keyhole::flicker( LED_BUILTIN, 50, 50, 1000 );
}

void loop()
{
  int newInput = 0;
  if( gInputPin >= 0 ) newInput = ( int )( digitalRead( gInputPin ) != INPUT_RESTING_VALUE );
  if( newInput != gInput )
  {
    static unsigned long sPulseStartTimeInMillis = 0;
    unsigned long pulseDurationInMillis;
    if( newInput ) sPulseStartTimeInMillis = millis();
    else pulseDurationInMillis = millis() - sPulseStartTimeInMillis;
    if( !gMute )                    // without the "mute" mechanism, you get a LOT of these messages
    {                               //  if the pin voltage happens to be floating, and that might disrupt
      Serial.print( "TTLInput " );  //  other communications (only a problem with some microcontrollers)
      Serial.println( newInput );
      if( !newInput )
      {
        Serial.print( "PulseDurationMsec " );
        Serial.print( pulseDurationInMillis );
        Serial.println( " 0" ); // optional third field of Event Descriptor (must be 0, if supplied) - makes the Event a transient with a duration of 1 sample
      }
      Serial.flush();
    }
    gInput = newInput;
  }
  
  
  KEYHOLE keyhole( Serial );
  if( keyhole.begin() )
  {
    keyhole.variable( "mute", gMute );  // "mute=0\n" and "mute=1\n" could be used as start and stop commands

    keyhole.variable( "input", gInput, VARIABLE_READ_ONLY );
    
    if( keyhole.variable( "output", gOutput ) )
    {
      if( gOutputPin >= 0 ) digitalWrite( gOutputPin, gOutput ? HIGH : LOW );
      digitalWrite( LED_BUILTIN, gOutput ? HIGH : LOW ); // LED_BUILTIN will follow your output in addition to whatever output pin you configured 
    }
    
    if( keyhole.variable( "TTLInputPin",  gInputPin  ) && gInputPin  >= 0 )
    { // Since we have declared this as a parameter, BCI2000 will set its value during Initialize()
      pinMode( gInputPin, INPUT_PIN_MODE );
    }
      
    if( keyhole.variable( "TTLOutputPin", gOutputPin ) && gOutputPin >= 0 )
    { // Since we have declared this as a parameter, BCI2000 will set its value during Initialize()
      pinMode( gOutputPin, OUTPUT );
      digitalWrite( gOutputPin, gOutput ? HIGH : LOW );
    }

    if( keyhole.command( "publish" ) ) // The string "publish\n" will prompt the sketch to reply with the following definitions.
    {                                  // The BCI2000's SerialInterface will send it if you start it with --PublishCommand=publish\n

      // BCI2000 Parameter definitions (any line containing an `=` sign will be interpreted as a parameter definition):
      keyhole << "Source:Microcontroller%20Sketch int    TTLInputPin=  " << gInputPin  << " % % % // microcontroller pin number for TTL voltage input";
      keyhole << "Source:Microcontroller%20Sketch int    TTLOutputPin= " << gOutputPin << " % % % // microcontroller pin number for TTL voltage output";
      keyhole << "Source:Microcontroller%20Sketch string SketchDescription= TTL%20input/output%20example % % % // description provided by the microcontroller (readonly)";

      // BCI2000 Event definitions (any line without an `=` sign will be interpreted as an event definition):
      keyhole << "TTLInput           1 0 0 0";
      keyhole << "PulseDurationMsec 15 0 0 0";

      // A blank line lets BCI2000 know to stop listening for definitions:
      keyhole << "";

      // NB: using `keyhole << whatever` requires Keyhole version 1.5.0+. It is slower than stringing together the equivalent Serial.print(); and
      //     Serial.println(); calls but it is much more convenient to write (and this section, the response to "publish\n", is not time-critical)
    }

    keyhole.end();
  }
}
