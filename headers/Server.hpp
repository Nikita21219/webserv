//
// Created by Nikita Madorsky on 25.11.2022.
//

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

#include "webserv.h"

#define BUF_SZ 2048

class Server {
private:
    typedef struct fd_info_t {
        bool readyToWriting;
        std::string response;
        int status;
        std::string mimeType;
        int belongPort;
    } fd_info;

public:
    Server(std::vector<Parser> conf);
    ~Server();
    void mainLoop();

private:
    int getListenSocket(struct sockaddr_in addr);
    struct sockaddr_in getAddr(int port);
    int acceptNewConnection(fd_set *set, struct sockaddr_in *addr);
    int recieve(std::map<int, fd_info>::iterator *it, char **buf);
    int sendResponse(std::map<int, fd_info>::iterator it);
    int getMaxSock();
    void printErr(std::string s); // TODO delete this func
    void printWar(std::string s); // TODO delete this func
    static std::vector<std::string> split(std::string s, std::string sep);
    void codeResponseInit();
    int pageNotFound(std::map<int, fd_info>::iterator it);
    Parser *getConfByPort(int port);

    std::vector<Parser> conf;
    fd_set read_set;
    fd_set write_set;
    std::map<int, fd_info> client_sockets;
    std::map<int, std::string> code_response;
    std::map<int, int> listen_socks;
};

#endif //WEBSERV_SERVER_HPP
