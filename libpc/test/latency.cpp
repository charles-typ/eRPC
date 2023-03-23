#include <iostream>
#include <string>

#include <arpa/inet.h>  // htons, inet_addr
#include <netinet/in.h> // sockaddr_in
#include <sys/types.h>  // uint16_t
#include <sys/socket.h> // socket, sendto
#include <unistd.h>     // close
#include "util.h"
#include "pc/parser/command_parser.h"

/*
 * Usage: ./latency [hostname] [port] [Command configuration] [REPEAT]
 * vmhost4 10.10.11.204
 * FPGA NIC 10.10.10.221
 */

int main(int argc, char const *argv[])
{
    std::string hostname = std::string(argv[1]);
    uint16_t port = std::stoul(std::string(argv[2]));
    std::string config_f = std::string(argv[3]);
    CMDLoader cmd(config_f);
    int repeat = std::stol(argv[4]);

    char buf[1024];

    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    std::cout << "Connection status: " << sock << std::endl;

    sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_port = htons(port);
    destination.sin_addr.s_addr = inet_addr(hostname.c_str());

    // std::string msg = cmd.GetPayload();
    std::string msg(24, 'a');
    std::cout << "Payload is: " << msg << std::endl;
    long total_latency = 0;
    for (int i = 0; i < repeat; i++)
    {
        auto start = get_time();
        int n_bytes = ::sendto(sock, msg.c_str(), msg.length(), 0, reinterpret_cast<sockaddr *>(&destination), sizeof(destination));
        recv(sock, buf, 1024, 0);
        auto end = get_time();
        std::cout << "Result for " << i << " iteration is: " << (end - start) / 1000.0 << " us " << std::endl;
        total_latency += end - start;
    }
    std::cout << "Average latency for " << repeat << " runs is: " << total_latency / repeat / 1000.0 << " us " << std::endl;
    // std::cout << n_bytes << " bytes sent" << std::endl;
    std::cout << "The response is: " << buf << std::endl;
    ::close(sock);

    return 0;
}
