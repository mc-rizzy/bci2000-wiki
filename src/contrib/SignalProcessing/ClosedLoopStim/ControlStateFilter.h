////////////////////////////////////////////////////////////////////////////////
// Authors: Brunnerlab@DESKTOP-F8KRI7F.wucon.wustl.edu
// Description: ControlStateFilter header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_CLOSEDLOOPSTIMFILTER_H  // makes sure this header is not included more than once
#define INCLUDED_CLOSEDLOOPSTIMFILTER_H

#include "GenericFilter.h"
#include "Expression/Expression.h"

class ControlStateFilter : public GenericFilter
{
 public:
  ControlStateFilter();
  ~ControlStateFilter();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;

 private:
   Expression mControlExpression;
   // Initialize numeric variables to safe values
   int mMaxWait = 0;
   int mWait = 0;
   int mStateLength = 0;
   int mMinControlSamples = 0;
   int mConsecutiveTrue = 0;
};

#endif // INCLUDED_CLOSEDLOOPSTIMFILTER_H
