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
        std::string mimeType;
        std::string browserInfo;
        int status;
        int belongPort;
    } fd_info;

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
    int sendResponse(std::map<int, fd_info>::iterator it);
    void codeResponseInit();
    Parser *getConfByPort(int port);
    bool isAllowMethod(std::string method, std::string allowed_methods);
    void setMimeType(std::map<int, fd_info>::iterator it, std::string path);
    int getRequest(std::string path, std::string rootDir, std::map<int, fd_info>::iterator it);
    void initBrowserInfo(std::map<int, fd_info>::iterator it, std::string location);
    int redirect(std::map<int, fd_info>::iterator it, std::string path);

    // Utils
    void printErr(std::string s); // TODO delete this func
    void printWar(std::string s); // TODO delete this func
    static std::vector<std::string> split(std::string s, std::string sep);
    void deleteDirFromPath(std::string *path);

    // Error pages
    int renderErrorPage(std::map<int, fd_info>::iterator it, int status);


    // Fields
    std::vector<Parser> conf;
    fd_set read_set;
    fd_set write_set;
    std::map<int, fd_info> client_sockets;
    std::map<int, std::string> code_response;
    std::map<int, int> listen_socks;
};

#endif //WEBSERV_SERVER_HPP
