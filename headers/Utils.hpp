//
// Created by Nikita Madorsky on 12.12.2022.
//

#ifndef WEBSERV_UTILS_HPP
#define WEBSERV_UTILS_HPP

#include "webserv.h"

typedef struct fd_info_t {
    std::string response;
    std::string headers;
    std::string mimeType;
    std::string redirectTo;
    bool readyToWriting;
    int status;
    int belongPort;
} fd_info;

static inline void printErr(std::string s) {std::cout << ERROR << s << "\n" << TERM_RESET;} // TODO tmp func

static inline void printWar(std::string s) {std::cout << WARNING << s << "\n" << TERM_RESET;} // TODO tmp func

static inline std::string itos(int num) {
    std::stringstream out;
    out << num;
    return out.str();
}

static inline std::string ltrim(std::string s, std::string subStr) {
    if (s.substr(0, subStr.length()) == subStr)
        s.erase(0, subStr.length());
    return s;
}

static inline std::string rtrim(std::string s, std::string subStr) {
    if (s.substr(s.length() - subStr.length(), subStr.length()) == subStr)
        s.erase(s.length() - subStr.length(), subStr.length());
    return s;
}

static inline std::string trim(std::string s, std::string subStr) {
    return rtrim(ltrim(s, subStr), subStr);
}

static inline std::vector<std::string> split(std::string s, std::string sep) {
    std::vector<std::string> arr;
    std::string token;
    size_t pos = 0;
    while ((pos = s.find(sep)) != std::string::npos) {
        arr.push_back(s.substr(0, pos));
        s.erase(0, pos + sep.length());
    }
    arr.push_back(s.substr(0, pos));
    return arr;
}

static inline void replaceOn(std::string &s, std::string subStr, std::string repl) {
    size_t pos;
    if ((pos = s.find(subStr)) != std::string::npos)
        s.replace(pos, subStr.length(), repl);
}

static inline bool startswith(std::string s, std::string start) {
    return (s.find(start) == 0) ? true : false;
}

static inline bool endswith(std::string s, std::string end) {
    return (s.find(end) == s.length() - end.length()) ? true : false;
}

#endif
