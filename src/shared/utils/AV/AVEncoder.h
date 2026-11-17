//////////////////////////////////////////////////////////////////////
// $Id: AVEncoder.h 9402 2026-06-10 13:40:42Z mellinger $
// Authors: mellinger@neurotechcenter.org
// Description: A class that encapsulates writing of video and audio
//    frames into a media file.
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
#ifndef AV_ENCODER_H
#define AV_ENCODER_H

#include "TimeUtils.h"
#include "Rate.h"

#include <map>
#include <string>

class AVEncoder
{
  public:
    typedef std::map<std::string, std::string> Options;

    AVEncoder();
    ~AVEncoder();

    AVEncoder(const AVEncoder&) = delete;
    AVEncoder& operator=(const AVEncoder&) = delete;

    // URLs (e.g., RTSP) are also accepted as arguments to OpenFile().
    AVEncoder& OpenFile(const std::string& file);
    enum { anyCodec = 0, h264 = 1, aac = 100 };
    AVEncoder& AddVideoStream(int* pStreamIndex, int codec = anyCodec);
    // For video streams, 32-bit RGB input is assumed.
    AVEncoder& InitializeVideoStream(int streamIndex, int width, int height, const Rate& fps, int preset = 0, int compressionFactor = 8, const Options& = Options()); 
    AVEncoder& AddAudioStream(int* pStreamIndex, int codec = anyCodec);
    // For audio streams, float input data is assumed.
    AVEncoder& InitializeAudioStream(int streamIndex, int samplesPerBlock, int channels, const Rate& sps, const Options& = Options());
    AVEncoder& BeginWriting();
    AVEncoder& WriteFrame(int streamIndex, const void*, int bytes, Time::Interval timestamp);
    AVEncoder& FinalizeStream(int streamIndex);
    AVEncoder& CloseFile();

    std::string Error() const;

  private:
    struct Private;
    Private* p;
};

#endif // AV_ENCODER_H
