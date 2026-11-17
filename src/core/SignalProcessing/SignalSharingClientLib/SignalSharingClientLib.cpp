/*********************************************************************
 * $Id$
 * Author:      mellinger@neurotechcenter.org
 * Description: C-style interface to the BCI2000 SignalSharingClient
 *   library.
 *   You can use this library to connect to the BCI2000 SignalSharing
 *   interface in order to exchange signal and state data
 *   with a BCI2000 system.
 *
 * $BEGIN_BCI2000_LICENSE$
 *
 * This file is part of BCI2000, a platform for real-time bio-signal research.
 * [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
 *
 * BCI2000 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * BCI2000 is distributed in the hope that it will be useful, but
 *                         WITHOUT ANY WARRANTY
 * - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_BCI2000_LICENSE$
 *********************************************************************/
#include "SignalSharingClientLib.h"

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

#include <atomic>
#include <mutex>

namespace {

// This function builds a vector of C strings from a vector of std::strings.
// The lifetime of the C string vector must not exceed that of the std::strings vector.
// Also, the C string vector must be rebuilt on any change in the std::strings vector.
std::vector<const char*> BuildCStrings(const std::vector<std::string>& stringVector)
{
    std::vector<const char*> result;
    for (const auto& s : stringVector)
        result.push_back(s.c_str());
    return result;
}

class SignalSharingConnector : MessageChannel
{
public:
    SignalSharingConnector();

    int Listen(const std::string&);
    void Start();
    void Stop();

    void SetSignalCallback(SSC_CALLBACK, void*);
    void SetStatesCallback(SSC_CALLBACK, void*);
    void SetParametersCallback(SSC_CALLBACK, void*);

    void LockSignal(bool);
    void LockStates(bool);
    void LockParameters(bool);

    bool GetSignal(const double**, int*, int*, const char* const**) const;
    bool GetStates(const double**, int*, int*, const char* const**) const;
    bool GetParameters(const char* const**, int*) const;

    bool PushSignal(const double*);
    bool PushState(const std::string&, uint32_t);
    bool Commit();

private:
    ServerTCPSocket mListeningSocket;
    ClientTCPSocket mClientSocket;

    mutable std::shared_mutex mSignalMutex, mStatesMutex, mParametersMutex;

    GenericSignal mSignal, mStates;
    std::vector<std::string> mSignalLabelStrings, mStateLabelStrings;
    std::vector<const char*> mSignalLabels, mStateLabels;

    ParamList mParamList;
    std::vector<std::string> mParameterLineStrings;
    std::vector<const char*> mParameterLines;

    struct Callback
    {
        SSC_CALLBACK func;
        void* refdata;
    };
    SynchronizedObject<Callback> mpSignalCallback, mpStatesCallback, mpParametersCallback;

    Thread mThread;
    void ThreadFunc();
    MemberCall<void(SignalSharingConnector*)> mThreadCall;

