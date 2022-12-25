//
// Created by Nikita Madorsky on 16.12.2022.
//

#ifndef WEBSERV_CGI_HPP
#define WEBSERV_CGI_HPP

#include "webserv.h"

class Cgi {
public:
    Cgi(std::string path, std::map<int, fd_info>::iterator it, Parser *conf);
    ~Cgi();
    bool wrongBinPath();
    int launch(char **env);

private:
    int execute(int out, char **args, char **env);
    bool noSuchFile();
    std::string getRootDir(Parser *conf);

    std::string path;
    std::map<int, fd_info>::iterator it;
    std::string bin_path;
};

#endif //WEBSERV_CGI_HPP
