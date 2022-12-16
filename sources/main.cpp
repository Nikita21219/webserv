#include "webserv.h"

int cgiExec(int out, std::string bin_path, char **args, char **env) {
    int pid = fork();
    if (pid < 0) {
        printErr("Fork error");
        return 1;
    }
    else if (pid == 0) {
        dup2(out, STDOUT_FILENO);
        if (execve(bin_path.c_str(), args, env) < 0)
            exit(1);
        exit(0);
    }
    if (waitpid(pid, NULL, 0) < 0)
        return 1;
    return 0;
}

int main(int argc, char **argv, char **env) {
	std::vector<Parser> conf;
	const char *file_conf;
	if (argc > 2) {
		std::cerr << "wrong number of arguments!\n";
		return 1;
	} else if (argc == 2)
		file_conf = argv[1];
	else
		file_conf = "/etc/webserv.conf";
	if (get_conf(file_conf, conf))
		return 1;

    TempFile tmpFile = TempFile("cgi_out");
    if (!tmpFile.isOpen()) {
        printErr("File not opened");
        return 1;
    }
    std::string path = "cgi/main.py";
    char **args = (char **)malloc(sizeof(char *) * 3);
    args[0] = (char *)path.c_str();
    args[1] = (char *)path.c_str();
    args[2] = NULL;
    cgiExec(tmpFile.getFd(), "/usr/local/bin/python3", args, env);
    delete [] args;
    std::cout << tmpFile.read() << "\n";

    // Server serv = Server(conf);
    // serv.mainLoop();

    return 0;
}

/*
	std::cout << " -----some tests----- \n";
	std::cout << "serv1 methods: " << conf[0].getServfield("methods") << '\n';
	std::cout << "serv2 methods: " << conf[1].getServfield("methods") << '\n';
	std::cout << "serv1 (location /hub) redirection: " << conf[0].getLocfield("/hub", "redirection") << '\n';
	if (conf[0].getLocfield("/wrong_path", "methods") == NOT_FOUND)
		std::cout << "serv1 (location /wrong_path) methods: wasn't wound!\n";
	std::cout << " -----done!----- \n";
*/
