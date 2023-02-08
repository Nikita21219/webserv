//
// Created by Nikita Madorsky on 22.12.2022.
//

#ifndef WEBSERV_CGIENV_HPP
#define WEBSERV_CGIENV_HPP

#include "webserv.h"

class CgiEnv {
public:
    CgiEnv(char **env);
    ~CgiEnv();
    void addVariable(std::string key, std::string val);
    char **toCArray();

private:
    CgiEnv();

    std::map<std::string, std::string> parseEnv(char **env);
    std::map<std::string, std::string> env;
};

#endif //WEBSERV_CGIENV_HPP
