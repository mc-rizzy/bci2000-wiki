//////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: Unit tests for Tiny::Semaphore.
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
#include "TinySemaphore.h"

#include "UnitTest.h"
#include "Thread.h"

struct WaitingThread : Thread
{
    int OnExecute() override
    {
        return pSem->Wait();
    }
    Semaphore* pSem;
};

UnitTest(SemaphoreTest)
{
    for (int i = 0; i < 1000; ++i)
    {
        Semaphore sem(0);
        sem.Increase();
        TestFail_if(!sem.Test(), "Semaphore increased but does not test true");
        sem.Decrease();
        TestFail_if(sem.Test(), "Semaphore decreased but does not test false");

        for (int i = 0; i < 20; ++i)
            sem.Increase();
        TestFail_if(!sem.Test(), "Semaphore increased but does not test true");
        for (int i = 0; i < 20; ++i)
            sem.Decrease();
        TestFail_if(sem.Test(), "Semaphore decreased but does not test false");

        WaitingThread thread;
        thread.pSem = &sem;
        thread.Start();
        sem.Increase();
        thread.Terminate();
        TestFail_if(!thread.Result(), "Semaphore increase did not trigger waiting thread");
    }
}
