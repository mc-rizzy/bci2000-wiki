//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: An AVEncoder wrapper that feeds data to the encoder
//   using a queue.
//   Unlike AVEncoder, this class only supports one video and one
//   audio stream.
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
#ifndef AV_BUFFERED_ENCODER_H
#define AV_BUFFERED_ENCODER_H

#include "AVEncoder.h"

class AVBufferedEncoder
{
  public:
    AVBufferedEncoder();
    virtual ~AVBufferedEncoder();

    AVBufferedEncoder(const AVBufferedEncoder&) = delete;
    AVBufferedEncoder& operator=(const AVBufferedEncoder&) = delete;

    // URLs (e.g., RTSP) are also accepted as arguments to OpenFile().
    AVBufferedEncoder& OpenFile(const std::string& file);
     // Call this after all streams have been initialized.
    AVBufferedEncoder& BeginWriting();
    AVBufferedEncoder& CloseFile();

    // For video streams, 32-bit RGB input is assumed.
    AVBufferedEncoder& InitializeVideoStream(
        int width, int height, const Rate& fps,
        int codec = AVEncoder::anyCodec, int preset = 0, int compressionFactor = 8,
        const AVEncoder::Options& = AVEncoder::Options()
    );
    AVBufferedEncoder& WriteVideoFrame(const void*, int bytes, Time::Interval timestamp);

    // For audio streams, float input data is assumed.
    AVBufferedEncoder& InitializeAudioStream(
        int samplesPerBlock, int channels, const Rate& sps,
        int codec = AVEncoder::anyCodec,
        const AVEncoder::Options& = AVEncoder::Options()
    );
    AVBufferedEncoder& WriteAudioFrame(const void*, int bytes, Time::Interval timestamp);

    std::string Error() const;

  private:
    struct Private;
    Private* p;
};

#endif // AV_BUFFERED_ENCODER_H
