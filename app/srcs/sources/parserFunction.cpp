
#include "Parser.hpp"

bool get_conf(const char *file, std::vector<Parser> &conf) {
	std::ifstream in_file;
	std::string serv;
	in_file.open(file);
	if (!in_file.is_open()) {
		std::cerr << "Config wasn't opened!\n";
		return true;
	}
	while(1) {
		try {
			if (get_one_serv(in_file, serv))
				break;
			conf.push_back(Parser(serv));
		} catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			return true;
		}
		serv.clear();
	}
	return false;
}

bool get_one_serv(std::ifstream &in_file, std::string &serv) {
	std::string tmp;
	std::getline(in_file, tmp);
	while (tmp.size() == 0) {
		std::getline(in_file, tmp);
		if (in_file.eof())
			return true;
	}
	if (tmp.find("server") == std::string::npos || tmp.find('{') == std::string::npos || tmp.size() == 0)
		throw Parser::WrongBracketsException();
	int par = 1;
	serv += tmp + '\n';
	while (1) {
		std::getline(in_file, tmp);
		if (tmp.size() == 0) {
			std::getline(in_file, tmp);
			if (in_file.eof())
				throw Parser::WrongBracketsException();
		}
		else if (tmp.find("server") != std::string::npos && tmp.find("server") == 0)
			throw Parser::WrongBracketsException();
		std::string sub = tmp;
		while (sub.find('{') != std::string::npos) {
			sub = sub.substr(sub.find('{') + 1, sub.size());
			++par;
		}
		sub = tmp;
		while (sub.find('}') != std::string::npos) {
			sub = sub.substr(sub.find('}') + 1, sub.size());
			--par;
		}
		serv += tmp + '\n';
		if (!par)
			return false;
	}
}