    UnbufferedIO mBuffer;
    // MessageChannel overrides
    bool OnVisSignalProperties(std::istream&) override;
    bool OnVisSignal(std::istream&) override;
    bool OnParam(std::istream&) override;
    bool OnSysCommand(std::istream&) override;
};

SignalSharingConnector::SignalSharingConnector()
  : MessageChannel(mBuffer),
    mThreadCall(&SignalSharingConnector::ThreadFunc, this),
    mThread(&mThreadCall, "SignalSharingClientLib listening/receiving thread")
{
    mBuffer.SetInput(&mClientSocket.Input());
    mBuffer.SetOutput(&mClientSocket.Output());
}

int SignalSharingConnector::Listen(const std::string& address)
{
    mListeningSocket.Open(address);
    if (!mListeningSocket.IsOpen()) {
        return mListeningSocket.LastErrorCode();
    }
    return 0;
}

void SignalSharingConnector::Start()
{
    mThread.StartIfNotRunning();
}

void SignalSharingConnector::Stop()
{
    mThread.Terminate();
}

void SignalSharingConnector::SetSignalCallback(SSC_CALLBACK func, void* refdata)
{
    WithLocked(p = mpSignalCallback.Mutable()) {
        p->func = func;
        p->refdata = refdata;
    }
}

void SignalSharingConnector::SetStatesCallback(SSC_CALLBACK func, void* refdata)
{
    WithLocked(p = mpStatesCallback.Mutable()) {
        p->func = func;
        p->refdata = refdata;
    }
}

void SignalSharingConnector::SetParametersCallback(SSC_CALLBACK func, void* refdata)
{
    WithLocked(p = mpParametersCallback.Mutable()) {
        p->func = func;
        p->refdata = refdata;
    }
}

#pragma push
#pragma warning (disable : 26115)
#pragma warning (disable : 26117)
void SignalSharingConnector::LockSignal(bool b)
{
    if (b)
        mSignalMutex.lock_shared();
    else
        mSignalMutex.unlock_shared();
}

void SignalSharingConnector::LockStates(bool b)
{
    if (b)
        mStatesMutex.lock_shared();
    else
        mStatesMutex.unlock_shared();
}

void SignalSharingConnector::LockParameters(bool b)
{
    if (b)
        mParametersMutex.lock_shared();
    else
        mParametersMutex.unlock_shared();
}
#pragma pop

bool SignalSharingConnector::GetSignal(const double** pData, int* channels, int* samples, const char* const** pChannelNames) const
{
    if (pData)
        *pData = mSignal.ConstData();
    if (channels)
        *channels = mSignal.Channels();
    if (samples)
        *samples = mSignal.Elements();
    if (pChannelNames)
        *pChannelNames = mSignalLabels.data();
    return true;
}

bool SignalSharingConnector::GetStates(const double** pData, int* states, int* samples, const char* const** pStateNames) const
{
    if (pData)
        *pData = mStates.ConstData();
    if (states)
        *states = mStates.Channels();
    if (samples)
        *samples = mStates.Elements();
    if (pStateNames)
        *pStateNames = mStateLabels.data();
    return true;
}

bool SignalSharingConnector::GetParameters(const char* const** pLines, int* count) const
{
    if (pLines)
        *pLines = mParameterLines.data();
    if (count)
        *count = mParameterLines.size();
    return true;
}

bool SignalSharingConnector::PushSignal(const double* signal)
{
    GenericSignal s(mSignal.Properties());
    auto* p = s.MutableData(GenericSignal::intentOverwriteContents);
    std::memcpy(p, signal, s.Channels() * s.Elements() * sizeof(GenericSignal::ValueType));
    return MessageChannel::Send(s);
}

bool SignalSharingConnector::PushState(const std::string& name, uint32_t value)
{
    State s;
    s.FromDefinition(name + " 32 " + std::to_string(value) + " 0 0");
    return MessageChannel::Send(s);
}

bool SignalSharingConnector::Commit()
{
    return MessageChannel::Send(SysCommand::EndOfData);
}

void SignalSharingConnector::ThreadFunc()
{
    while (!mThread.Terminating())
    { // wait for a connection
        while (mListeningSocket.Input().Wait())
        { // accept pending connection
            if (mListeningSocket.WaitForAccept(mClientSocket, 0))
            {
                MessageChannel::ClearIOState();
                while (mClientSocket.Input().Wait()) // will be interrupted by Thread::Terminate()
                {
                    MessageChannel::HandleMessage(); // will dispatch to our overridden functions as appropriate
                }
                std::unique_lock lock(mSignalMutex);
                mSignal = GenericSignal();
            }
        }
    }
}

bool SignalSharingConnector::OnVisSignalProperties(std::istream& is)
{
    VisSignalProperties properties; // signal properties are wrapped into VisSignalProperties
    properties.Unserialize(is);
    const auto& labelList = properties.SignalProperties().ChannelLabels();
    std::vector<std::string> labels;
    for (int i = 0; i < labelList.Size(); ++i) {
        labels.push_back(labelList[i]);
    }
    if (properties.VisID() == "Signal") {
        std::unique_lock lock(mSignalMutex);
        mSignalLabelStrings = labels;
        mSignalLabels = BuildCStrings(mSignalLabelStrings);
    }
    else if (properties.VisID() == "States") {
        std::unique_lock lock(mStatesMutex);
        mStateLabelStrings = labels;
        mStateLabels = BuildCStrings(mStateLabelStrings);
    }
    return true; // indicate we read our data from the stream
}

bool SignalSharingConnector::OnVisSignal(std::istream& is)
{
    VisSignal visSignal; // signals are wrapped into VisSignal messages
    visSignal.Unserialize(is);
    if (visSignal.VisID() == "Signal") {
        std::unique_lock lock(mSignalMutex);
        mSignal.AssignValues(visSignal.Signal()); // get current signal content
        lock.unlock();

        WithLocked(p = mpSignalCallback.Const()) {
            if (p->func)
                p->func(p->refdata);
        }
    }
    else if (visSignal.VisID() == "States") {
        std::unique_lock lock(mStatesMutex);
        mStates.AssignValues(visSignal.Signal());
        lock.unlock();

        WithLocked(p = mpStatesCallback.Const()) {
            if (p->func)
                p->func(p->refdata);
        }
    }
    return true; // indicate we read our data from the stream
}

bool SignalSharingConnector::OnParam(std::istream& is)
{
    Param param;
    param.Unserialize(is);
    std::unique_lock lock(mParametersMutex);
    mParamList.Add(param);
    return true; // indicate we read our data from the stream
}

bool SignalSharingConnector::OnSysCommand(std::istream& is)
{
    SysCommand sysCommand;
    sysCommand.Unserialize(is);
    if (sysCommand == SysCommand::EndOfParameter) {
        std::unique_lock lock(mParametersMutex);
        mParameterLines.clear();
        mParameterLineStrings.clear();
        for (const auto& param : mParamList)
            mParameterLineStrings.push_back(param.ToDefinition());
        mParameterLines = BuildCStrings(mParameterLineStrings);

        WithLocked(p = mpParametersCallback.Const()) {
            if (p->func)
                p->func(p->refdata);
        }
    }
    return true;
}

} // namespace

