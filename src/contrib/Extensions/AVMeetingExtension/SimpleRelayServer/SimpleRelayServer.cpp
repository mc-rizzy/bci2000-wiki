////////////////////////////////////////////////////////////////////
// Author: mellinger@neurotechcenter.org
// Description: A simple relay server that allows a single pair
//   of clients to connect and then forwards each client's output
//   to the other client's input.
//   Regarding the protocol between clients, it makes the single
//   assumption that each data packet starts with a 4-byte length
//   field in network byte order.
//   This assumption is necessary to avoid incomplete packets to
//   be forwarded from the first client to the second one when
//   the second one is connecting.
/////////////////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>

static const int LISTEN_PORT = 1915;
static std::mutex sOutputMutex; // for std::cout and std::cerr

static int DoRead(int fd, void* pOut, int count)
{
    char* p = static_cast<char*>(pOut);
    int totalRead = 0, n = 0;
    do {
      n = ::recv(fd, p, count - totalRead, 0);
      totalRead += n;
      p += n;
    } while(n > 0 && count > totalRead);
    return n > 0 ? totalRead : -1;
}

static int DoWrite(int fd, const void* pIn, int count)
{
    const char* p = static_cast<const char*>(pIn);
    int totalSent = 0, n = 0;
    do {
      n = ::send(fd, p, count - totalSent, MSG_NOSIGNAL);
      totalSent += n;
      p += n;
    } while(n > 0 && count > totalSent);
    return n > 0 ? totalSent : -1;
}

static void ErrorExit(int retval, const std::string& context = "")
{
    if (retval < 0) {
        std::unique_lock<std::mutex> lock(sOutputMutex);
        if (!context.empty())
            std::cerr << context << ": ";
         std::cerr << ::strerror(errno) << std::endl;
         ::exit(-1);
    }
}

class Session
{
public:
    int AddSocket(int);
    void Cleanup();
    void CleanupSocket(int which);

private:
    void CopyLoop(int from, int to);

    std::mutex mMutex;
    int mSockets[2] = {-1, -1};
};

int Session::AddSocket(int sock)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mSockets[0] < 0) {
        mSockets[0] = sock;
        std::thread copyThread([this]{CopyLoop(0, 1);});
        copyThread.detach();
        return 1;
    }
    if (mSockets[1] < 0) {
        mSockets[1] = sock;
        std::thread copyThread([this]{CopyLoop(1, 0);});
        copyThread.detach();
        return 2;
    }
    return 0;
}

void Session::Cleanup()
{
    std::unique_lock<std::mutex> lock(mMutex);
    for (auto& sock : mSockets) {
        ::close(sock);
        sock = -1;
    }
}

void Session::CleanupSocket(int which)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mSockets[which] >= 0) {
        ::close(mSockets[which]);
        mSockets[which] = -1;
        std::unique_lock<std::mutex> outputLock(sOutputMutex);
        std::cerr << "Closed connection " << which + 1 << std::endl;
    }
}

void Session::CopyLoop(int from, int to)
{
    std::unique_lock<std::mutex> outputLock(sOutputMutex);
    std::cerr << "Starting copy loop from " << from + 1 << " to " << to + 1 << std::endl;
    outputLock.unlock();
    std::vector<char> buffer;
    while (true) {
        uint32_t size_n, size_h = 0;
        int err = DoRead(mSockets[from], &size_n, sizeof(size_n));
        if (err <= 0) {
            break;
        }
        size_h = ntohl(size_n);
        if (size_h > 0) {
            if (buffer.size() < size_h)
                buffer.resize(size_h);
            err = DoRead(mSockets[from], buffer.data(), size_h);
            if (err <= 0) {
                break;
            }
            if (err != size_h) {
                std::cerr << "Incomplete packet read" << std::endl;
            }
            else if (1){//std::string(buffer.data()) != "AVPacket") {
                std::cerr << "Received " << buffer.data() << " packet of size " 
                          << size_h << " from " << from + 1 << std::endl;
            }
        }

        std::unique_lock<std::mutex> lock(mMutex);
        if (mSockets[to] < 0)
            continue;
        lock.unlock();

        err = DoWrite(mSockets[to], &size_n, sizeof(size_n));
        if (size_h > 0) {
            err = DoWrite(mSockets[to], buffer.data(), size_h);
            if (err != size_h) {
                std::cerr << "Incomplete packet write" << std::endl;
            }
        }
    }
    CleanupSocket(from);
    outputLock.lock();
    std::cerr << "Finished copy loop from " << from + 1 << " to " << to + 1 << std::endl;
}

class SimpleRelayServer
{
public:
    explicit SimpleRelayServer(int port);
    ~SimpleRelayServer();
    void Run();

private:
    int mServerSocket = -1;
    Session mSession;
};

SimpleRelayServer::SimpleRelayServer(int port)
{
    mServerSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    ErrorExit(mServerSocket);

    int val = 1;
    ::setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int err = ::bind(mServerSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    ErrorExit(err, "bind");

    err = ::listen(mServerSocket, 10);
    ErrorExit(err, "listen");

    std::cerr << "Listening on port " << port << std::endl;
}

SimpleRelayServer::~SimpleRelayServer()
{
    ::close(mServerSocket);
}

void SimpleRelayServer::Run()
{
    while (true) {
        int clientSocket = ::accept(mServerSocket, nullptr, 0);
        int val = 1;
        ErrorExit(clientSocket, "accept");
        ::setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
        int n = mSession.AddSocket(clientSocket);
        std::unique_lock<std::mutex> lock(sOutputMutex);
        if (n) {
            std::cerr << "Accepted connection " << n << std::endl;
        }
        else {
            ::close(clientSocket);
            std::cerr << "Refused additional connection" << std::endl;
        }
    }
    mSession.Cleanup();
}

int main(int argc, char** argv)
{
    int port = LISTEN_PORT;
    if (argc > 1) {
        port = ::atoi(argv[1]);
    }
    SimpleRelayServer server(port);
    server.Run();
    return 0;
}
