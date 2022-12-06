
#include "Parser.hpp"

bool get_conf(char *file, std::vector<Parser> &conf) {
	std::ifstream in_file;
	std::string serv;
	in_file.open(file);
	if (!in_file.is_open()) {
		std::cerr << "Config wasn't opened!\n";
		return true;
	}
	while(1) {
		serv = get_one_serv(in_file);
		if (serv.find("error") != std::string::npos && serv.find("error") == 0) {
			std::cerr << "Wrong config file!\n";
			return true;
		}
		if (serv.find("end") != std::string::npos && serv.find("end") == 0)
			break;
		conf.push_back(Parser(serv));
		if ((*(--(conf.end()))).geterror()) {
			std::cerr << "Wrong config file!\n";
			return true;
		}
	}
	return false;
}

std::string get_one_serv(std::ifstream &in_file) {
	std::string res;
	std::string tmp;
	std::getline(in_file, tmp);
	while (tmp.size() == 0) {
		std::getline(in_file, tmp);
		if (in_file.eof())
			return "end";
	}
	if (tmp.find("server") == std::string::npos || tmp.find('{') == std::string::npos || tmp.size() == 0)
		return "error";
	int par = 1;
	res += tmp + '\n';
	while (1) {
		std::getline(in_file, tmp);
		if (tmp.size() == 0) {
			std::getline(in_file, tmp);
			if (in_file.eof())
				return "error";
		}
		else if (tmp.find("server") != std::string::npos && tmp.find("server") == 0)
			return "error";
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
		res += tmp + '\n';
		if (!par)
			return res;
	}
}
