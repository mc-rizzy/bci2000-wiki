/////////////////////////////////////////////////////////////////////////////
// $Id: SerialInterface.cpp 9216 2026-01-01 14:20:40Z mellinger $
// Authors: jezhill@gmail.com
// Description: An extension that allows flexible communication to and from
// serial-port devices such as programmable microcontrollers (e.g. Arduino,
// Teensy, Pico, etc.)
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
#include "SerialInterface.h"
#include "BCIEvent.h"
#include "ProcessUtils.h"
#include "EnvVariable.h"

#include <stdio.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>

#define REPORT(X)  #X<<"="<<(X)<<"; "
#define REPORTS(X) #X<<"="<<EncodeStringLiteral(X)<<"; "
#define EXPRESSION_IS_EMPTY( E )  ( E.AsString().length() == 0 )

#ifdef DBSTREAM
#	define DB(X)        DBSTREAM<<X
#	define DBREPORT(X)  DBSTREAM<<REPORT(X)
#	define DBREPORTS(X) DBSTREAM<<REPORTS(X)
#else
#	define DB(X)       
#	define DBREPORT(X) 
#	define DBREPORTS(X)
#endif

#define SERIALDB( LEVEL, X ) if( mSerialDebugLevel >= LEVEL ) { bciout( "SerialDebugLevel>=" #LEVEL ) << X; }
	// for run-time debugging of messages to and fro (only used if SerialDebugLevel > 0)

#ifdef WIDGET_ADC // WIDGET_ADC will be left undefined if we are compiling this as an Extension (Logger) for another source module, but it will be defined if we are compiling the SerialWidget source module
	//RegisterFilter( SerialInterface, 1 );
	// Uncommenting the line above causes a linker error *even when* WIDGET_ADC is not defined
	// Under that circumstance, this code should not be seen by the compiler at all!
	// Is something somewhere grepping this line out of the file and using it??
#else
	Extension( SerialInterface );
#endif

SerialInterface::SerialInterface() :
	mBlocked( false ),
	mEnabled( false ),
	mGood( true ),
	mRecordEvents( false ),
	mMonitorThreadIsRunning( false ),
	mKillThreads( false ),
	mThreadMessage( DO_NOTHING ),
	mTimeSlice( Time::Seconds( 0.001 ) ),
#	ifdef WIDGET_ADC
		mBlockReceived( true ),
#	endif
	mThreadLock( true )
{
#	define CONFLICT_FLAG_NAME "_BCI2000_WIDGET_ADC_IS_HERE" // let's assume that the user will not have manually defined a variable with this unlikely name
#	ifdef WIDGET_ADC
		EnvVariable::Set( CONFLICT_FLAG_NAME, "1" );
		mSignalBuffer = NULL;
#	endif
}

SerialInterface::~SerialInterface()
{
	mKillThreads = true;
	PassMessageToMonitorThread( EXIT_THREAD );
	if (mMonitorThread.joinable())
		mMonitorThread.join();	
	mPort.Close();
}

void
SerialInterface::PassMessageToMonitorThread( int messageValue )
{
	mThreadMessage = messageValue;
	mThreadLock.Set();
}

void
SerialInterface::PassMessageToMonitorThread( const std::string & serialCommand )
{
	mThreadMessage = SEND_COMMAND;
	mPendingCommand = serialCommand;
	mThreadLock.Set();
}

int
SerialInterface::SendCommand( const std::string & serialCommand, const std::string & description )
{
	bciout << "Sending " << description << ( description.length() ? " " : "" ) << EncodeStringLiteral( serialCommand, "\"" );
	return ( int )mPort.Send( serialCommand );
}

