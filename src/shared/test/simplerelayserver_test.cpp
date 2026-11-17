#include <iostream>
#include <string>
#include "SockStream.h"
#include "BinaryData.h"

#if _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

int main(int argc, char* argv[])
{
    if (argc != 3 || std::string(argv[1]) == "--help") {
        std::cerr << "usage: simplerelayserver_test <server address> [r|w]" << std::endl;
        return -1;
    }
    client_tcpsocket socket(argv[1]);
    if (!socket.is_open()) {
        std::cerr << "Could not open " << argv[1] << std::endl;
        return -1;
    }
    socket.set_tcpnodelay(true);
    if (argv[2][0] == 'r') {
        std::vector<char> buffer;
        while (true) {
            uint32_t size = 0;
            int count = socket.read(reinterpret_cast<char*>(&size), sizeof(size));
            if (count != sizeof(size))
                return -1;
            size = ntohl(size);
            if (buffer.size() < size)
                buffer.resize(size);
            if (size > 0) {
                count = socket.read(buffer.data(), size);
                if (count != size)
                    return -1;
            }
            if (size > 0)
                std::cout.write(buffer.data(), size);
            std::cout << std::endl;
        }
    }
    else if (argv[2][0] == 'w') {
        while (true) {
            std::string line;
            std::getline(std::cin, line);
            uint32_t size = line.length();
            size = htonl(size);
            int count = socket.write(reinterpret_cast<char*>(&size), sizeof(size));
            if (count != sizeof(size))
                return -1;
            if (size > 0) {
                count = socket.write(line.data(), line.length());
                if (count != line.length())
                    return -1;
            }
        }
    }
    else {
        std::cerr << "Expected r or w for second argument, got " << argv[2][0] << std::endl;
        return -1;
    }
    return 0;
}
