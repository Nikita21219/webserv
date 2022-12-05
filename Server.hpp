//
// Created by Nikita Madorsky on 25.11.2022.
//

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

#include "color.hpp"

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
        std::string response;
        std::string mimeType;
    } fd_info;

public:
    Server(std::string ip, int port);
    ~Server();
    void mainLoop();

private:
    int getListenSocket(struct sockaddr_in addr);
    struct sockaddr_in getAddr(int port);
    int acceptNewConnection(int sock, fd_set *set, struct sockaddr_in *addr);
    int recieve(std::map<int, fd_info>::iterator *it, char **buf);
    int sendResponse(std::map<int, fd_info>::iterator it, std::string filename);
    int getMaxSock(int listenSock);
    void printErr(std::string s); // TODO delete this func
    void printWar(std::string s); // TODO delete this func
    std::string parseRequest(char *buf); // TODO delete this func
    static std::vector<std::string> split(std::string s, std::string sep);

    std::string ip;
    int port;
    fd_set read_set;
    fd_set write_set;
    std::map<int, fd_info> client_sockets;
};

#endif //WEBSERV_SERVER_HPP