#define TRY( X )   try{ X } catch( std::string const & error ) { bcierr << error; }
void
SerialInterface::Publish( void )
{
#	ifdef WIDGET_ADC
		// In the ADC, the environment variable was set in the constructor.
#	else
		// By contrast, the Extension checks that environment variable later, here in Publish(), so it
		// will know the ADC has already been constructed, in which case the Extension will be blocked.		
		std::string conflictFlagValue;
		EnvVariable::Get( CONFLICT_FLAG_NAME, conflictFlagValue );
		mBlocked = ( conflictFlagValue != "" && conflictFlagValue != "0" );
		if( mBlocked ) bciout << "Extension disabled, because an ADC incorporating the same functionality will be using the same serial device and BCI2000 parameters.";
#	endif
	
	std::string portSpec = ( std::string )OptionalParameter( "SerialPort" );
#	ifdef WIDGET_ADC
		BEGIN_PARAMETER_DEFINITIONS
		
			"Source:Signal%20Properties int       SamplingRate=     1000Hz 1000Hz 0 % // the signal sampling rate",
			"Source:Signal%20Properties int       SampleBlockSize=    40     40   1 % // number of samples per block",
			"Source:Signal%20Properties int       SourceCh=         auto      1   1 % // number of digitized channels total",
			"Source:Signal%20Properties floatlist SourceChGain=   1 auto      %   % % // conversion factors from AD units into microvolts",
			"Source:Signal%20Properties floatlist SourceChOffset= 1 auto      %   % % // AD offsets for recorded channels",
			"Source:Signal%20Properties list      ChannelNames=   1 auto      %   % % // channel names",
			// TODO: are the last two/three needed? or are they handled by DataIOFilter?
		
			"Source:Signal%20Source     intlist   SourceChPins=   1    0      0   % % // microcontroller pin index corresponding to each source channel",
		
		END_PARAMETER_DEFINITIONS
		// In ADC mode, do NOT abort if SerialPort is empty - fall through to define all the parameters regardless.
#	else
		if( mBlocked || !portSpec.length() ) return;
#	endif
	
	BEGIN_PARAMETER_DEFINITIONS
		"Source:Serial%20Port string SerialPort=              %    %  % % // serial port address (optionally, append colon followed by comma-separated MODE options like dtr=on etc.)(noedit)",
		"Source:Serial%20Port string PublishCommand=          %    %  % % // if defined, send these bytes to the serial device on startup (escape strings \\t \\r \\n \\0 \\\\ and \\xNN will be decoded) and expect newline-delimited event definitions, followed by a blank line, in return (noedit)",
		"Source:Serial%20Port string StartCommand=            %    %  % % // if defined, send these bytes to the serial device to indicate it is OK to send event information (escape strings \\t \\r \\n \\0 \\\\ and \\xNN will be decoded)",
		"Source:Serial%20Port string StopCommand=             %    %  % % // if defined, send these bytes to the serial device to indicate that event information should no longer be sent (escape strings \\t \\r \\n \\0 \\\\ and \\xNN will be decoded)",
		"Source:Serial%20Port string ParameterUpdateCommand=  %    %  % % // if defined, send these bytes to the serial device when a run stops (escape strings \\t \\r \\n \\0 \\\\ and \\xNN will be decoded), and expect 'key=value' parameter-update lines, followed by a blank line, in return",
		"Source:Serial%20Port matrix SerialInputs=  1 { When%20these%20bytes%20are%20received, set%20this%20event to%20this%20value and%20then%20(0%20or%20blank) } % %    % % % // table of incoming byte strings (escape strings \\t \\r \\n \\0 \\\\ and \\xNN will be decoded) and associated actions",
		"Source:Serial%20Port int    SkipUnrecognizedBytes=   1    1  0 1 // whether to throw away unrecognized preceding bytes while matching SerialInputs entries (boolean)",
		"Source:Serial%20Port matrix SerialOutputs= 1 { When%20this%20Expression%20becomes%20true, send%20these%20bytes:     } % %    % % % // table of contingences and outgoing byte strings (escape strings \\t \\r \\n \\0 \\\\ and \\xNN will be decoded)",
		"Source:Serial%20Port int    ElseIf=                  0    0  0 1 // how to process the rows of SerialOutputs: 0: process all rows, 1: stop at the first match (enumeration)",
		"Source:Serial%20Port int    SerialDebugLevel=        0    0  0 % // how verbose to be about bytes sent and received (values > 0 potentially sacrifice performance)",
	END_PARAMETER_DEFINITIONS
	
	mStartCommand = DecodeStringLiteral( OptionalParameter( "StartCommand" ) );
	mStopCommand  = DecodeStringLiteral( OptionalParameter( "StopCommand"  ) );
	std::string publishCommand = DecodeStringLiteral( OptionalParameter( "PublishCommand" ) );
	if( !portSpec.length() )
	{
		if( publishCommand.length() ) bcierr << "cannot set --PublishCommand on the command-line without also setting --SerialPort";
		return;
	}
	std::vector< std::string > definitions;
	TRY(
		bciout << "Connecting to " << portSpec;
		mPort.Open( portSpec );
		if( mStopCommand.length() ) SendCommand( mStopCommand, "stop command" );
		if( publishCommand.length() )
		{
			SendCommand( publishCommand, "publish command" );
			bciout << "Waiting for replies followed by a blank line";
			std::string definition;
			while( true )
			{
				int64_t nBytes = mPort.ReceiveUntil( definition, '\n' );
				if( nBytes >= 2 && definition[ nBytes - 2 ] == '\r' ) definition.resize( nBytes - 2 ); // remove possible trailing CRLF
				else if( nBytes ) definition.resize( nBytes - 1 ); // or maybe just trailing LF
				if( !definition.length() ) break;
				bciout << "Received " << ( definition.find( '=' ) == std::string::npos ? "event" : "parameter" ) << " definition " << EncodeStringLiteral( definition, "\"" );
				definitions.push_back( definition );
			}
		}
	)
	for( int i = 0; i < definitions.size(); i++ )
	{
		std::string & definition = definitions[ i ];
		if( definition.find( '=' ) != std::string::npos )
		{
			std::stringstream ss( definition );
			std::string section, type, name;
			ss >> section >> type >> name;
			name.resize( name.length() - 1 ); // removes trailing `=` from parameter name
			size_t positionOfLastComment = definition.rfind( "//" );
			size_t positionOfReadOnlyFlag = definition.rfind( "(readonly)" );
			size_t positionOfNoeditFlag = definition.rfind( "(noedit)" );
			if( positionOfLastComment != std::string::npos && positionOfReadOnlyFlag != std::string::npos && positionOfReadOnlyFlag > positionOfLastComment )
				{ if( positionOfNoeditFlag == std::string::npos || positionOfNoeditFlag < positionOfLastComment ) definition += "(noedit)"; }
			else if( type == "int" || type == "float" ) mNumericParameters.push_back( name );
			else if( type == "string" ) mStringParameters.push_back( name );
			else bcierr << "parameter definitions received via the serial port (if not readonly) must be of type int, float or string, whereas " << name << " was defined as " << type;
			BEGIN_PARAMETER_DEFINITIONS
				definition.c_str(),
			END_PARAMETER_DEFINITIONS
		}
		else
		{
			BEGIN_EVENT_DEFINITIONS
				definition.c_str(),
			END_EVENT_DEFINITIONS
		}
	}
}

