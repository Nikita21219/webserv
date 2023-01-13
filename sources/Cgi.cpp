//
// Created by Nikita Madorsky on 16.12.2022.
//

#include "../headers/webserv.h"

Cgi::Cgi(std::string path, std::string bin_path):
path(path), bin_path(bin_path) {}

Cgi::~Cgi() {}

std::string Cgi::getRootDir(Parser *conf) {
    std::vector<std::string> v = split(path, "/");
    try {
        std::string cgiPath = v.at(0);
        std::string result;
        if ((result = conf->getLocfield("/" + cgiPath, "root")) != NOT_FOUND)
            return result + "/" + v.at(1);
    } catch (std::out_of_range &e) {
        return path;
    }
    return path;
}

int Cgi::execute(int out, char **args, char **env) {
    int pid = fork();
    int status;
    if (pid < 0) {
        printErr("Fork error"); //TODO delete line
        return 1;
    }
    else if (pid == 0) {
        dup2(out, STDOUT_FILENO);
        if (execve(bin_path.c_str(), args, env) < 0)
            exit(1);
        exit(0);
    }
    if (waitpid(pid, &status, 0) < 0)
        return 1;
    return status;
}

bool Cgi::wrongBinPath() {return bin_path == NOT_FOUND ? true : false;}

bool Cgi::noSuchFile() {
    bool result = false;
    std::ifstream ifs(path);
    if (!ifs.is_open())
        result = true;
    ifs.close();
    return result;
}

int Cgi::launch(char **env, int fd) {
    CgiEnv environ = CgiEnv(env);
    std::vector<std::string> pathSplit = split(path, "?");
    path = pathSplit[0];
    if (pathSplit.size() == 2)
        environ.addVariable("QUERY_STRING", pathSplit[1]);

    char **args = (char **)malloc(sizeof(char *) * 3);
    args[0] = (char *)path.c_str();
    args[1] = (char *)path.c_str();
    args[2] = NULL;

    int status = execute(fd, args, environ.toCArray());
    delete [] args; //TODO fix free
    return status;
}
