////////////////////////////////////////////////////////////////////////////////
// Authors: mellinger@neurotechcenter.org
////////////////////////////////////////////////////////////////////////////////
#ifndef MIRROR_DISPLAY_H
#define MIRROR_DISPLAY_H

#include "GraphObject.h"
#include "AVBidirectionalChannel.h"

#include <string>

class MirrorDisplay : public GUI::GraphObject, public AVBidirectionalChannel::RenderClient
{
public:
    MirrorDisplay(GUI::GraphDisplay&);
    ~MirrorDisplay();
    // Text to display when no video has been received for 1s
    void SetBackgroundText(const std::string&);
    const std::string& BackgroundText() const;

    void OnVideoFrame(int streamIdx, int width, int height, const uint8_t* pData, int64_t frame) override;
    void OnAudioFrame(int streamIdx, int samplingrate, int samples, int channels, const float* pData, int64_t frame) override {}

protected:
    void OnPaint(const GUI::DrawContext&) override;
    void OnResize(GUI::DrawContext&) override;

private:
    struct Private;
    Private* p;
};

#endif // MIRROR_DISPLAY_H
