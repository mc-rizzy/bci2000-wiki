////////////////////////////////////////////////////////////////////////////////
// Authors: engelhardt@neurotechcenter.org
// Description: ControlState implementation. Useful for transferring the output 
// of the signal processing pipeline (control signal) to the source module
////////////////////////////////////////////////////////////////////////////////

#include "ControlStateFilter.h"
#include "BCIStream.h"

RegisterFilter( ControlStateFilter, 2.A );

ControlStateFilter::ControlStateFilter()
  : mControlExpression(),
  mMaxWait(0),
  mWait(0),
  mStateLength(0),
  mMinControlSamples(0),
  mConsecutiveTrue(0)
{
}

ControlStateFilter::~ControlStateFilter()
{
  Halt();
}

void
ControlStateFilter::Publish()
{

 BEGIN_PARAMETER_DEFINITIONS
  "Filtering:Control string ControlExpression=  % % % % // Triggers ControlState",
  "Filtering:Control float MinControlDuration= 0.1s 0 % % // Minimum time ControlExpression must remain true",
  "Filtering:Control float RefractoryPeriod=  0.5s 0 % % // Time until ControlState can be triggered again",
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
  "ControlState 1 0 0 0",
  "RefractoryPeriod 1 0 0 0",
 END_STATE_DEFINITIONS

}

void
ControlStateFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  Output = Input;
  GenericSignal preflightSignals(Input);
  Expression exp = Parameter("ControlExpression").ToString();
  exp.Compile();
  exp.Evaluate(&preflightSignals);

  State("ControlState");
  State("RefractoryPeriod");
  Parameter("RefractoryPeriod");
  Parameter("MinControlDuration");
}


void
ControlStateFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  mControlExpression = Parameter("ControlExpression").ToString();
  mStateLength = Statevector->Samples();
  mMaxWait = ceil(Parameter("RefractoryPeriod").InSampleBlocks() * mStateLength);
  mMinControlSamples = ceil(Parameter("MinControlDuration").InSampleBlocks() * mStateLength);
  if (mMinControlSamples <= 0)
      mMinControlSamples = 1;
}

void
ControlStateFilter::StartRun()
{
  mWait = 0;
  mConsecutiveTrue = 0;
  State("RefractoryPeriod") = false;
}


void
ControlStateFilter::Process(const GenericSignal& Input, GenericSignal& Output)
{
	Output = Input;
	// Skip whole block if still in refractory
	if (mWait > mStateLength)
	{
		mWait -= mStateLength;
		State("ControlState") = false;
		State("RefractoryPeriod") = true;
		mConsecutiveTrue = 0;
		return;
	}

	//record with sample resolution
	//state vector length might be different than Input length
	for (int i = 0; i < Statevector->Samples() - 1; ++i)
	{
		if (mWait == 0)
		{
			// Check Control Expression
			if (mControlExpression.Evaluate(&Input, i))
			{
				// Control Expresion is true
				// Accumulate trigger state
				mConsecutiveTrue++;

				if (mConsecutiveTrue >= mMinControlSamples)
				{
					// Trigger and start refractory
					mWait = mMaxWait;
					State("ControlState")(i) = true;
					State("RefractoryPeriod")(i) = true;
					mConsecutiveTrue = 0;
					continue;
				}
				else
				{
					// Not yet enough duration
					State("ControlState")(i) = false;
					State("RefractoryPeriod")(i) = false;
				}
			}
			else
			{
				// Expression is false: reset duration counter and states
				mConsecutiveTrue = 0;
				State("ControlState")(i) = false;
				State("RefractoryPeriod")(i) = false;
			}
		}
		else // Refractory period
		{
			mWait--;
			State("ControlState")(i) = false;
			State("RefractoryPeriod")(i) = (mWait > 0);
			mConsecutiveTrue = 0;
		}
	}
}

void
ControlStateFilter::StopRun()
{
}

void
ControlStateFilter::Halt()
{
}

