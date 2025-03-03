#ifndef A315E282_4CE3_475F_B4D7_1AB155D23089
#define A315E282_4CE3_475F_B4D7_1AB155D23089

//
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

//
#include "./std/types.hpp"
#include "./std/hex.hpp"

//
class UDP {
public:

    //
    ~UDP() { close(sockfd); }

    //
    inline int init() {
        //
        if ((sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        { std::cerr << "Error: Unable to create socket. " << std::to_string(sockfd) << std::endl; return 1; }

        //
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_addr   = IN6ADDR_ANY_INIT;
        server_addr.sin6_port   = htons(port);

        //
        bound = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (bound < 0) { std::cerr << "Error: Unable to bind socket to port " << port << ". " << bound << std::endl; close(sockfd); return 1; }

        //
        buffer  = make_bytes(1024);
        reserve = make_bytes(1024);
        return 0;
    }

    //
    inline bytespan_t handleRequest() {
        socklen_t client_len = sizeof(client_addr);
        size_t received = recvfrom(sockfd, buffer->data(), buffer->size() - 1, 0, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);

        //
        if (received < 0) { std::cerr << "Error: Failed to receive message." << std::endl; return {}; }; (*buffer)[received] = '\0';

        //
        std::cout << "Received message: " << hex::b2h(buffer->data(), received) << std::endl;
        return bytespan_t(buffer->data(), received);
    }

    //
    inline bool sendResponse(bytespan_t const& stream) {
        std::cout << "Sending message: " << hex::b2h(stream) << std::endl;
        sendto(sockfd, stream->data(), stream->size(), 0, (struct sockaddr const *)&client_addr, sizeof(client_addr));
        return true;
    }

//
private:
    int bound = -1, sockfd = -1;
    const int port = 5540;

    //
    sockaddr_in6 server_addr = {};
    sockaddr_in6 client_addr = {};
    bytes_t buffer  = {}; //buffer[0] = '\0';
    bytes_t reserve = {};
};

//
#endif
