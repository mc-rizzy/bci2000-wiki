////////////////////////////////////////////////////////////////////////////////
// Authors: juergen@VBox-Win10.fritz.box
// Description: BufferedADCTestADC header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_BUFFEREDADCTESTADC_H  // makes sure this header is not included more than once
#define INCLUDED_BUFFEREDADCTESTADC_H

#include "BufferedADC.h"
#include "Clock.h"

class BufferedADCTestADC : public BufferedADC
{
public:
    BufferedADCTestADC();
    ~BufferedADCTestADC();
    void OnPublish() override;
    void OnAutoConfig() override;
    void OnPreflight(SignalProperties& Output) const override;
    void OnInitialize(const SignalProperties& Output) override;
    void OnStartAcquisition() override;
    void DoAcquire(GenericSignal& Output) override;
    void OnStopAcquisition() override;

private:
    int mNumberOfSignalChannels;
    float mSamplingRate, mTimeSeconds;
    Clock mClock;
};

#endif // INCLUDED_BUFFEREDADCTESTADC_H
