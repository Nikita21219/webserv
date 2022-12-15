//
// Created by Nikita Madorsky on 13.12.2022.
//

#ifndef WEBSERV_REQUEST_HPP
#define WEBSERV_REQUEST_HPP

#include "webserv.h"

class Request {
public:
    Request(std::map<int, fd_info>::iterator it, char *buf, Parser *conf, fd_set *write_set);
    ~Request();
    int parse();
    int mainLogic();

private:
    std::string getLocURL();
    int redirect();
    void preparePathToOpen(std::string locURL, std::string rootDir);
    void setMimeType();
    int getRequest();
    int renderErrorPage(int status);
    bool isAllowMethod(std::string allowed_methods);
    bool isBadRequest();
    int badRequest();

    std::map<int, fd_info>::iterator it;
    char *buf;
    Parser *conf;
    std::string path;
    std::string method;
    fd_set *write_set;
};

#endif //WEBSERV_REQUEST_HPP
