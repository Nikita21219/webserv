#include "webserv.h"

int main(int argc, char **argv) {
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



    Server serv = Server(conf);
    serv.mainLoop();


/*
**	these tests should be run with configs/example file!
*/

//	std::cout << " -----some tests----- \n";
//	std::cout << "serv1 methods: " << conf[0].getServfield("methods") << '\n';
//	std::cout << "serv2 methods: " << conf[1].getServfield("methods") << '\n';
//	std::cout << "serv1 (location /hub) redirection: " << conf[0].getLocfield("/hub", "redirection") << '\n';
//	std::cout << "serv2 (location /directory/*.bla) cgi_pass: " << conf[1].getLocfield("/directory/*.bla", "cgi_pass") << '\n';
//	if (conf[0].getLocfield("/wrong_path", "methods") == NOT_FOUND)
//		std::cout << "serv1 (location /wrong_path) methods: wasn't wound!\n";
//	std::cout << " -----done!----- \n";

	return 0;
}
