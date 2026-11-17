////////////////////////////////////////////////////////////////////////////////
// $Id$
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
////////////////////////////////////////////////////////////////////////////////
#include "SignalSharingClient.h"

#include "GenericSignal.h"
#include "State.h"
#include "ParamList.h"
#include "Runnable.h"
#include "Sockets.h"
#include "Streambuf.h"
#include "MessageChannel.h"
#include "GenericVisualization.h"
#include "SysCommand.h"
#include "StringUtils.h"
#include "SynchronizedObject.h"
#include "Thread.h"
#include "SelfPipeQueue.h"

#include <atomic>

struct SignalSharingClient::Private : MessageChannel
{
    ServerTCPSocket mListeningSocket;
    ClientTCPSocket mClientSocket;
    std::atomic<bool> mConnected;
    SynchronizedObject<GenericSignal> mpSignal, mpStates;
    SynchronizedObject<ParamList> mpParamList;
    double mSamplingRate;
    SelfPipeQueue<char, 128> mNotifier;

    Thread mThread;
    void ThreadFunc();
    MemberCall<void(Private *)> mThreadCall;

    UnbufferedIO mBuffer;
    // MessageChannel overrides
    bool OnVisSignalProperties(std::istream&) override;
    bool OnVisSignal(std::istream&) override;
    bool OnParam(std::istream&) override;
    bool OnSysCommand(std::istream&) override;

    Private();
};

SignalSharingClient::Private::Private()
    : MessageChannel(mBuffer),
      mThreadCall(&Private::ThreadFunc, this),
      mThread(&mThreadCall, "SignalSharingClient listening/receiving thread"),
      mSamplingRate(0.0)
{
    mBuffer.SetInput(&mClientSocket.Input());
    mBuffer.SetOutput(&mClientSocket.Output());
}

void SignalSharingClient::Private::ThreadFunc()
{
    while (!mThread.Terminating())
    { // wait for a connection
        while (mListeningSocket.Input().Wait())
        { // accept pending connection
            if (mListeningSocket.WaitForAccept(mClientSocket, 0))
            {
                mConnected = true;
                MessageChannel::ClearIOState();
                while (mClientSocket.Input().Wait()) // will be interrupted by Thread::Terminate()
                {
                    MessageChannel::HandleMessage(); // will dispatch to our overridden functions
                                                     // as appropriate
                }
                *mpSignal.Mutable() = GenericSignal();
                mConnected = false;
                mNotifier.Push(SignalSharingClient::none);
            }
        }
    }
}

bool SignalSharingClient::Private::OnVisSignalProperties(std::istream& is)
{
    VisSignalProperties properties; // signal properties are wrapped into VisSignalProperties
    properties.Unserialize(is);
    if (properties.VisID() == "Signal") { // ignore state properties
        // The only information we need from signal properties is the sampling rate
        // because the signal's dimensions will be transported by signal messages as well.
        double sampleDuration = properties.SignalProperties().ElementUnit().RawToPhysicalValue(1);
        mSamplingRate = 1.0 / sampleDuration;
    }
    return true; // indicate we read our data from the stream
}

bool SignalSharingClient::Private::OnVisSignal(std::istream& is)
{
    VisSignal visSignal; // signals are wrapped into VisSignal messages
    visSignal.Unserialize(is);
    if (visSignal.VisID() == "Signal") {
        mpSignal.Mutable()->AssignValues(visSignal.Signal()); // get current signal content
        mNotifier.Push(SignalSharingClient::signal);
    }
    else if (visSignal.VisID() == "States") {
        mpStates.Mutable()->AssignValues(visSignal.Signal());
        mNotifier.Push(SignalSharingClient::states);
    }
    return true; // indicate we read our data from the stream
}

bool SignalSharingClient::Private::OnParam(std::istream& is)
{
    Param param;
    param.Unserialize(is);
    mpParamList.Mutable()->Add(param);
    return true; // indicate we read our data from the stream
}

bool SignalSharingClient::Private::OnSysCommand(std::istream& is)
{
    SysCommand sysCommand;
    sysCommand.Unserialize(is);
    if (sysCommand == SysCommand::EndOfParameter)
        mNotifier.Push(SignalSharingClient::parameters);
    return true;
}

SignalSharingClient::SignalSharingClient(const std::string& address)
: p(new Private)
{
    p->mListeningSocket.Open(address);
    if (!p->mListeningSocket.Listening())
        throw std_runtime_error << "Cannot listen on " << address;
    p->mThread.Start();
}

SignalSharingClient::~SignalSharingClient()
{
    delete p;
}

std::string SignalSharingClient::ListeningAddress() const
{
    return p->mListeningSocket.LocalAddress();
}

int SignalSharingClient::WaitForData(int timeoutMs)
{
    auto timeout = TimeUtils::TimeInterval::FromIntTimeout(timeoutMs);
    if (p->mNotifier.Wait(timeout))
        return p->mNotifier.Pop();
    return none;
}

const GenericSignal& SignalSharingClient::Signal() const
{
    return *p->mpSignal;
}

const GenericSignal& SignalSharingClient::States() const
{
    return *p->mpStates;
}

const ParamList& SignalSharingClient::Parameters() const
{
    return *p->mpParamList;
}
