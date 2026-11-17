//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: A class for bidirectional streaming of AV data in
//   real time.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
///////////////////////////////////////////////////////////////////////
#ifndef AV_BIDIRECTIONAL_CHANNEL_H
#define AV_BIDIRECTIONAL_CHANNEL_H

#include "TimeUtils.h"
#include "Rate.h"

class AVBidirectionalChannel
{
public:
    // RenderClient interface
    class RenderClient {
    public:
        virtual void OnVideoFrame(int streamIdx, int width, int height, const uint8_t* pData, int64_t frame) = 0;
        virtual void OnAudioFrame(int streamIdx, int samplingrate, int samples, int channels, const float* pData, int64_t frame) = 0;
        virtual ~RenderClient() = default;
    };

public:
    AVBidirectionalChannel();
    ~AVBidirectionalChannel();

    AVBidirectionalChannel(const AVBidirectionalChannel&) = delete;
    AVBidirectionalChannel& operator=(const AVBidirectionalChannel&) = delete;

    enum Position { local, remote };
    AVBidirectionalChannel& AddRenderClient(Position, RenderClient*);
    AVBidirectionalChannel& RemoveRenderClient(Position, RenderClient*);

    AVBidirectionalChannel& Open(const std::string& networkAddress);
    AVBidirectionalChannel& Close();

    AVBidirectionalChannel& AddVideoStream(int* pStreamIndex, const std::string& codecName = "");
    // For video streams, 32-bit RGB input is assumed.
    AVBidirectionalChannel& InitializeVideoStream(int streamIndex, int width, int height, const Rate& fps, int preset = 0, int compressionFactor = 8); 

    AVBidirectionalChannel& AddAudioStream(int* pStreamIndex, const std::string& codecName = "");
    // For audio streams, float input data is assumed.
    AVBidirectionalChannel& InitializeAudioStream(int streamIndex, int samplesPerBlock, int channels, const Rate& sps);

    AVBidirectionalChannel& BeginWriting();
    AVBidirectionalChannel& WriteFrame(int streamIndex, const void*, int bytes, Time::Interval timestamp);
    AVBidirectionalChannel& FinalizeStream(int streamIndex);

    std::string Error() const;

  private:
    struct Private;
    Private* p;
};

#endif // AV_BIDIRECTIONAL_CHANNEL_H
