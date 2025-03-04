#ifndef E5681469_E53D_468F_AE3F_BA6C7D1BB6D4
#define E5681469_E53D_468F_AE3F_BA6C7D1BB6D4

//
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//
#include "./std/types.hpp"
#include "./std/hex.hpp"

//
class TCPClient {
public:

    //
    ~TCPClient() { close(sockfd); }

    //
    inline int connectToServer(const std::string& ip_address, int port) {
        //
        if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            std::cerr << "Error: Unable to create socket. " << std::to_string(sockfd) << std::endl;
            return 1;
        }

        //
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        //
        if (inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Error: Invalid IP address format: " << ip_address << std::endl;
            close(sockfd);
            return 1;
        }

        //
        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Error: Unable to connect to server at " << ip_address << ":" << port << std::endl;
            close(sockfd);
            return 1;
        }

        //
        buffer  = make_bytes(1024);
        //reserve = make_bytes(1024);
        std::cout << "Connected to server at " << ip_address << ":" << port << std::endl;
        return 0;
    }

    //
    inline bool sendRequest(bytespan_t const& stream) {
        std::cout << "Sending message: " << hex::b2h(stream) << std::endl;
        if (send(sockfd, stream->data(), stream->size(), 0) < 0) {
            std::cerr << "Error: Failed to send message." << std::endl;
            return false;
        }
        return true;
    }

    //
    inline bytespan_t receiveResponse() {
        //socklen_t len = sizeof(server_addr);
        size_t received = recv(sockfd, buffer->data(), buffer->size() - 1, 0/*, reinterpret_cast<struct sockaddr*>(&server_addr), &len*/);

        //
        if (received < 0) {
            std::cerr << "Error: Failed to receive message." << std::endl;
            return {};
        }
        (*buffer)[received] = '\0';

        //
        if (received > 0) {
            std::cout << "Received message: " << hex::b2h(buffer->data(), received) << std::endl;
        }
        return bytespan_t(buffer->data(), received);
    }

//
private:
    int sockfd = -1;

    //
    sockaddr_in server_addr = {};
    bytes_t buffer = {};
    //bytes_t reserve = {};
};

//
#endif
