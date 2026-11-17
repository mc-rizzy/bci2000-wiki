/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: The screen capture logger logs screen content, and writes
//   it into one or more video files. Also, it logs the current frame
//   number into an event state.
//
// Parameterization:
//   Screen capture logging is enabled from the command line adding
//     --LogScreenCapture=1
//
// Event Variables:
//   ScreenFrame<n> - The current frame number for capture index n 
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
/////////////////////////////////////////////////////////////////////////////
#ifndef SCREENCAPTURE_LOGGER_H
#define SCREENCAPTURE_LOGGER_H

#include <vector>
#include <memory>

#include "Environment.h"
#include "ScreenCaptureThread.h"

class ScreenCaptureLogger : public EnvironmentExtension
{
public:
  ScreenCaptureLogger();
  ~ScreenCaptureLogger();
  void Publish() override;
  void Preflight() const override;
  void Initialize() override;
  void StartRun() override;
  void StopRun() override;
  void Process() override;
  void Halt() override;
  void Halt2() override { Halt(); }

private:
  static std::unique_ptr<ScreenCaptureThread> CreateScreenCapture(const ParamRef&, int row);
  typedef std::vector<std::unique_ptr<ScreenCaptureThread>> CaptureThreadContainer;
  CaptureThreadContainer mScreenCaptures;
  int mFilePart = 0;
};

#endif // SCREENCAPTURE_LOGGER_H
