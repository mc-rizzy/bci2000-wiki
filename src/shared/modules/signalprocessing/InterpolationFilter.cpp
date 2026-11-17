////////////////////////////////////////////////////////////////////////////////
// Authors: engelhardt@neurotechcenter.org
// Description: InterpolationFilter implementation
////////////////////////////////////////////////////////////////////////////////

#include "InterpolationFilter.h"
#include "BCIStream.h"

RegisterFilter( InterpolationFilter, 1.001 );

InterpolationFilter::InterpolationFilter()
: mPrevData(nullptr),
  mEnableInterpolation(false)
{
}

InterpolationFilter::~InterpolationFilter()
{
  Halt();

  if (mPrevData != nullptr)
    delete[] mPrevData;
}

void
InterpolationFilter::Publish()
{
 BEGIN_PARAMETER_DEFINITIONS

   "Source:InterpolationFilter string InterpolationExpression=  % % % % // Expression that determines which samples to replace", 

 END_PARAMETER_DEFINITIONS
}

void
InterpolationFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  Output = Input;

  //test if valid expression
  GenericSignal signal(Output);
  Expression e = Parameter("InterpolationExpression").ToString();
  e.Compile();
  e.Evaluate(&signal);
}

void
InterpolationFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  if (Parameter("InterpolationExpression").ToString() == "")
  {
    mEnableInterpolation = false;
    return;
  }

  mEnableInterpolation = true;
  mLostSampleExp = Parameter("InterpolationExpression").ToString();

  if (mPrevData == nullptr)
    mPrevData = new double[Input.Channels()] {};
}

void
InterpolationFilter::StartRun()
{
}

void
InterpolationFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  Output = Input;
  if (!mEnableInterpolation)
    return;

  std::vector<ChannelSignal> interpolate;
  int lastValidSample = -2;
  //find lost samples
  int el = 0;
  while (el < Output.Elements())
  {
    if (mLostSampleExp.Evaluate(&Input, el))
    {
      ChannelSignal sample;
      sample.data = new double[Input.Channels()] {};
      //get previous sample
      if (el != 0)
      {
        getChannelSignal(Input, el - 1, sample.data);
        sample.prevEl = el - 1;
      }
      else
      {
        std::copy(mPrevData, mPrevData + Output.Channels(), sample.data); // deep copy
        //*sample.data = *mPrevData; //deep copy?
        sample.prevEl = -1; //mark it as right before the start of the block
      }
      //find next valid sample
      sample.nextEl = EvalSample(Input, el + 1);

      //save for interpolation
      interpolate.push_back(sample);

      //prepare for next loop
      if (sample.nextEl != 0)
      {
        el = sample.nextEl + 1;
      }
      else
        break; //no valid samples in rest of block
    }
    else
      el++;
  }

  //fill the output when invalid
  for (auto& chunk : interpolate)
  {
    int nSamples = chunk.nextEl - chunk.prevEl;
    if (nSamples <= 1)
    {
      //edge cases
      if (nSamples == 1)
      {
        lastValidSample = -1;
        bcidbg << "InterpolationFilter: Cannot interpolate entire block" << std::endl;
      }
      else
      {
        lastValidSample = chunk.prevEl;
        bcidbg << "InterpolationFilter: Cannot interpolate sample at end of block" << std::endl;
      }
      //hold same value
      for (int ch = 0; ch < Output.Channels(); ch++)
      {
        for (int el = chunk.prevEl + 1; el < Output.Elements(); el++)
        {
          Output(ch, el) = chunk.data[ch];
        }
      }
    }
    else
    {
      //interpolate
      for (int ch = 0; ch < Output.Channels(); ch++)
      {
        double dy = (Output(ch, chunk.nextEl) - chunk.data[ch]) / (double) nSamples;
        for (int i = chunk.prevEl + 1; i < chunk.nextEl; i++)
        {
          Output(ch, i) = chunk.data[ch] + (i - chunk.prevEl) * dy;
        }
      }
      lastValidSample = chunk.nextEl;
    }
    delete[] chunk.data;
  }

  //get newest valid sample for next block
  if (lastValidSample == -2)
  {
    //all samples in block were valid, get last one
    getChannelSignal(Input, Output.Elements() - 1, mPrevData);
  }
  else if (lastValidSample != -1)
  {
    //at least one valid sample in the block
    for (int el = Output.Elements() - 1; el >= lastValidSample; el--)
    {
      if (!mLostSampleExp.Evaluate(&Input, el))
      {
        //valid, store
        getChannelSignal(Input, el, mPrevData);
        break;
      }
    }
  }
}

void
InterpolationFilter::StopRun()
{
}

void
InterpolationFilter::Halt()
{
}



//recursive, loops until sample is valid
int
InterpolationFilter::EvalSample(const GenericSignal& Input, int el)
{
  if (el >= Input.Elements())
    return 0; //reached end of block, no valid samples

  bool lost = mLostSampleExp.Evaluate(&Input, el);
  if (!lost)
    return el;
  else
    return EvalSample(Input, el + 1);
}

void
InterpolationFilter::getChannelSignal(const GenericSignal& Input, int el, double* data)
{
  for (int ch = 0; ch < Input.Channels(); ch++)
  {
    data[ch] = Input(ch, el);
  }
}
