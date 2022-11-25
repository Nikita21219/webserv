//
// Created by Nikita Madorsky on 25.11.2022.
//

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <vector>

class Server {
public:
    Server(std::string ip, int port);
    ~Server();
    void mainLoop();

private:
    int getListenSocket(struct sockaddr_in addr);
    struct sockaddr_in getAddr(int port);
    int getMaxSock(std::vector<int> socks);
    int acceptNewConnection(int sock, fd_set *set, struct sockaddr_in *addr);
    int recieve(int fd, char **buf);

    std::string ip;
    int port;
    fd_set read_set;
    fd_set write_set;
    std::vector<int> client_sockets;
};

#endif //WEBSERV_SERVER_HPP