SSC_RESULT STDCALL SSC_CreateConnector(const char* listeningAddress, SSC_HANDLE* pHandle)
{
    if (!pHandle)
        return SSC_INVALID_ARG;
    *pHandle = nullptr;
    auto* pConnector = new SignalSharingConnector;
    int err = pConnector->Listen(listeningAddress);
    if (err) {
        delete pConnector;
        return SSC_CANNOT_LISTEN;
    }
    *pHandle = pConnector;
    pConnector->Start();
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_DeleteConnector(SSC_HANDLE handle)
{
    if (!handle)
        return SSC_INVALID_ARG;
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    pConnector->Stop();
    delete pConnector;
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_RegisterSignalCallback(SSC_HANDLE handle, SSC_CALLBACK callback, void* refdata)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->SetSignalCallback(callback, refdata);
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_RegisterStatesCallback(SSC_HANDLE handle, SSC_CALLBACK callback, void* refdata)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->SetStatesCallback(callback, refdata);
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_RegisterParametersCallback(SSC_HANDLE handle, SSC_CALLBACK callback, void* refdata)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->SetParametersCallback(callback, refdata);
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_LockSignal(SSC_HANDLE handle, const double** pSignal, int* channels, int* samples, const char* const** pChannelNames)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->LockSignal(true);
    if (!pConnector->GetSignal(pSignal, channels, samples, pChannelNames)) {
        pConnector->LockSignal(false);
        return SSC_CANNOT_FULFILL_REQUEST;
    }
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_ReleaseSignal(SSC_HANDLE handle)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->LockSignal(false);
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_LockStates(SSC_HANDLE handle, const double** pStates, int* count, int* samples, const char* const** pStateNames)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->LockStates(true);
    if (!pConnector->GetStates(pStates, count, samples, pStateNames)) {
        pConnector->LockStates(false);
        return SSC_CANNOT_FULFILL_REQUEST;
    }
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_ReleaseStates(SSC_HANDLE handle)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->LockStates(false);
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_LockParameters(SSC_HANDLE handle, const char* const** parameterLines, int* count)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->LockStates(true);
    if (!pConnector->GetParameters(parameterLines, count)) {
        pConnector->LockStates(false);
        return SSC_CANNOT_FULFILL_REQUEST;
    }
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_ReleaseParameters(SSC_HANDLE handle)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->LockParameters(false);
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_GetSignalDimensions(SSC_HANDLE handle, int* channels, int* samples)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    pConnector->LockSignal(true);
    bool ok = pConnector->GetSignal(nullptr, channels, samples, nullptr);
    pConnector->LockSignal(false);
    return ok ? SSC_SUCCESS : SSC_CANNOT_FULFILL_REQUEST;
}

SSC_RESULT STDCALL SSC_PushSignal(SSC_HANDLE handle, const double* pSignal)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    if (!pConnector->PushSignal(pSignal))
        return SSC_CANNOT_FULFILL_REQUEST;
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_PushState(SSC_HANDLE handle, const char* name, unsigned int value)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    if (!pConnector->PushState(name, value))
        return SSC_CANNOT_FULFILL_REQUEST;
    return SSC_SUCCESS;
}

SSC_RESULT STDCALL SSC_Commit(SSC_HANDLE handle)
{
    auto* pConnector = static_cast<SignalSharingConnector*>(handle);
    if (!pConnector)
        return SSC_INVALID_ARG;
    if (!pConnector->Commit())
        return SSC_CANNOT_FULFILL_REQUEST;
    return SSC_SUCCESS;
}
