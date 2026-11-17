//////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: Unit tests for Tiny::Thread.
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
#include "Thread.h"
#include "ThreadUtils.h"

#include "UnitTest.h"

static const double cWaitDuration = 0.1; // seconds

struct SelfDeletingThread : Thread
{
    SelfDeletingThread()
        : Thread(true)
    {
        sHasBeenRunning = false;
        sHasBeenDeleted = false;
    }
    ~SelfDeletingThread()
    {
        sHasBeenDeleted = true;
    }
    int OnExecute() override
    {
        ThreadUtils::TimeSlice timeSlice(Time::FromMilliseconds(1));
        sHasBeenRunning = true;
        ThreadUtils::SleepFor(Time::Seconds(cWaitDuration));
        return 0;
    }
    static std::atomic<bool> sHasBeenDeleted, sHasBeenRunning;
};
std::atomic<bool> SelfDeletingThread::sHasBeenDeleted = false;
std::atomic<bool> SelfDeletingThread::sHasBeenRunning = false;

struct ClockThread : Thread
{
    int OnExecute() override
    {
        std::cout << std::endl;
        WaitableTimeout clock;
        while (clock.Wait(Time::OneSecond()))
            std::cout << '.' << std::flush;
        std::cout << std::endl;
        return 0;
    }
};

UnitTest(ThreadTest)
{
    for (int i = 0; i < 20; ++i)
    {
        SelfDeletingThread* pThread = new SelfDeletingThread;
        auto tBegin = TimeUtils::MonotonicTime();
        pThread->Start();
        pThread->Wait(); // this is inherently dangerous with a self-deleting thread, it might have deleted itself already
        auto tEnd = TimeUtils::MonotonicTime();
        ThreadUtils::SleepFor(Time::Seconds(cWaitDuration));
        double duration = (tEnd - tBegin).Seconds();
        double relativeDuration = ::fabs((duration - cWaitDuration) / cWaitDuration);
        TestFail_if(relativeDuration > 0.3, "Thread duration deviates by more than 30%");
        TestFail_if(!SelfDeletingThread::sHasBeenRunning, "Thread failed to run");
        TestFail_if(!SelfDeletingThread::sHasBeenDeleted, "Thread failed to delete itself");
    }

    ClockThread clockThread;
    clockThread.Start();
    ThreadUtils::SleepFor(Time::Seconds(10));
    clockThread.Terminate();
    TestFail_if(clockThread.Running(), "Thread still running after Terminate()");
}