void
SerialInterface::ParseOutputs( OutputVector & outputs ) const
{
	int serialDebugLevel = Parameter( "SerialDebugLevel" );
	ParamRef param = Parameter( "SerialOutputs" );
	int expectedNumberOfColumns = 2;
	if( param->NumColumns() != expectedNumberOfColumns && !( param->NumColumns() == 0 && param->NumRows() == 0 ) )
		{ bcierr << param->Name() << " parameter must have " << expectedNumberOfColumns << " columns"; return; }
	outputs.clear();
	for( int iRow = 0; iRow < param->NumRows(); iRow++ )
	{
		std::string expression = param( iRow, 0 );
		std::string byteString = DecodeStringLiteral( param( iRow, 1 ) );
		if( expression.length() ) outputs.emplace_back( iRow + 1, expression, byteString, serialDebugLevel ); // SerialInterface::Output() constructor args
		// it's OK to have an empty byte string on a particular row if the expression is meaningful and ElseIf mode is set - could translate as  if(row1BecameTrue) SendNothing(); else if(row2BecameTrue) SendSomething(); ...
	}
}

size_t
SerialInterface::ParseInputs( InputVector & inputs ) const
{
	bool allowSkip = Parameter( "SkipUnrecognizedBytes" );
	int serialDebugLevel = Parameter( "SerialDebugLevel" );
	ParamRef param = Parameter( "SerialInputs" );
	int expectedNumberOfColumns = 4;
	if( param->NumColumns() != expectedNumberOfColumns && !( param->NumColumns() == 0 && param->NumRows() == 0 ) )
		{ bcierr << param->Name() << " parameter must have " << expectedNumberOfColumns << " columns"; return 0; }
	inputs.clear();
	size_t maxByteStringLength = 0;
	for( int iRow = 0; iRow < param->NumRows(); iRow++ )
	{
		std::string byteString = DecodeStringLiteral( param( iRow, 0 ) );
		std::string event = param( iRow, 1 );
		std::string value = param( iRow, 2 );
		std::string zero  = param( iRow, 3 );
		if( !byteString.length() ) continue; 
		
		std::stringstream ss;
		if( event.length() )
		{
			State( event ); // TODO: at Preflight time, this serves to assert that the state exists: can we also assert that it is an Event rather than just a vanilla State Variable?
			ss.clear(); ss.str( "" ); ss << value; unsigned int x; ss >> x;
			if( ss.fail() ) { bcierr << "Could not interpret \"" << value << "\" as an unsigned integer in the third column of " << param->Name(); return 0; }
			ss.clear(); ss.str( "" ); ss << event << " " << value;
			if( zero == "0" ) ss << " 0";
			else if( zero.length() ) { bcierr << "In the fourth column of " << param->Name() << ", each cell must either contain 0 or be left blank"; return 0; }
		}
		inputs.emplace_back( iRow + 1, byteString, ss.str(), allowSkip, serialDebugLevel ); // SerialInterface::Input() constructor args
		if( byteString.length() > maxByteStringLength ) maxByteStringLength = byteString.length();
	}
	std::sort( inputs.begin(), inputs.end() ); // sorts in descending order of byteString length
	return maxByteStringLength;
}

