#include "webserv.h"

int main(int argc, char **argv) {
	std::vector<Parser> conf;
	if (argc != 2) {
		std::cerr << "wrong number of arguments!\n";
		return 1;
	}
	if (get_conf(argv[1], conf))
		return 1;

    Server serv = Server(conf);
    serv.mainLoop();

    // std::cout << conf[0].getLocfield("/test", "methods") << "\n";
    // std::cout << conf[0].getServfield("methods") << "\n";

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
