#include <iostream>
#include <string>
#include <thread>
#include <random>
#include <mutex>
#include <cstring>
#include "SockStream.h"

static const int cUdpMaxlen = 1470;
static std::mutex sMutex;

static void threadFunc(int messageCount, int seed)
{
    std::mt19937 r(seed);
    std::uniform_int_distribution<int> d(1, cUdpMaxlen);
    client_udpsocket socket("127.0.0.1:4000");
    char* buf = new char[cUdpMaxlen];
    for (int k = 0; k < messageCount; ++k) {
        int length = d(r);
        std::uniform_int_distribution<int> charDist('a', 'z');
        std::string s;
        s.resize(length);
        for (int j = 0; j < length; ++j)
            s[j] = charDist(r);
        std::unique_lock<std::mutex> lock(sMutex);
        int bytesWritten = socket.write(s.data(), s.size());
        if (bytesWritten < 0)
            std::cerr << socket.error_message() << std::endl;
        else if (bytesWritten != s.size())
            std::cerr << "Could not send all data" << std::endl;
        int bytesRead = socket.read(buf, cUdpMaxlen);
        if (bytesRead < 0)
            std::cerr << socket.error_message() << std::endl;
        else if (bytesRead != bytesWritten)
            std::cerr << "Echo disagrees in size: " << bytesWritten << " vs " << bytesRead << std::endl;
        else if (::memcmp(buf, s.data(), bytesRead))
            std::cerr << "Echo does not agree with original data" << std::endl;
    }
    delete[] buf;
}

int main(int argc, char* argv[])
{
    if (argc != 3 || std::string(argv[1]) == "--help") {
        std::cerr << "usage: udpsocket_test <thread count> <message count>" << std::endl;
        return -1;
    }
    int threadCount = ::atoi(argv[1]);
    int messageCount = ::atoi(argv[2]);
    std::mt19937 r;
    std::vector<std::thread*> threads;
    std::vector<fdio::fd_object*> sockets;
    server_udpsocket serverSocket("127.0.0.1:4000");
    for (int i = 0; i < threadCount; ++i) {
        threads.push_back(new std::thread(&threadFunc, messageCount, r()));
        std::cout << "." << std::flush;
        if (threads.size() % 80 == 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;

    char* pData = new char[cUdpMaxlen];
    int count = 0;
    while (count < messageCount * threadCount) {
        int bytesRead = serverSocket.read(pData, cUdpMaxlen);
        if (bytesRead < 0)
            std::cerr << serverSocket.error_message() << std::endl;
        int bytesWritten = serverSocket.write(pData, bytesRead);
        if (bytesWritten < 0)
            std::cerr << serverSocket.error_message() << std::endl;
        else if (bytesWritten != bytesRead)
            std::cerr << "Echo server could not write all bytes read" << std::endl;
        ++count;
    }
    delete[] pData;

    for (auto& pThread : threads)
        pThread->join();

    return 0;
}
