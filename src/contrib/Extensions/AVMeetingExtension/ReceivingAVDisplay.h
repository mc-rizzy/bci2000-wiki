////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
// Description: An object that displays zoom video on the BCI2000 task screen.
//      This object is a zoom video renderer delegate. It is instantiated
//      outside the Zoom SDK but must be tied to a renderer object using
//      ZOOMSDK::createRenderer().
////////////////////////////////////////////////////////////////////////////////
#ifndef RECEIVING_AV_DISPLAY_H
#define RECEIVING_AV_DISPLAY_H

#include "GraphObject.h"
#include "AVBidirectionalChannel.h"

#include <string>

class ReceivingAVDisplay
    : public GUI::GraphObject,
      public AVBidirectionalChannel::RenderClient
{
public:
    ReceivingAVDisplay(GUI::GraphDisplay&);
    ~ReceivingAVDisplay();
    // Audio volume between 0 and 1
    void SetAudioVolume(float);
    float AudioVolume() const;
    // Audio buffer duration in seconds,
    // used to adjust for clock discrepancies between audio received from network, and playback
    // defaults to 0.04 seconds (40ms)
    void SetAudioBufferSeconds(float);
    float AudioBufferSeconds() const;
    // Text to display when no video has been received for 1s
    void SetBackgroundText(const std::string&);
    const std::string& BackgroundText() const;

protected:
    void OnPaint(const GUI::DrawContext&) override;
    void OnResize(GUI::DrawContext&) override;

protected:
    virtual void OnVideoFrame(int streamIdx, int width, int height, const uint8_t* pData, int64_t frame) override;
    virtual void OnAudioFrame(int streamIdx, int samplingrate, int samples, int channels, const float* pData, int64_t frame) override;

private:
    struct Private;
    Private* p;
};

#endif // RECEIVING_AV_DISPLAY_H
