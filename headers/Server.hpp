//
// Created by Nikita Madorsky on 25.11.2022.
//

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

#include "webserv.h"

#define BUF_SZ 2048

class Server {
public:
    Server(std::vector<Parser> conf);
    ~Server();
    void mainLoop();

private:
    // Sockets
    int getListenSocket(struct sockaddr_in addr);
    struct sockaddr_in getAddr(int port);
    int acceptNewConnection(fd_set *set, struct sockaddr_in *addr);
    int getMaxSock();

    // Handle request and send response
    int recieve(std::map<int, fd_info>::iterator *it, char **buf);
    int sendResponse(std::map<int, fd_info>::iterator *it);
    void codeResponseInit();
    Parser *getConfByPort(int port);
    void formResponse(std::map<int, fd_info>::iterator it);
    // Handle errors
    int removeClient(std::map<int, fd_info>::iterator *it);

    // Fields
    std::vector<Parser> conf;
    fd_set read_set;
    fd_set write_set;
    std::map<int, fd_info> client_sockets;
    std::map<int, std::string> code_response;
    std::map<int, int> listen_socks;
};

#endif //WEBSERV_SERVER_HPP
