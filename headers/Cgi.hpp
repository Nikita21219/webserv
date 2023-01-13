//
// Created by Nikita Madorsky on 16.12.2022.
//

#ifndef WEBSERV_CGI_HPP
#define WEBSERV_CGI_HPP

#include "webserv.h"

class Cgi {
public:
    Cgi(std::string path, std::string bin_path);
    ~Cgi();
    int launch(char **env, int fd);

private:
    int execute(int out, char **args, char **env);

    std::string path;
    std::map<int, fd_info>::iterator it;
    std::string bin_path;

    // class smartTwoDimensionalArray {
    // public:
    //     smartTwoDimensionalArray(char **ptr): ptr(ptr) {}
    //     ~smartTwoDimensionalArray() {
    //         char **tmpPtr = ptr;
    //         while (*ptr)
    //             free(*ptr++);
    //         free(tmpPtr);
    //     }
    //     char **getPtr() {return ptr;}
    //
    // private:
    //     char **ptr;
    // };
};

#endif //WEBSERV_CGI_HPP
