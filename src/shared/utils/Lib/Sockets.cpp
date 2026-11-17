////////////////////////////////////////////////////////////////////////////////
// $Id: Sockets.cpp 9309 2026-03-26 18:15:13Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for TCP and UDP sockets.
//    Addresses/ports are specified as in "192.2.14.18:21"
//    or as in "dog.animals.org:8080".
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
#include "Sockets.h"

#include "SysError.h"
#include "Waitable.h"

using sockio::socket;

namespace Tiny
{

const std::vector<std::string> &Socket::LocalIPs()
{
    socket::set_of_addresses s = socket::local_addresses();
    static std::vector<std::string> IPs;
    IPs.clear();
    for (socket::set_of_addresses::const_iterator i = s.begin(); i != s.end(); ++i)
        IPs.push_back(*i);
    return IPs;
}

Socket::Socket(sockio::socket *s)
: mpSocket(s),
  mpTcpSocket(dynamic_cast<sockio::tcpsocket *>(s)),
  mpUnixSocket(dynamic_cast<sockio::unixsocket*>(s))
{
}

Socket::~Socket()
{
    OnDestruct();
    Close();
    delete static_cast<sockio::socket *>(mpSocket);
}

Socket &Socket::Open(const std::string &address)
{
    mpSocket.load()->open(address);
    FdObject::Open(mpSocket);
    return *this;
}

Socket &Socket::Open(const std::string &ipOrFileName, int portOrFilePermissions)
{
    if (mpUnixSocket) {
        mpSocket.load()->open(ipOrFileName);
        mpUnixSocket.load()->set_file_permissions(portOrFilePermissions);
    }
    else {
        mpSocket.load()->open(ipOrFileName, portOrFilePermissions);
    }
    FdObject::Open(mpSocket);
    return *this;
}

bool Socket::WaitForAccept(int timeout)
{
    socket* pNew = nullptr;
    if (mpTcpSocket) {
        pNew = new tcpsocket;
    }
    else if (mpUnixSocket) {
        pNew = new unixsocket;
    }
    if (!pNew)
        return false;

    bool accepted = mpSocket.load()->wait_for_accept(*pNew, timeout);
    if (accepted)
    {
        sockio::socket *p = mpSocket;
        mpSocket = pNew;
        mpTcpSocket = dynamic_cast<sockio::tcpsocket*>(pNew);
        mpUnixSocket = dynamic_cast<sockio::unixsocket*>(pNew);
        FdObject::Open(mpSocket);
        delete p;
    }
    else
    {
        delete pNew;
    }
    return accepted;
}

bool Socket::WaitForAccept(Socket &s, int timeout)
{
    if (&s == this)
        return WaitForAccept(timeout);

    bool accepted = mpSocket.load()->wait_for_accept(*s.mpSocket, timeout);
    if (accepted)
        s.FdObject::Open(s.mpSocket);
    return accepted;
}

bool Socket::ConnectionOriented() const
{
    return mpTcpSocket || mpUnixSocket;
}

int Socket::Connected()
{
    if (!ConnectionOriented())
        return unconnected;
    return mpSocket.load()->connected();
}

Socket &Socket::SetTCPNodelay(bool b)
{
    if (mpTcpSocket)
    {
        mTcpNodelay = b;
        mpTcpSocket.load()->set_tcpnodelay(b);
    }
    return *this;
}

bool Socket::TCPNodelay() const
{
    return mTcpNodelay;
}

int64_t Socket::OnWrite(const char *p, int64_t n)
{
    if (mFlushAfterWrite && !mTcpNodelay)
        mpTcpSocket.load()->set_tcpnodelay(false);
    int64_t written = FdObject::OnWrite(p, n);
    if (mFlushAfterWrite && !mTcpNodelay)
        mpTcpSocket.load()->set_tcpnodelay(true);
    return written;
}

} // namespace Tiny
