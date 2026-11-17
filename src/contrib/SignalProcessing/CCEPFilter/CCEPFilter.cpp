////////////////////////////////////////////////////////////////////////////////
// Authors: Will Engelhardt
// Description: CCEPFilter implementation
////////////////////////////////////////////////////////////////////////////////

#include "CCEPFilter.h"
#include "BCIStream.h"
//#include "BitmapImage.h"
#include "WorkerThread.h"
#include "GenericVisualization.h"
#include <iomanip>

using namespace std;
RegisterFilter( CCEPFilter, 2.A );


CCEPFilter::CCEPFilter()
: mVisualize(false),
  mNumChannels(0),
  mStimulator(nullptr),
  mEpochSums(nullptr),
  mBaselineSums(nullptr),
  mBaselineBuffer(nullptr)
{
}

CCEPFilter::~CCEPFilter()
{
  if (mStimulator != nullptr)
  {
    delete mStimulator;
    mStimulator = nullptr;
  }
  Halt();
}

void
CCEPFilter::Publish()
{
 BEGIN_PARAMETER_DEFINITIONS
   //General configuration settings
   "Filtering:CCEP%20General%20Settings string OnsetExpression= 0 % % % "
      "// Expression that defines stimulus onset",

   //Duration parameters
   "Filtering:CCEP%20Duration%20Settings int BaselineEpochLength= 50ms 50ms 0 % "
      "// Length of data epoch before stimulus onset",
   "Filtering:CCEP%20Duration%20Settings int CCEPEpochLength= 500ms 500ms 0 % "
      "// Length of data epoch after stimulus onset",
   //Software trigger parameters
   "Filtering:CCEP%20Trigger%20Settings int SoftwareTrigger= 0 0 0 1 "
   "// Enable if using software trigger (instead of hardware) (boolean) ",
   "Filtering:CCEP%20Trigger%20Settings int PreStimLength= 0 0 0 % "
   "// Length of data before stimulation train",
   "Filtering:CCEP%20Trigger%20Settings int PostStimLength= 0 0 0 % "
   "// If high frequency stim, the length of the train",

   //Quantity parameters
   "Filtering:CCEP%20Counting%20Settings int TriggerPeriodEnd= 0 0 0 1 "
      "// Enable to trigger at end of period instead of start (boolean) ",
   "Filtering:CCEP%20Counting%20Settings int EpochsToAverage= 1 1 0 % "
      "// Number of epochs to average",
   "Filtering:CCEP%20Counting%20Settings int OnsetPeriod= 1 1 % % "
      "// Only count every number of onset expressions ",

 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
   "CCEPTriggered 1 0 0 0",
   "StimulatingChannel 32 0 0 0",
   "ResetOnsetPeriod 8 0 0 0"
 END_STATE_DEFINITIONS


}

void
CCEPFilter::AutoConfig(const SignalProperties& Input)
{
  mStimulator = new StimulatorConfiguration();
  Parameter("CCEPEpochLength") = "500ms";
  Parameter("BaselineEpochLength") = "50ms";
}

void
CCEPFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  Output = Input; // this simply passes information through about SampleBlock dimensions, etc....

  //stimulation parameters
  mStimulator->checkParameters(Input);

  Expression onsetExpression = Parameter("OnsetExpression").ToString();
  GenericSignal input(Input);
  onsetExpression.Evaluate(&input, 0);
  Expression t = Expression(Parameter("OnsetExpression"));
  t.Compile();

  int epochsToAverage = Parameter("EpochsToAverage");

  bool softTrig = Parameter("SoftwareTrigger");
  string preName = softTrig ? "PreStimLength" : "BaselineEpochLength";
  string postName = softTrig ? "PostStimLength" : "CCEPEpochLength";

  double baselineSamples = Parameter(preName).InSampleBlocks();
  double outputSamples = Parameter(postName).InSampleBlocks();
  //double outputSamples = mStimulator->getStimulationLength();
  outputSamples *= Input.Elements();
  baselineSamples *= Input.Elements();
  outputSamples = ::ceil(outputSamples);
  baselineSamples = ::ceil(baselineSamples);
  bciout << "Number of samples " << outputSamples << std::endl;
  bciout << "Number of baseline samples " << baselineSamples << std::endl;
  // Requested output signal properties.
  Output = Input;
  Output.SetChannels(Input.Channels())
    .SetElements(static_cast<size_t>(outputSamples + baselineSamples))
    .SetType(SignalType::float32)
    .SetIsStream(false)
    .ElementUnit()
    .SetRawMin(0)
    .SetRawMax(outputSamples + baselineSamples - 1);

  Parameter("TriggerPeriodEnd");
  Parameter("OnsetPeriod");
  Parameter("SamplingRate");

  State("CCEPTriggered");
  State("StimulatingChannel");
  State("ResetOnsetPeriod");
}


