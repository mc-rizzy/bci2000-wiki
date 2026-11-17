////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: ZoomTask implementation
////////////////////////////////////////////////////////////////////////////////
#ifndef AV_MEETING_FILTER_H
#define AV_MEETING_FILTER_H

#include "ApplicationBase.h"
#include "ReceivingAVDisplay.h"
#include "MirrorDisplay.h"

class AVMeetingFilter : public ApplicationBase
{
public:
    AVMeetingFilter();
    ~AVMeetingFilter();
    void Publish() override;
    void Preflight(const SignalProperties& Input, SignalProperties& Output) const override;
    void Initialize(const SignalProperties& Input, const SignalProperties& Output) override;
    void StartRun() override;
    void Process(const GenericSignal& Input, GenericSignal& Output) override;
    void StopRun() override;
    void Halt() override;

private:
    struct Private;
    Private* p;
};

#endif // AV_MEETING_FILTER_H
