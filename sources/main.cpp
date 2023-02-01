#include "webserv.h"

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

    try {
        Server serv = Server(conf, env);
        serv.mainLoop();
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
