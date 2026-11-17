// Authors: mellinger@neurotechcenter.org
// Description: StatesStreamsEventsTestFilter
//   This filter accesses a number of states, streams, and events, driven by
//   by random numbers.
//   In the Operator's watches window, you should see a lineup of states being
//   set to 1, with the FooState leading.
//   The BarEvent will follow the FooEvent with a delay of 60ms.
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_TESTFILTER_H  // makes sure this header is not included more than once
#define INCLUDED_TESTFILTER_H

#include "GenericFilter.h"
#include "Thread.h"
#include "WaitableEvent.h"
#include "Runnable.h"

class TestFilter : public GenericFilter
{
 public:
  TestFilter();
  ~TestFilter();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;

 private:
  void ThreadFunc();
  MemberCall<void(TestFilter*)> mThreadFunc;
  Thread mThread;
  WaitableEvent mWaitable;
};

#endif // INCLUDED_TESTFILTER_H
