//////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: Unit tests for Tiny::SelfPipeQueue.
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
#include "SelfPipeQueue.h"

#include "UnitTest.h"
#include "Thread.h"
#include "ThreadUtils.h"

static const int bufsize = 4096;

template <class T> struct WaitingThread : Thread
{
    int OnExecute() override
    {
        while (pQueue->Wait())
        {
            data += pQueue->Pop();
        }
        return 0;
    }
    SelfPipeQueue<T, bufsize>* pQueue;
    std::basic_string<T> data;
};

UnitTest(SelfPipeQueueTest)
{
    unsigned int seed = static_cast<unsigned int>(::time(nullptr));
    ::srand(seed);
    TestReport(seed);

    for (int i = 0; i < 50; ++i)
    {
        SelfPipeQueue<char, bufsize> queue;
        std::string data = Random<std::string>(0.95);

        WaitingThread<char> thread;
        thread.pQueue = &queue;
        thread.Start();
        for (const auto& c : data)
            queue.Push(c);
        std::cout << "." << std::flush;
        ThreadUtils::SleepFor(Time::Seconds(1));
        thread.Terminate();
        TestFail_if(data != thread.data, "Input data differs from output");
    }
    std::cout << std::endl;

    for (int i = 0; i < 50; ++i)
    {
        SelfPipeQueue<wchar_t, bufsize> queue;
        std::wstring data = Random<std::wstring>(0.95);

        WaitingThread<wchar_t> thread;
        thread.pQueue = &queue;
        thread.Start();
        for (const auto& c : data)
            queue.Push(c);
        std::cout << "." << std::flush;
        ThreadUtils::SleepFor(Time::Seconds(1));
        thread.Terminate();
        TestFail_if(data != thread.data, "Input data differs from output");
    }
    std::cout << std::endl;
    ::srand(1);
}