void
CCEPFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  mEpochs.clear();
  mBaselineEpochs.clear();
  if (mEpochSums != nullptr)
  {
    delete mEpochSums;
    mEpochSums = nullptr;
  }
  if (mBaselineSums != nullptr)
  {
    delete mBaselineSums;
    mBaselineSums = nullptr;
  }

  mOnsetExpression = Expression(Parameter("OnsetExpression"));
  mOnsetExpression.Compile();

  mOutputProperties = Output;
  mEpochsToAverage = Parameter("EpochsToAverage");

  //choose parameters differently based on soft trigger
  bool softTrig = Parameter("SoftwareTrigger");
  string preName = softTrig ? "PreStimLength" : "BaselineEpochLength";
  string postName = softTrig ? "PostStimLength" : "CCEPEpochLength";

  double baselineSamples = Parameter(preName).InSampleBlocks();
  baselineSamples *= Input.Elements();
  mBaselineSamples = ::ceil(baselineSamples);

  //initialize stimulator with set parameters
  mStimulator->initializeStimulator();

  if (mBaselineBuffer != nullptr)
  {
    delete mBaselineBuffer;
    mBaselineBuffer = nullptr;
  }
  mBaselineProperties = mOutputProperties;
  mBaselineProperties.SetElements(mBaselineSamples);

  mEpochProperties = mOutputProperties;
  double outputSamples = Parameter(postName).InSampleBlocks();
  outputSamples *= Input.Elements();
  outputSamples = ::ceil(outputSamples);
  mEpochProperties.SetElements(outputSamples);

  //mBaselineBuffer = new EpochBuffer(sp, 0);
  mBaselineBuffer = new ReverseBuffer(mBaselineProperties);

  //let onset period be number of time to skip
  string onset = Parameter("OnsetPeriod").ToString();
  bool isNum = true;
  for (int i = 0; i < onset.length(); i++)
    if (isdigit(onset[i]) == false)
      //unit exists
      isNum = false;

  mTrigPeriodStart = !(bool)Parameter("TriggerPeriodEnd").ToNumber();
  if (isNum)
  {
    mOnsetPeriod = Parameter("OnsetPeriod").ToNumber();
    mSkipSamples = 0;
  }
  else
  {
    //has unit, therefore it is a time value
    mOnsetPeriod = 1;
    mSkipSamples = Parameter("OnsetPeriod").InSeconds() * Parameter("SamplingRate").InHertz();
  }
  bciout << "Skipping every " << mSkipSamples << " samples" << endl;


  mAllData = GenericSignal(mOutputProperties);
}

void
CCEPFilter::StartRun()
{
  // The user has just pressed "Start" (or "Resume")
  mPreviousExpressionValue = false;
  mEpochs.clear();
  mBaselineEpochs.clear();
  if (mEpochSums != nullptr)
  {
    delete mEpochSums;
    mEpochSums = nullptr;
  }
  if (mBaselineSums != nullptr)
  {
    delete mBaselineSums;
    mBaselineSums = nullptr;
  }

  mOnsetCounter = 0; 
  mSkipCounter = 0;
  mEpochMap.clear();

  State("CCEPTriggered") = 0;
  State("StimulatingChannel") = 0;
  State("ResetOnsetPeriod") = mOnsetPeriod;
}