#ifdef WIDGET_ADC
	void SerialInterface::AutoConfig( const SignalProperties & props )
	{
		ParamRef
			SourceChPins   = Parameter( "SourceChPins" );
		MutableParamRef
			SourceCh       = Parameter( "SourceCh" ),
			SourceChGain   = Parameter( "SourceChGain" ),
		    SourceChOffset = Parameter( "SourceChOffset" ),
			ChannelNames   = Parameter( "ChannelNames" );
		int nChannels = SourceChPins->NumValues();
		SourceCh = nChannels;
		SourceChGain->SetNumValues( nChannels );
		SourceChOffset->SetNumValues( nChannels );
		ChannelNames->SetNumValues( nChannels );
		for( int i = 0; i < nChannels; i++ )
		{
			SourceChGain( i ) = 1;
			SourceChOffset( i ) = 0;
			std::stringstream ss;
			ss << "Pin" << ( int )SourceChPins( i );
			ChannelNames( i ) = ss.str();
		}
	}
#endif

#ifdef WIDGET_ADC
	void SerialInterface::Preflight( const SignalProperties & inputProperties, SignalProperties & outputProperties ) const
#else
	void SerialInterface::Preflight() const
#endif
{
	std::string portSpec = ( std::string )OptionalParameter( "SerialPort" );
#	ifdef WIDGET_ADC
		if( !portSpec.length() ) bcierr << "SerialPort parameter cannot be empty";
#	endif
	if( mBlocked || !portSpec.length() ) return;
	DecodeStringLiteral( Parameter( "StartCommand"           ) );
	DecodeStringLiteral( Parameter( "StopCommand"            ) );
	DecodeStringLiteral( Parameter( "ParameterUpdateCommand" ) );

	InputVector inputs; ParseInputs( inputs );
	OutputVector outputs; ParseOutputs( outputs );
	Parameter( "ElseIf" );
	
	for( int i = 0; i < mNumericParameters.size(); i++ ) Parameter( mNumericParameters[ i ] );
	for( int i = 0; i < mStringParameters.size();  i++ ) Parameter( mStringParameters[  i ] );
#	ifdef WIDGET_ADC
		double samplesPerSecond = Parameter( "SamplingRate" ).InHertz();
		int samplesPerBlock = Parameter( "SampleBlockSize" );
		int numberOfChannels = Parameter( "SourceCh" );
		if( Parameter( "SourceChPins" )->NumValues()   != numberOfChannels ) bcierr << "number of elements in SourceChPins must be "   << numberOfChannels << " as specified by SourceCh";
		if( Parameter( "ChannelNames" )->NumValues()   != numberOfChannels ) bcierr << "number of elements in ChannelNames must be "   << numberOfChannels << " as specified by SourceCh";
		if( Parameter( "SourceChGain" )->NumValues()   != numberOfChannels ) bcierr << "number of elements in SourceChGain must be "   << numberOfChannels << " as specified by SourceCh";
		if( Parameter( "SourceChOffset" )->NumValues() != numberOfChannels ) bcierr << "number of elements in SourceChOffset must be " << numberOfChannels << " as specified by SourceCh";
		// TODO: are the last two/three needed? or are they handled by DataIOFilter?
		outputProperties.SetType( SignalType::float32 ); // it will be assumed that the widget delivers signals in this format; in principle if we changed this line to match the sketch's behavior, the rest of this file should automatically handle float32, int32 or int16 (but not float24)
		outputProperties.SetChannels( numberOfChannels );
		outputProperties.SetElements( samplesPerBlock );
		for( int i = 0; i < numberOfChannels; i++ )
		{
			outputProperties.ChannelLabels()[ i ] = Parameter( "ChannelNames" )( i );
			for( int j = i + 1; j < numberOfChannels; j++ )
				if( ( int )Parameter( "SourceChPins" )( i ) == ( int )Parameter( "SourceChPins" )( j ) )
					bcierr << "Pin number " << ( int )Parameter( "SourceChPins" )( i ) << " is repeated in SourceChPins";
		}

#	endif
}

#ifdef WIDGET_ADC
	void SerialInterface::Initialize( const SignalProperties & inputProperties, const SignalProperties & outputProperties )
#else
	void SerialInterface::Initialize()
