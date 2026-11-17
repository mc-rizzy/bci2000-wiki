////////////////////////////////////////////////////////////////////////////////
// Authors:engelhardt@neurotechcenter.org
// Description: InterpolationFilter header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_INTERPOLATIONFILTER_H 
#define INCLUDED_INTERPOLATIONFILTER_H

#include "GenericFilter.h"
#include "Expression.h"
#include <vector>

class InterpolationFilter : public GenericFilter
{
 public:
  InterpolationFilter();
  ~InterpolationFilter();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;

 private:
  struct ChannelSignal {
    double* data;
    int prevEl;
    int nextEl;
  };
  void getChannelSignal(const GenericSignal& Input, int el, double* data);
  int EvalSample(const GenericSignal& Input, int el);
  Expression mLostSampleExp;
  double* mPrevData;
  bool mEnableInterpolation;
};

#endif
