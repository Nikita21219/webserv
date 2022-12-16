//
// Created by Nikita Madorsky on 16.12.2022.
//

#include "../headers/webserv.h"

Cgi::Cgi(std::string path, std::map<int, fd_info>::iterator it, std::string bin_path):
path(ltrim(path, "/")), it(it), bin_path(bin_path) {}

Cgi::~Cgi() {}

int Cgi::execute(int out, char **args, char **env) {
    int pid = fork();
    int status;
    if (pid < 0) {
        printErr("Fork error"); // TODO delete line
        return 1;
    }
    else if (pid == 0) {
        // printWar("bin_path: " + bin_path);
        // printWar("args[0]: " + std::string(args[0]));
        // printWar("args[1]: " + std::string(args[1]));
        dup2(out, STDOUT_FILENO);
        if (execve(bin_path.c_str(), args, env) < 0)
            exit(1);
        exit(0);
    }
    if (waitpid(pid, &status, 0) < 0)
        return 1;
    return status;
}

int Cgi::launch(char **env) {
    TempFile tmpFile = TempFile("cgi_out" + itos(it->first));
    if (!tmpFile.isOpen()) {
        printErr("File not opened"); //TODO tmp line
        return 1;
    }
    std::string path = "cgi/main.py";
    char **args = (char **)malloc(sizeof(char *) * 3);
    args[0] = (char *)path.c_str();
    args[1] = (char *)path.c_str();
    args[2] = NULL; //TODO fix this stuff

    int status = execute(tmpFile.getFd(), args, env);
    delete [] args;
    if (status == 0)
        it->second.response = tmpFile.read();
    else
        it->second.status = 500;
    return status;
}
