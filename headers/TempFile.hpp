//
// Created by Nikita Madorsky on 16.12.2022.
//

#ifndef WEBSERV_TEMPFILE_HPP
#define WEBSERV_TEMPFILE_HPP

#include "webserv.h"

class TempFile {
public:
    TempFile(std::string fname);
    ~TempFile();
    bool isOpen();
    std::string getName();
    int getFd();
    std::string read();

private:
    std::string fname;
    int fd;
};

#endif //WEBSERV_TEMPFILE_HPP