#endif
{
	mGood = true;
	mPortSpec = ( std::string )OptionalParameter( "SerialPort" );
	mEnabled = mPortSpec.length() > 0 && !mBlocked; // mEnabled will always be true from this point on if we're in WIDGET_ADC mode, because in that mode an empty SerialPort parameter will have already caused Preflight() to fail
	if( !mEnabled ) return;
	mStartCommand        = DecodeStringLiteral( Parameter( "StartCommand"            ) );
	mStopCommand         = DecodeStringLiteral( Parameter( "StopCommand"             ) );
	mParamUpdateCommand  = DecodeStringLiteral( Parameter( "ParameterUpdateCommand"  ) );
		
	if( !mMonitorThreadIsRunning )
	{
		mMonitorThread = std::thread( &SerialInterface::MonitorThread, this );
	}
	mMaxInputCommandLength = ParseInputs( mInputs );
	ParseOutputs( mOutputs );
	mElseIf = Parameter( "ElseIf" );
	mAllowSkip = Parameter( "SkipUnrecognizedBytes" );
	mSerialDebugLevel = Parameter( "SerialDebugLevel" );
	
	std::string parameterCommand;
	TRY(
		mThreadMessage = DO_NOTHING;
		mPendingCommand = "";
		if( mPort.IsOpen() && mStopCommand.length() ) SendCommand( mStopCommand, "stop command" );
		mPort.Close();
		mPort.Open( mPortSpec ); // NB: in principle the port may have changed, but we are not going to ask it to publish again
		if( mStopCommand.length() ) SendCommand( mStopCommand, "stop command" );
		for( int i = 0; i < mNumericParameters.size(); i++ )
		{
			parameterCommand = mNumericParameters[ i ] + "=" + Parameter( mNumericParameters[ i ] ) + "\n";
			SendCommand( parameterCommand );
		}
		for( int i = 0; i < mStringParameters.size();  i++ )
		{
			parameterCommand = mStringParameters[ i ] + "=" + EncodeStringLiteral( Parameter( mStringParameters[ i ] ), "\"" ) + "\n";
			SendCommand( parameterCommand );
		}
	)
#	ifdef WIDGET_ADC
		mIncoming.SetProperties( outputProperties );
		mSignalBuffer = new char[ mIncoming.Elements() * mIncoming.Channels() * mIncoming.Type().Size() ];
#		define BUILD_STRING( VARNAME, TERMS ) { std::stringstream _ss; _ss << TERMS; VARNAME = _ss.str(); }
		TRY (
			BUILD_STRING( parameterCommand, "samplesPerSecond=" << ( double )Parameter( "SamplingRate" ).InHertz() << "\n" );
			SendCommand( parameterCommand );
			BUILD_STRING( parameterCommand, "samplesPerBlock="  << ( int )Parameter( "SampleBlockSize" ) << "\n" );
			SendCommand( parameterCommand );
			std::stringstream ss;
			ss << "sourcePins=\"";
			for( int i = 0; i < Parameter( "SourceChPins" )->NumValues(); i++ ) ss << ( i ? " " : "" ) << ( int )Parameter( "SourceChPins" )( i );
			ss << "\"\n";
			parameterCommand = ss.str();
			SendCommand( parameterCommand );
			SerialConnection::SleepSeconds( 0.250 );
		)
		if( mGood )
		{
			if( mStartCommand.length() ) SendCommand( mStartCommand, "start command" );
			PassMessageToMonitorThread( FLUSH_INCOMING );
		}
#	endif
}

void
SerialInterface::Halt( void )
{
	if( !mEnabled ) return;
#	ifdef WIDGET_ADC
		delete [] mSignalBuffer;
		mSignalBuffer = NULL;
	
		if( mStopCommand.length() ) SendCommand( mStopCommand, "stop command" );
#	else
		// NB: in the Extension, flow doesn't ever seem to reach here before a regular Preflight/Initialize, only on error - do not rely on this method for regular cleanup
		mPort.Close();
#	endif
}

void
SerialInterface::StartRun( void )
{
	if( !mEnabled ) return;
#	ifdef WIDGET_ADC
		// StartCommand is used at Initialize() time rather than StartRun()
#	else
		if( mStartCommand.length() ) SendCommand( mStartCommand, "start command" );
		PassMessageToMonitorThread( FLUSH_INCOMING );
#	endif
	mRecordEvents = true;
}

