//
// Created by Nikita Madorsky on 16.12.2022.
//

#include "../headers/webserv.h"

TempFile::TempFile(std::string fname): fname(fname) {
    if ((fd = open(fname.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU)) < 0)
        printErr(fname + " file didn't open");
}

TempFile::~TempFile() {
    close(fd);
    if (remove(fname.c_str()) < 0)
        printErr("File " + fname + "not deleted");
}

bool TempFile::isOpen() {return fd < 0 ? false : true;}

std::string TempFile::getName() {return fname;}

int TempFile::getFd() {return fd;}

std::string TempFile::read() {
    close(fd);
    std::ifstream ifs(fname.c_str());
    if (!ifs.is_open()) {
        printErr(fname + " not opened");
        return "";
    }
    std::string line;
    std::string s;
    while (std::getline(ifs, line))
        s += line + "\n";
    ifs.close();
    return s;
}
