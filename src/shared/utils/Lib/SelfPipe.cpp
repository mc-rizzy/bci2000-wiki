//////////////////////////////////////////////////////////////////////
// $Id: SelfPipe.cpp 9316 2026-03-29 19:51:40Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A pipe-like object that allows synchronized transfer
//   of data between threads.
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
#ifndef USE_UNIX_PIPES_ON_WINDOWS
# define USE_UNIX_PIPES_ON_WINDOWS 1
#endif
#include "SelfPipe.h"
#include <stdexcept>
#include <string>
#include <cassert>

#if _WIN32
#include <Winsock2.h>
# if USE_UNIX_PIPES_ON_WINDOWS
#  include <afunix.h>
#  include <direct.h>
# endif
#else // _WIN32
#include <sys/socket.h>
# include <sys/fcntl.h>
#include <unistd.h>
#define closesocket(x) close(x)
#endif // _WIN32

namespace fdio
{

selfpipe::selfpipe(int inBufsize)
{
    bool ok = true;
    // socket type must be SOCK_STREAM to avoid losing "set" events
#if !_WIN32
    errno = 0;
    ok = !::socketpair(AF_UNIX, SOCK_STREAM, 0, mFds);
    if (ok) { // SOCK_CLOEXEC not available on macOS
        for (int fd : mFds)
            ::fcntl(fd, F_SETFD, FD_CLOEXEC);
    }
#elif USE_UNIX_PIPES_ON_WINDOWS
    SOCKET listener = ::socket(PF_UNIX, SOCK_STREAM, 0);
    ok = (listener != INVALID_SOCKET);
    // disable automatic socket inheritance to child processes
    ok = ok && ::SetHandleInformation(reinterpret_cast<HANDLE>(listener), HANDLE_FLAG_INHERIT, 0);
    SOCKET connector = socket(PF_UNIX, SOCK_STREAM, 0);
    ok = (connector != INVALID_SOCKET);
    // disable automatic socket inheritance to child processes
    ok = ok && ::SetHandleInformation(reinterpret_cast<HANDLE>(connector), HANDLE_FLAG_INHERIT, 0);

    sockaddr_un addr;
    int addr_size = sizeof(addr);
    ::memset(&addr, 0, addr_size);
    addr.sun_family = AF_UNIX;

    char tmppath[MAX_PATH] = "";
    ::GetTempPathA(sizeof(tmppath) - 1, tmppath);
    std::string tmpprefix = std::string(tmppath) + "sock." + std::to_string(::GetProcessId(::GetCurrentProcess())) + ".";

    static std::atomic<int> instance = 0;
    std::string path = tmpprefix + std::to_string(instance++);
    assert(path.length() < sizeof(addr.sun_path));
    std::strcpy(addr.sun_path, path.c_str());
    ::unlink(path.c_str());
    ok = ok && !::bind(listener, (const sockaddr*)&addr, addr_size);
    ok = ok && !::listen(listener, 1);
    u_long val = 1;
    // use nonblocking sockets to avoid secondary thread
    ok = ok && !::ioctlsocket(listener, FIONBIO, &val);
    ok = ok && !::ioctlsocket(connector, FIONBIO, &val);
    ok = ok && ::connect(connector, (const sockaddr*)&addr, addr_size) && ::WSAGetLastError() == WSAEWOULDBLOCK;

    // wait for connect() to succeed
    if (ok) {
        struct timeval timeout = {1, 0};
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        int result = ::select(0, &readfds, nullptr, nullptr, &timeout);
        ok = (result == 1);
    }
    SOCKET accepted = INVALID_SOCKET;
    if (ok) {
        accepted = ::accept(listener, nullptr, nullptr);
        ok = (accepted != INVALID_SOCKET);
        // disable automatic socket inheritance to child processes
        ok = ok && ::SetHandleInformation(reinterpret_cast<HANDLE>(accepted), HANDLE_FLAG_INHERIT, 0);
    }
    ::closesocket(listener);
    ::unlink(path.c_str());

    // return to blocking mode
    val = 0;
    ok = ok && !::ioctlsocket(connector, FIONBIO, &val);

    mFds[0] = connector;
    mFds[1] = accepted;

#else // USE_UNIX_PIPES_ON_WINDOWS
    mFds[0] = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ok = (mFds[0] != INVALID_SOCKET);
    int value = 1;
    ok = ok && !::setsockopt(mFds[0], SOL_SOCKET, SO_DONTLINGER, (const char*)&value, sizeof(value));
    value = 1;
    ok = ok && !::setsockopt(mFds[0], SOL_SOCKET, SO_REUSEADDR, (const char*)&value, sizeof(value));
    // disable automatic socket inheritance to child processes
    ok = ok && ::SetHandleInformation(reinterpret_cast<HANDLE>(mFds[0]), HANDLE_FLAG_INHERIT, 0);

    mFds[1] = mFds[0]; // connect socket to itself in order to save ports

    sockaddr_in addr;
    int addr_size = sizeof(addr);
    ::memset(&addr, 0, addr_size);
    addr.sin_family = AF_INET;
    uint32_t loopback = INADDR_LOOPBACK | 'tny';

    bool bound = false;
    int tries = 10; // a total of 65535*10 possible ports
    while (ok && !bound && --tries > 0) {
        bound = false;
        addr.sin_addr.s_addr = htonl(++loopback);
        addr.sin_port = 0; // ask for an unused port
        bound = !::bind(mFds[0], (const sockaddr *)&addr, addr_size);
        ok = bound || ::WSAGetLastError() == WSAEADDRINUSE;
    }
    ok = ok && !::getsockname(mFds[0], (sockaddr*)&addr, &addr_size);
    addr.sin_addr.s_addr = htonl(loopback); // getsockname() not guaranteed to return IP address part
    ok = ok && !::connect(mFds[1], (const sockaddr*)&addr, addr_size);
#endif

    if (inBufsize >= 0) {
        int bufsize = inBufsize, bufs[] = {SO_SNDBUF, SO_RCVBUF};
        for (size_t i = 0; i < sizeof(mFds) / sizeof(*mFds); ++i) {
            for (size_t j = 0; j < sizeof(bufs) / sizeof(*bufs); ++j) {
                ok = ok && !::setsockopt(mFds[i], SOL_SOCKET, bufs[j], (const char*)&bufsize, sizeof(bufsize));
            }
        }
    }
    if (!ok) {
#if _WIN32
        DWORD lastError = ::WSAGetLastError();
#else
        int lastError = errno;
#endif
        throw std::runtime_error(std::string("Could not set up self pipe (") + std::to_string(lastError) + ")");
    }

    m_fd = mFds[1];
}

selfpipe::~selfpipe()
{
    close();
}

int selfpipe::on_close()
{
    if (mFds[1] != mFds[0])
        ::closesocket(mFds[1]);
    ::closesocket(mFds[0]);
    return 0;
}

int selfpipe::on_read(char *p, size_t n)
{
    return ::recv(m_fd, p, n, 0);
}

int selfpipe::on_write(const char *p, size_t n)
{
    return ::send(mFds[0], p, n, MSG_DONTROUTE);
}

} // namespace fdio