void
SerialInterface::StopRun( void )
{
	if( !mEnabled ) return;
#	ifdef WIDGET_ADC
		// StopCommand is used at Halt() time rather than StopRun()
#	else
		if( mStopCommand.length() ) SendCommand( mStopCommand, "stop command" );
#	endif
	mRecordEvents = false;
	if( mParamUpdateCommand.length() )
	{
		std::lock_guard< std::mutex > lock( mMutex ); // start of mutex critical section
		
		SendCommand( mParamUpdateCommand, "end-of-run update command" );
		bciout << "Waiting for replies followed by a blank line";
		std::string line;
		while( true )
		{
			int64_t nBytes = mPort.ReceiveUntil( line, '\n' );
			if( nBytes >= 2 && line[ nBytes - 2 ] == '\r' ) line.resize( nBytes - 2 ); // remove possible trailing CRLF
			else if( nBytes ) line.resize( nBytes - 1 ); // or maybe just trailing LF
			if( !line.length() ) break;
			
			bciout << "Received update: " << line;
			std::string paramName, paramValue;
			const char * cursor = line.c_str();
			const char * lineEnd = cursor + line.length();
			while( lineEnd != cursor && ::isspace( *( lineEnd - 1 ) ) ) lineEnd--;
			while( cursor != lineEnd && *cursor != '=' && !::isspace( *cursor ) ) paramName += *cursor++;
			while( cursor != lineEnd && ::isspace( *cursor ) ) cursor++;
			if( *cursor != '=' ) { bcierr << "failed to understand line as a `ParamName = Value` pattern"; continue; }
			cursor++;
			while( cursor != lineEnd && ::isspace( *cursor ) ) cursor++;
			bool quoted = ( *cursor == '"' || *cursor == '\'' );
			if( quoted )
			{
				if( lineEnd == cursor || lineEnd == cursor + 1 || *( lineEnd - 1 ) != *cursor ) { bcierr << "unmatched quote in line"; continue; }
				cursor++;
				lineEnd--;
			}
			while( cursor != lineEnd ) paramValue += *cursor++;
			if( quoted ) paramValue = DecodeStringLiteral( paramValue );
			Parameter( paramName ) = paramValue;
		}
	} // end of mutex critical section
}

#ifdef WIDGET_ADC
	void SerialInterface::Process( const GenericSignal & inputSignal, GenericSignal & outputSignal )
#else
	void SerialInterface::Process()
#endif
{
	if( !mEnabled ) return;
	bool sending = false;
	for( OutputVector::iterator itOutput = mOutputs.begin(); itOutput != mOutputs.end(); itOutput++ )
	{
		sending |= itOutput->Evaluate();
		if( sending && mElseIf ) break;
	}
	if( sending ) PassMessageToMonitorThread( SEND_TRIGGERS );
#	ifdef WIDGET_ADC
		WithThreadPriority( ThreadUtils::Priority::Maximum - 1 ) mBlockReceived.Wait();
		outputSignal = mIncoming;
#	endif
}

