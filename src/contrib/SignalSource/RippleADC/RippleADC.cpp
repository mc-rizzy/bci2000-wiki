////////////////////////////////////////////////////////////////////////////////
// Authors: bci2000@DESKTOP-503T7ML
// Description: RippleADC implementation
////////////////////////////////////////////////////////////////////////////////
#include "RippleADC.h"

#include "BCIStream.h"
#include "BCIEvent.h"
#include <stdint.h>
#include <iostream>
#include <unistd.h>
#include "ThreadUtils.h"

const RippleADC::FrontEndInfo RippleADC::cFrontends[2] =
{
	{ "stim", 24, 32, 0.022e-6,(SamplingRates)(HIRES & HiFreq)},
	{ "raw", 16, 32, 0.25e-6,(SamplingRates)(LFP & HIRES & RAW)}
};



// Make the source filter known to the framework.
RegisterFilter(RippleADC, 1); // ADC filters must be registered at location "1" in order to work.

RippleADC::RippleADC() :m_databuffer(NULL)
{

}

RippleADC::~RippleADC()
{
	delete m_databuffer;
}

void
RippleADC::OnPublish()
{

	BEGIN_PARAMETER_DEFINITIONS

		"Source:Signal%20Properties int SourceCh= auto "
		"auto 1 % // number of digitized and stored channels",

		"Source:Signal%20Properties int SampleBlockSize= auto "
		"auto 1 % // number of samples transmitted at a time",

		"Source:Signal%20Properties float SamplingRate= auto "
		"auto 0.0 % //  Macro LFP: 1khz HiRes: 2kHz Raw: 15kHz; Micro LFP: 1khz HiRes: 2kHz Raw: 30kHz",

		"Source:Signal%20Properties list SourceChGain= 1 auto "
		" // physical units per raw A/D unit",

		"Source:Signal%20Properties list SourceChOffset= 1 auto "
		" // raw A/D offset to subtract, typically 0",

		"Source:Signal%20Properties list ChannelNames= 1 auto "
		" // names of amplifier channels",

		"Source:Signal%20Properties list SourceChList= 1 auto"
		" // List of channels transmitted ",

		END_PARAMETER_DEFINITIONS

		BEGIN_STREAM_DEFINITIONS

		"Timestamp 32 0"

		END_STREAM_DEFINITIONS


}

bool RippleADC::Open()
{
	int err = 0;
	if ((err = xl_open_tcp()) != XL_ERR_SUCCESS) //right now use default settings
	{
		bcierr << "Error while connecting to Ripple System! Make sure your interface is within 192.168.42.*! Error code: " << std::to_string(err) << std::endl;
		return false;
	}
	return true;

}

bool RippleADC::Close()
{

	if (xl_close() != XL_ERR_SUCCESS)
	{
		return false;
		bcierr << "Could not close connection to Ripple Device!";
	}

	return true;

}


std::string RippleADC::GetFrontEndName(uint32_t el)
{
	char chl_name[STRLEN_LABEL];
	xl_get_fe_streams(&chl_name, 1, el);
	int frontend = xl_get_fe(el);
	return std::string(chl_name);
}

void
RippleADC::OnAutoConfig()
{

	if (!Open()) return;
	// The user has pressed "Set Config" and some parameters may be set to "auto",
	// indicating that they should be set from the current amplifier configuration.
	// In this handler, we behave as if any parameter were set to "auto", and the
	// framework will transparently make sure to preserve user-defined values.

	// Device availability (or connection parameters) may have changed, so close
	// and reopen the connection to the device before proceeding.



	uint32_t  channels = xl_list_elec(m_chList, MAX_NUM_CHLS, "all");
	Parameter("SourceCh") = channels; // Set SourceCh in case of "auto"
	// If SourceCh != auto but e.g. SourceChGain != auto, we need to use the actual
	// rather than the auto-set value of SourceCh in order to consistently configure
	// SourceChGain.
	// For this purpose, an ActualParameter() call will retrieve a parameter's current
	// value, no matter whether auto-configured or user-configured.
	channels = ActualParameter("SourceCh");
	Parameter("ChannelNames")->SetNumValues(channels);
	Parameter("SourceChGain")->SetNumValues(channels);
	Parameter("SourceChOffset")->SetNumValues(channels);
	Parameter("SourceChList")->SetNumValues(channels);
	for (int i = 0; i < channels; ++i)
	{


		Parameter("ChannelNames")(i) << GetFrontEndName(m_chList[i]) << "_" << m_chList[i]; // Omit "<< "_"" << i" if channel names are unique.

		double gainFactor = 1.0;

		Parameter("SourceChGain")(i) << gainFactor << "muV"; // Always provide correct physical unit for documentation and display purposes.
		// For most amplifiers, offset removal is done within the amp hardware or driver. Otherwise, you may set this to a nonzero value.
		Parameter("SourceChOffset")(i) = 0;
		Parameter("SourceChList")(i) = i;
	}

	double samplingRate = HIRES_SAMPLINGRATE;

	Parameter("SamplingRate") << samplingRate << "Hz";

	samplingRate = ActualParameter("SamplingRate");
	int samplesPerPacket = std::round(samplingRate / 10);

	Parameter("SampleBlockSize") = samplesPerPacket;
	Close();

}

