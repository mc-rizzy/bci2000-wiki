/////////////////////////////////////////////////////////////////////////////
// $Id$
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
#include "RefLogger.h"

#include "BCIEvent.h"
#include "BCIStream.h"

Extension(RefLogger);

RefLogger::RefLogger()
: mRefLoggerFrequency(0)
{
}

RefLogger::~RefLogger()
{
    Halt();
}

void RefLogger::Publish()
{
    bool enabled = (OptionalParameter("RefLoggerFrequency", 0) > 0);
    if (enabled)
    {
      BEGIN_PARAMETER_DEFINITIONS
        "Source float /RefLoggerFrequency= 100Hz % % % // Frequency of emission for bcievents",
      END_PARAMETER_DEFINITIONS

      BEGIN_EVENT_DEFINITIONS
         "RefTime 32 0 0 0",
      END_EVENT_DEFINITIONS
    }
}

void RefLogger::Preflight() const
{
    float RefLoggerFrequency = OptionalParameter("RefLoggerFrequency", 0).InHertz();
    PreflightCondition(RefLoggerFrequency >= 0);
    if (RefLoggerFrequency > 1000)
      bciwarn << "RefLogger period is too small for system accuracy, frequency will be around 1kHz";
}

void RefLogger::Initialize()
{
    mRefLoggerFrequency = OptionalParameter("RefLoggerFrequency", 0).InHertz();
    if (mRefLoggerFrequency > 0)
    {
        State("RefTime") = uint32_t(-1);
    }
}

void RefLogger::StartRun()
{
    if (mRefLoggerFrequency > 0)
    {
        State("RefTime") = uint32_t(-1);
        Thread::Start();
    }
}

void RefLogger::StopRun()
{
    if (mRefLoggerFrequency > 0)
    {
        Thread::Terminate();
        State("RefTime") = uint32_t(-1);
    }
}

void RefLogger::Halt()
{
    Thread::Terminate();
}

int RefLogger::OnExecute()
{
    while (!Thread::Terminating())
    {
        PrecisionTime now = PrecisionTime::Now();
        bcievent << "RefTime " << now << " 0" << std::flush;
        PrecisionTime wakeupTime = now + static_cast<int>(1000 / mRefLoggerFrequency);
        ThreadUtils::SleepUntil(wakeupTime);
    }
    return 0;
}