void
SerialInterface::MonitorThread()
{	
	mMonitorThreadIsRunning = true;
	bool keepGoing = true;
	std::string line;
	static char incomingMessage[ 4096 ];
	const std::string littleEndianMarker( "\x01\x00", 2 );
	const std::string    bigEndianMarker( "\x00\x01", 2 );
	while( keepGoing && !mKillThreads )
	{
		bool gotMessage;
		WithThreadPriority( ThreadUtils::Priority::Maximum - 1 )
			gotMessage = mThreadLock.Wait( Time::Seconds( 0.001 ) );
		
		if( gotMessage )
		{
			switch( mThreadMessage )
			{
				case SEND_COMMAND:
					if( mPendingCommand.length() )
					{
						SendCommand( mPendingCommand );
						mPendingCommand = "";
					}
					break;
				case SEND_TRIGGERS:
					for( OutputVector::iterator itOutput = mOutputs.begin(); itOutput != mOutputs.end(); itOutput++ )
						itOutput->Send( mPort );
					break;
				
				case FLUSH_INCOMING:
					line = "";
					mPort.ReceiveAvailable( incomingMessage, sizeof( incomingMessage ) );
					break;
				
				case EXIT_THREAD:
					keepGoing = false;
					break;
			} // end of switch( mThreadMessage )
			mThreadMessage = DO_NOTHING; // message handled, so put the flag down
			continue; // next iteration of while(keepGoing) loop
		}
		
		// At this point, we must have timed out while waiting for a message from Process().
		// So, let's see if the widget has sent us anything.
		
		{
			std::lock_guard< std::mutex > lock( mMutex ); // start of mutex critical section
			// TODO: do we need to start and stop the thread, or perhaps acquire the mutex, in a second/subsequent Initialize()
			//       which might concurrently be disconnecting/reconnecting/sending parameter values? So far haven't needed to...

			int64_t nBytesReceived = mPort.ReceiveAvailable( incomingMessage, sizeof( incomingMessage ) );
			if( nBytesReceived > 0 ) SERIALDB( 3, "Received " << EncodeStringLiteral( std::string( incomingMessage, nBytesReceived ), "\"" ) << "; buffer is now " << EncodeStringLiteral( line + std::string( incomingMessage, nBytesReceived ), "\"" ) );
			for( int64_t iMessageByte = 0; iMessageByte < nBytesReceived; iMessageByte++ )
			{
				size_t matchedLength = 0;
				for( InputVector::iterator itInput = mInputs.begin(); itInput != mInputs.end(); itInput++ )
					matchedLength = itInput->Match( line, incomingMessage[ iMessageByte ], matchedLength );
				if( matchedLength ) { line = ""; continue; } // next iMessageByte
				
				if( incomingMessage[ iMessageByte ] != '\n' )
				{
					if( line.length() >= 4096 && line.length() >= mMaxInputCommandLength ) // overflow
					{
						if( mAllowSkip ) line = line.substr( line.length() - mMaxInputCommandLength + 1 );
						else continue; // next iMessageByte
					}
					line += incomingMessage[ iMessageByte ];
					continue; // next iMessageByte
				}
				// Now we know we have received a line terminated by \n (possibly \r\n)
				if( line.length() && line[ line.length() - 1 ] == '\r' ) line.resize( line.length() - 1 );
				if( !line.length() ) continue;
				// Now we have a non-empty line, stripped of its line ending
				
	#			ifdef WIDGET_ADC
					if( line == littleEndianMarker || line == bigEndianMarker )
					{
						uint16_t endianMarker = *( uint16_t * )line.c_str();
						bool endianSwapNeeded = ( endianMarker != 1 );
						size_t bytesPerSample = mIncoming.Type().Size();
						size_t nBytesNeeded = mIncoming.Elements() * mIncoming.Channels() * bytesPerSample;
						char * cursor = mSignalBuffer;
						// we expect nBytesNeeded bytes of binary data to immediately follow the "@" line
						// but some of this may already have arrived from the ReceiveAvailable() call, so
						// let's use it up first...
						iMessageByte++; // first, get past the '\n' currently under consideration in the for loop
						while( iMessageByte < nBytesReceived && nBytesNeeded ) { *cursor++ = incomingMessage[ iMessageByte++ ]; nBytesNeeded--; }
						// ...and if that wasn't enough, synchronously read the rest
						int64_t nExtraBytesReceived = nBytesNeeded ? mPort.Receive( cursor, nBytesNeeded ) : 0;
						if( nExtraBytesReceived == nBytesNeeded )
						{
							cursor = mSignalBuffer;
							switch( mIncoming.Type() )
							{
								case SignalType::float32:
									for( int iSample = 0; iSample < mIncoming.Elements(); iSample++ )
										for( int iChannel = 0; iChannel < mIncoming.Channels(); iChannel++, cursor += bytesPerSample )
										{
											if( endianSwapNeeded ) { char tmp = cursor[ 0 ]; cursor[ 0 ] = cursor[ 3 ]; cursor[ 3 ] = tmp; tmp = cursor[ 1 ]; cursor[ 1 ] = cursor[ 2 ]; cursor[ 2 ] = tmp; }
											mIncoming( iChannel, iSample ) = *( float * )cursor;
										}
									break;
									
								case SignalType::int32:
									for( int iSample = 0; iSample < mIncoming.Elements(); iSample++ )
										for( int iChannel = 0; iChannel < mIncoming.Channels(); iChannel++, cursor += bytesPerSample )
										{
											if( endianSwapNeeded ) { char tmp = cursor[ 0 ]; cursor[ 0 ] = cursor[ 3 ]; cursor[ 3 ] = tmp; tmp = cursor[ 1 ]; cursor[ 1 ] = cursor[ 2 ]; cursor[ 2 ] = tmp; }
											mIncoming( iChannel, iSample ) = *( int32_t * )cursor;
										}
									break;
									
								case SignalType::int16:
									for( int iSample = 0; iSample < mIncoming.Elements(); iSample++ )
										for( int iChannel = 0; iChannel < mIncoming.Channels(); iChannel++, cursor += bytesPerSample )
										{
											if( endianSwapNeeded ) { char tmp = cursor[ 0 ]; cursor[ 0 ] = cursor[ 1 ]; cursor[ 1 ] = tmp; }
											mIncoming( iChannel, iSample ) = *( int16_t * )cursor;
										}
									break;
									
								default:
									bcierr << "unsupported SignalType";
									break;
							}
							mBlockReceived.Set();
						}
						else { bcierr << "expected " << nBytesNeeded << " extra bytes, but got " << nExtraBytesReceived; }
						
						line = "";
						continue; // next iMessageByte
					}
	#			endif
				
				if( line[ 0 ] == '{' ) // some JSON message
				{
					if( line.find( "_ERROR_" ) != std::string::npos ) // The Keyhole library delivers error messages like {"_KEYHOLE_ERROR_TYPE": "BadKey", "_KEYHOLE_ERROR_MSG": "failed to recognize command"}
					{
						size_t pos;
						std::string first  = "{\"_KEYHOLE_ERROR_TYPE\": \""; // if these are present, finesse the error message
						std::string second = "\", \"_KEYHOLE_ERROR_MSG\": "; // if not, no harm no foul
						if( ( pos = line.find( first  ) ) != std::string::npos ) line.replace( pos, first.length(), "The device at " + mPort.Name() + " issued a " );
						if( ( pos = line.find( second ) ) != std::string::npos ) { line.replace( pos, second.length(), " error: " ); line.resize( line.length() - 1 ); }
						
						mGood = false;
						bcierr << line; 
					}
					// otherwise, just ignore it
				}
				else // if it isn't JSON, assume it must be a BCI2000 event descriptor line
				{
					if( mRecordEvents ) bcievent << line;
					// otherwise (i.e. if suspended) just ignore it
				}
				line = ""; // line processed. start a new line
				
			} // next iMessageByte
		} // end of mutex critical section
	} // end of main while(keepGoing) loop
	mMonitorThreadIsRunning = false;
}


