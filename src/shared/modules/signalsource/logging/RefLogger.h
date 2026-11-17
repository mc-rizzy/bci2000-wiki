/////////////////////////////////////////////////////////////////////////////
// Author: mellinger@neurotechcenter.org
// Description: A logger component that writes time stamps into a reference
//   event. By comparing these time stamps to the sample position inside
//   the current block, and the block's SourceTime stamp, precision of event
//   logging can be established in offline analysis.
//   To activate the RefLogger, set the RefLoggerFrequency parameter from the
//   command line:
//   SignalGenerator --RefLoggerFrequency=250Hz
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
#ifndef REF_LOGGER_H
#define REF_LOGGER_H

#include "Environment.h"
#include "Thread.h"
#include <cinttypes>

class RefLogger : public EnvironmentExtension, private Thread
{
  public:
    RefLogger();
    ~RefLogger();

    // EnvironmentExtension interface
    void Publish() override;
    void Preflight() const override;
    void Initialize() override;
    void StartRun() override;
    void StopRun() override;
    void Halt() override;

    // Thread interface
    int OnExecute() override;

  private:
    double mRefLoggerFrequency;
};

#endif // REF_LOGGER_H
