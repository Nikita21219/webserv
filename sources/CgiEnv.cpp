//
// Created by Nikita Madorsky on 22.12.2022.
//

#include "../headers/CgiEnv.hpp"

CgiEnv::CgiEnv(char **env) {
    if (env == NULL || *env == NULL)
        return;
    this->env = parseEnv(env);
}

CgiEnv::~CgiEnv() {}

std::map<std::string, std::string> CgiEnv::parseEnv(char **env) {
    std::map<std::string, std::string> result;
    while (env && *env) {
        std::string line = *env++;
        std::vector<std::string> splitLine = split(line, "=");
        if (splitLine.size() != 2)
            continue;
        result[splitLine[0]] = splitLine[1];
    }
    return result;
}

void CgiEnv::addVariable(std::string key, std::string val) {env[key] = val;}

char **CgiEnv::toCArray() {
    size_t sz = env.size() + 1;
    char **ptr = (char **) calloc(sz, (sizeof(char *) + 1));
    if (ptr == NULL) {
        printErr("Malloc error");
        return NULL;
    }
    int idx = 0;
    for (std::map<std::string, std::string>::iterator i = env.begin(); i != env.end(); i++) {
        std::string str = i->first + "=" + i->second;
        char *cstr = (char *) calloc(str.length() + 1, sizeof(char));
        std::strcpy(cstr, str.c_str());
        ptr[idx++] = cstr;
    }
    ptr[idx] = NULL;
    return ptr;
}
