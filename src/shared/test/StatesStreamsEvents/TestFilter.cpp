////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: StatesStreamsEventsTestFilter
//   This filter accesses a number of states, streams, and events, driven by
//   by random numbers.
//   In the Operator's watches window, you should see a lineup of states being
//   set to 1, with the FooState leading.
//   The BarEvent will follow the FooEvent with a delay of 60ms.
////////////////////////////////////////////////////////////////////////////////

#include "TestFilter.h"
#include "BCIStream.h"
#include "BCIEvent.h"
#include "ThreadUtils.h"

RegisterFilter( TestFilter, 2.A );

TestFilter::TestFilter()
: mThreadFunc(&TestFilter::ThreadFunc, this),
  mThread(&mThreadFunc, "TestFilterThread")
{
}

TestFilter::~TestFilter()
{
  Halt();
}

void
TestFilter::Publish()
{
 BEGIN_STATE_DEFINITIONS
    "BlockState 1 0",
    "FooState 1 0",
 END_STATE_DEFINITIONS

 BEGIN_EVENT_DEFINITIONS
    "FooEvent 1 0",
    "BarEvent 1 0",
 END_EVENT_DEFINITIONS

 BEGIN_STREAM_DEFINITIONS
    "FooStream 1 0",
    "Channel1Stream 32 0 0 0",
 END_STREAM_DEFINITIONS
}

void
TestFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  Output = Input;
}


void
TestFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
}

void
TestFilter::StartRun()
{
    mThread.Start();
}


void
TestFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{

  Output = Input;

  // Copy Input into Channel1Stream to make sure there is no time shift.
  for (int i = 0; i < Input.Elements(); ++i) {
    State("Channel1Stream").Sample(i) = Input(0, i) + (1ul<<31);
  }
  // Toggle BlockState so it indicates block boundaries.
  State("BlockState") = State("BlockState") ? 0 : 1;
  // Randomly set FooState to 1 for a block, 
  // and have FooStream, FooEvent, and BarEvent correlate with it.
  if (::rand() > 9. * RAND_MAX / 10.) {
      State("FooState") = 1;
      State("FooStream").Sample(0) = 1;
      bcievent << "FooEvent 1 0";
      mWaitable.Set(); // will set BarEvent after 60ms
  }
  else {
      State("FooState") = 0;
  }
}

void
TestFilter::StopRun()
{
    mThread.Terminate();
}

void
TestFilter::Halt()
{
    mThread.Terminate();
}

void
TestFilter::ThreadFunc()
{
    while (mWaitable.Wait()) {
        mWaitable.Reset();
        ThreadUtils::SleepForMs(60);
        bcievent << "BarEvent 1 0";
    }
}
