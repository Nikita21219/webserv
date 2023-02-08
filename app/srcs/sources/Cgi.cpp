//
// Created by Nikita Madorsky on 16.12.2022.
//

#include "../headers/webserv.h"

Cgi::Cgi(std::string path, std::string bin_path):
path(path), bin_path(bin_path) {}

Cgi::~Cgi() {}

int Cgi::execute(int out, char **args, char **env) {
    int pid = fork();
    int status;
    if (pid < 0)
        return 1;
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

int Cgi::launch(char **env, int fd) {
    CgiEnv environ = CgiEnv(env);
    std::vector<std::string> pathSplit = split(path, "?");
    path = pathSplit[0];
    if (pathSplit.size() == 2)
        environ.addVariable("QUERY_STRING", pathSplit[1]);

    char **args = (char **) malloc(sizeof(char *) * 3);
    args[0] = (char *)path.c_str();
    args[1] = (char *)path.c_str();
    args[2] = NULL;

    char **environArray = environ.toCArray();
    int status = execute(fd, args, environArray);
    free(args);
    char **ptr = environArray;
    while (*environArray)
        free(*environArray++);
    free(ptr);
    return status;
}
