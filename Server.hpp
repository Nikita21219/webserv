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
#include <fstream>
#include <vector>
#include <map>

class Server {
private:
    typedef struct fd_info_t {
        bool readyToWriting;
    } fd_info;

public:
    Server(std::string ip, int port);
    ~Server();
    void mainLoop();

private:
    int getListenSocket(struct sockaddr_in addr);
    struct sockaddr_in getAddr(int port);
    int acceptNewConnection(int sock, fd_set *set, struct sockaddr_in *addr);
    int recieve(std::map<int, fd_info>::iterator it, char **buf);
    int sendResponse(std::map<int, fd_info>::iterator it, std::string filename);

    std::string ip;
    int port;
    fd_set read_set;
    fd_set write_set;
    std::map<int, fd_info> client_sockets;
};

#endif //WEBSERV_SERVER_HPP
