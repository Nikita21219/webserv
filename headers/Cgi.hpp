//
// Created by Nikita Madorsky on 16.12.2022.
//

#ifndef WEBSERV_CGI_HPP
#define WEBSERV_CGI_HPP

#include "webserv.h"

// static const std::string cgiDir = "cgi";

class Cgi {
public:
    Cgi(std::string path, std::map<int, fd_info>::iterator it, std::string bin_path);
    ~Cgi();
    int launch(char **env);

private:
    int execute(int out, char **args, char **env);

    std::string path;
    std::map<int, fd_info>::iterator it;
    std::string bin_path;
};

#endif //WEBSERV_CGI_HPP
