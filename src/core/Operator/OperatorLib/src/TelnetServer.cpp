////////////////////////////////////////////////////////////////////////////////
// $Id: TelnetServer.cpp 9304 2026-03-20 13:48:52Z mellinger $
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A simple telnet server that runs inside a BCI2000
//   operator module.
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
#include "TelnetServer.h"

#include "ServerSession.h"
#include "BCIException.h"
#include "Streambuf.h"
#include "VersionInfo.h"

#include <string>

class TelnetServer::Session : public ServerSession
{
  public:
    Session(TelnetServer *parent);
    ~Session();

    TCPSocket& Socket();

    // ServerSession interface
    void Init() override;
    std::string RemoteHost() const override;
    bool WaitForInput() override;

  private:
    TCPSocket mSocket;
    BufferedIO mBuffer;
};

// TelnetServer
TelnetServer::TelnetServer(class StateMachine &inStateMachine, const std::string &inAddress)
    : mrStateMachine(inStateMachine)
{
    mListeningSocket.SetTCPNodelay(true);
    mListeningSocket.Open(inAddress);
    if (!mListeningSocket.IsOpen())
        throw bciexception << "TelnetServer: Cannot listen at " << inAddress;
    Thread::Start();
}

TelnetServer::~TelnetServer()
{
    mListeningSocket.Close();
    Thread::TerminateAndWait();
}

std::string TelnetServer::Address() const
{
    return mListeningSocket.LocalAddress();
}

int TelnetServer::OnExecute()
{
    while (mListeningSocket.Wait()) {
        auto pSession = new Session(this);
        pSession->Init();
        mListeningSocket.WaitForAccept(pSession->Socket(), 0);
        pSession->Start();
        // session will be auto-deleted when its thread ends
    }
    return 0;
}

// TelnetServer::Session
TelnetServer::Session::Session(TelnetServer *pParent)
    : ServerSession(pParent->mrStateMachine, &mBuffer)
{
}

TelnetServer::Session::~Session()
{
}

void TelnetServer::Session::Init()
{
    mSocket.SetBlockingMode(false);
    mBuffer.SetIO(&mSocket);
}

TCPSocket& TelnetServer::Session::Socket()
{
    return mSocket;
}

std::string TelnetServer::Session::RemoteHost() const
{
    return mSocket.RemoteAddress();
}

bool TelnetServer::Session::WaitForInput()
{
    return mSocket.Wait();
}