void
CCEPFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{

  if (mEpochsToAverage > 0)
  {
    set<int> stimulusOnsets;
    for (int i = 0; i < Statevector->Samples() - 1; ++i)
    {
      //watch for end of counter if triggering end of period
      if (mSkipCounter == 1 && !mTrigPeriodStart)
      {
        stimulusOnsets.insert(i);
      }

      if (mSkipCounter != 0)
        mSkipCounter--;
      else {
        bool value = mOnsetExpression.Evaluate(&Input, i);
        if (value && !mPreviousExpressionValue)
        {
          if (mTrigPeriodStart || (!mTrigPeriodStart && mSkipSamples == 0))
            stimulusOnsets.insert(i);
          mSkipCounter = mSkipSamples;
        }
        mPreviousExpressionValue = value;
      }
    }

    if (!stimulusOnsets.empty())
    {
      for (auto it = stimulusOnsets.begin(); it != stimulusOnsets.end(); it++)
      {
        mOnsetCounter++;
        if ((mOnsetCounter - mTrigPeriodStart) % mOnsetPeriod == 0) //only save a periodic number of onsets
        {
          mOnsetCounter = mTrigPeriodStart;
          // First block of stimulus presentation -- create a new epoch buffer.
          EpochBuffer* eBuf = new EpochBuffer(mEpochProperties, *it);
          mEpochs.insert(eBuf);
          auto s = mStimulator->getChannels(&Input);
          uint32_t stimChs = 0;
          for (auto it2 = s.begin(); it2 != s.end(); it2++) {
            stimChs += pow(2, *it2); //encode by bits
          }
          State("StimulatingChannel") = stimChs;

          //mBaselineBuffer->Finalize(Input, *it);
          ReverseBuffer* finalBuf = new ReverseBuffer(*mBaselineBuffer); //make a copy (hopefully deep?)
          finalBuf->Finalize(Input, *it);
          mEpochMap[eBuf] = finalBuf;
        }
      }
    }
    mBaselineBuffer->Process(Input);

    if (!mEpochs.empty())
    {
      EpochSet obsoleteEpochs;
      ReverseEpochSet obsoleteBaselineEpochs;
      for (auto& pBuffer : mEpochs)
      {
        pBuffer->Process(Input);
        if (pBuffer->EpochDone())
        { // Move buffer data into the epoch sum associated with the stimulus code,
            // and note the epoch buffer for later disposal.
          bcidbg << "Epoch done";
          if (mEpochSums == nullptr)
          {
            bcidbg << "Allocating result buffer";
            mEpochSums = new DataSum(mEpochProperties);
          }
          mEpochSums->Add(pBuffer->Data());
          obsoleteEpochs.insert(pBuffer);
          //baseline
          auto bBuf = mEpochMap.at(pBuffer);
          GenericSignal d = bBuf->PopData();
          obsoleteBaselineEpochs.insert(bBuf);
          //GenericSignal d = mBaselineBuffer->PopData();
          if (mBaselineSums == nullptr)
          {
            bcidbg << "Allocating baseline buffer";
            mBaselineSums = new DataSum(d.Properties());
          }
          mBaselineSums->Add(d);
          if (mEpochSums->Count() == mEpochsToAverage)
          { // When the number of required epochs is reached, copy the buffer average
            // into the output signal, and set states appropriately.
            for (int channel = 0; channel < Output.Channels(); ++channel)
            {
              for (int sample = 0; sample < mBaselineSamples; ++sample)
              {
                mAllData(channel, sample) = (*mBaselineSums)(channel, sample) / mEpochsToAverage;
              }
              for (int sample = mBaselineSamples; sample < Output.Elements(); ++sample)
              {
                mAllData(channel, sample) = (*mEpochSums)(channel, sample - mBaselineSamples) / mEpochsToAverage;
              }
            }
            //CCEP can only be triggered at most once per block
            State("CCEPTriggered")(0) = 1;
            State("CCEPTriggered")(1) = 0;
          }
        }
      }
      for (auto& pBuffer : obsoleteEpochs)
        mEpochs.erase(pBuffer);
      for (auto& pBuffer : obsoleteBaselineEpochs)
        mBaselineEpochs.erase(pBuffer);
      // Epochs will be deallocated from the obsoleteEpochs destructor.
    }

    if (mEpochSums != nullptr && mEpochSums->Count() >= mEpochsToAverage)
    { // Reset the data sum buffer.
      bcidbg << "Clearing buffer";
      *mEpochSums = DataSum(mEpochProperties);
      *mBaselineSums = DataSum(mBaselineProperties);
    }
  }
  Output = mAllData;
}

void
CCEPFilter::StopRun()
{
}

void
CCEPFilter::Halt()
{

}