////////////////////////////////////////////////////////////////
SerialInterface::Output::Output( unsigned int id, const std::string & triggerExpression, const std::string & outgoingMessage, int serialDebugLevel )
: mPreviousValue( false ), mBytesToSend( 0 ), mTriggerExpression( triggerExpression ), mOutgoingMessage( outgoingMessage ), mID( id ), mSerialDebugLevel( serialDebugLevel )
{
	mTriggerExpression.Compile();
	mBytesToSend = 0;
}

SerialInterface::Output::~Output()
{
	
}

bool
SerialInterface::Output::Evaluate( void )
{
	bool newValue = ( mTriggerExpression.Evaluate() != 0.0 );
	bool returnValue = newValue && !mPreviousValue;
	mPreviousValue = newValue;
	if( returnValue ) mBytesToSend = mOutgoingMessage.length();
	if( returnValue ) SERIALDB( 2, "SerialOutputs row " << mID << " was triggered" );
	return returnValue;
}

int64_t
SerialInterface::Output::Send( SerialConnection & port )
{
	if( !mBytesToSend ) return 0;
	SERIALDB( 1, "SerialOutputs row " << mID << " is sending " << mBytesToSend << " bytes " << EncodeStringLiteral( mOutgoingMessage, "\"" ) );
	int64_t returnValue = port.Send( mOutgoingMessage.c_str(), mBytesToSend );
	if( returnValue > 0 ) mBytesToSend = 0;
	return returnValue;
}
////////////////////////////////////////////////////////////////
SerialInterface::Input::Input( unsigned int id, const std::string & incomingCommand, const std::string & event, bool allowSkip, int serialDebugLevel )
: mIncomingCommand( incomingCommand ), mEvent( event ), mID( id ), mAllowSkip( allowSkip ), mSerialDebugLevel( serialDebugLevel )
{
	
}

SerialInterface::Input::~Input()
{
	
}

bool
SerialInterface::Input::operator<( const Input & other ) const
{ // defined such that Input instances get std::sort()ed in decreasing order of command length
	return this->mIncomingCommand.length() > other.mIncomingCommand.length(); // greater-than is used here to ensure *decreasing* length order
}

size_t
SerialInterface::Input::Match( const std::string & prefix, char extraChar, size_t previousMatchLength )
{
	size_t prefixLength = prefix.length();
	size_t matchLength = mIncomingCommand.length();
	if( previousMatchLength > matchLength ) return previousMatchLength; // Inputs will be tested in decreasing order of byteString length; if a longer string has previously been matched to the current message, do not consider shorter strings, since they might be trailing substrings and could generate erroneous matches
	if( matchLength > prefixLength + 1 ) return previousMatchLength;
	if( matchLength < prefixLength + 1 && !mAllowSkip ) return previousMatchLength;
	if( mIncomingCommand[ matchLength - 1 ] != extraChar ) return previousMatchLength;
	if( matchLength > 1 && ::memcmp( prefix.c_str() + prefixLength + 1 - matchLength, mIncomingCommand.c_str(), matchLength - 1 ) != 0 ) return previousMatchLength;
	// match has been made
	if( mEvent.length() ) bcievent << mEvent;
	if( matchLength < prefixLength + 1 ) SERIALDB( 2, "SerialInputs row " << mID << " skipped incoming bytes " << EncodeStringLiteral( prefix.substr( 0, prefixLength + 1 - matchLength ), "\"" ) );
	SERIALDB( 1, "SerialInputs row " << mID << " matched incoming message " << EncodeStringLiteral( mIncomingCommand, "\"" ) );
	return matchLength;
}
////////////////////////////////////////////////////////////////