void
RippleADC::OnPreflight(SignalProperties& Output) const
{
	SignalType sigType = SignalType::float32;
	int channels = Parameter("SourceCh");
	int samplesPerBlock = Output.Elements();
	int numberOfSignalChannels = Output.Channels();
	if (numberOfSignalChannels == 0)
		bcierr << "No appropriate recoding channels found!" << std::endl;
	Output = SignalProperties(numberOfSignalChannels + 1, samplesPerBlock, sigType);
	Output.ChannelLabels()[numberOfSignalChannels + 0] = "@Timestamp";
	int sr = Parameter("SamplingRate");

	if (Parameter("SourceChList")->NumValues() != channels)
		bcierr << "Number of SourceChList entries must be equal to the number of channels" << std::endl;

	switch (sr)
	{
	case RAW_SAMPLINGRATE:
		break;
	case HIRES_SAMPLINGRATE:
		break;
	case HIFREQ_SAMPLINGRATE:
		break;
	case LFP_SAMPLINGRATE:
		break;
	default:
		bcierr << "Unsupported samplingrate! Macro front ends can do: 1000Hz, 2000Hz, 15000Hz. Micro Front ends can support 1000Hz, 2000Hz, and 30000Hz";
	}


	// Note that the RippleADC instance itself, and its members, are read-only during the
	// Preflight phase---note the "const" at the end of the OnPreflight prototype above.
	// Any methods called by OnPreflight must also be declared as "const" in the same way.
}

void
RippleADC::OnInitialize(const SignalProperties& Output)
{
	// The user has pressed "Set Config" and all Preflight checks have passed.
	// The system will now transition into an "Initialized" state.

	// If the user should be able to control amplifier configuration through BCI2000,
	// you will need to use the amplifier's API here in order to apply parameter settings.
	// Preflight() checks should ensure that Initialize() never fails due to misconfiguration.
	// Still, unforeseen cases or hardware conditions may lead to failure.


	// The signal properties can no longer be modified, but the const limitation has gone, so
	// the RippleADC instance itself can be modified. Allocate any memory you need
	// store any information you need in private member variables.
	State("Timestamp");
	m_buffersize = Output.Channels() * Output.Elements();
	delete m_databuffer;
	m_databuffer = new float[m_buffersize];
	m_samplingRate = Parameter("SamplingRate");
	bciout << "Sampling Rate set to: " << m_samplingRate << std::endl;
	m_blocks = Parameter("SampleBlockSize");


	// Buffer allocation may happen in OnStartAcquisition as well, if more appropriate.
}

void
RippleADC::OnStartAcquisition()
{
	// This method is called from the acquisition thread once the system is initialized.
	// You should use this space to start up your amplifier using its API.  Any initialization
	// here is done in the acquisition thread, so non-thread-safe APIs should work if initialized here.
	Open();
	m_timestamp = xl_time();
	m_newtimestamp = 0;
	const int numCh = Parameter("SourceChList")->NumValues();
	for (int i = 0; i < numCh; i++)
		m_chList[i] = static_cast<uint32_t>(Parameter("SourceChList")(i));

}
void
RippleADC::DoAcquire(GenericSignal& Output)
{
	uint32_t total_pts = 0;
	uint32_t pts = 0;
	uint32_t timestamp = 0;
	std::fill_n(m_databuffer, m_buffersize, 0);

	uint32_t required_timestamp = m_timestamp + m_blocks * (RAW_SAMPLINGRATE / m_samplingRate);
	do
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
//		ThreadUtils::SleepForMs(0);
		timestamp = xl_time();
	} while ((required_timestamp - float(timestamp)) >= 0);


	switch (m_samplingRate)
	{
	case RAW_SAMPLINGRATE:
		pts = xl_cont_raw(&timestamp, m_databuffer, Output.Elements(), m_chList, Output.Channels() - 1, m_timestamp);
		break;
	case HIRES_SAMPLINGRATE:
		pts = xl_cont_hires(&timestamp, m_databuffer, Output.Elements(), m_chList, Output.Channels() - 1, m_timestamp);
		//pts = xl_cont_emg(&new_timestamp, m_databuffer, Output.Elements(), m_chList, Output.Channels(), m_timestamp);
		break;
	case HIFREQ_SAMPLINGRATE:
		pts = xl_cont_raw(&timestamp, m_databuffer, Output.Elements(), m_chList, Output.Channels() - 1, m_timestamp);
		break;
	case LFP_SAMPLINGRATE:
		pts = xl_cont_lfp(&timestamp, m_databuffer, Output.Elements(), m_chList, Output.Channels() - 1, m_timestamp);
		break;
	default:
		bcierr << "Unsupported samplingrate! Macro front ends can do: 1000Hz, 2000Hz, 15000Hz. Micro Front ends can support 1000Hz, 2000Hz, and 30000Hz";
	}

	if (pts != (Output.Elements()))
		bciwarn << "Wrong number of samples received! " << std::to_string(pts) << std::endl;
	m_timestamp = required_timestamp;
	m_newtimestamp = timestamp;

	//bciout << std::to_string(pts) << std::endl;
	for (int ch = 0; ch < Output.Channels() - 1; ++ch)
	{
		for (int s = 0; s < pts; ++s)
		{
			Output(ch, s) = m_databuffer[s + (ch * (pts))];


		}
	}
	for (int s = 0; s < pts; ++s)
		Output(Output.Channels() - 1, s) = m_newtimestamp;


}



void
RippleADC::OnStopAcquisition()
{
	Close();

}

