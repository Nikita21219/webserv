#include "Parser.hpp"

std::string const Parser::_key_words[12] = {"listen", "server_name", "root", "autoindex", "methods", \
		"max_body_size", "directory", "index", "bin_path", "redirection", "alias", "cgi_pass"};

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Parser::Parser()
{
}

Parser::Parser(std::string &data) {
	parsLocations(data);
	parsConf(data);
}

Parser::Parser( const Parser & src )
{
	_conf = src._conf;
	_locations = src._locations;
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Parser::~Parser()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Parser &				Parser::operator=( Parser const & rhs )
{
	if ( this != &rhs )
	{
		_conf = rhs._conf;
		_locations = rhs._locations;
	}
	return *this;
}

std::ostream &			operator<<( std::ostream & o, Parser const & i )
{
	(void)i;
	//o << "Value = " << i.getValue();
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/

void Parser::parsLocations(std::string &str) {
	while (1) {
		if (str.find("location") != std::string::npos) {
			std::string loc = getLocationstr(str);
			std::string loc_key = getLocationkey(loc);
			if (_locations.size() > 0 && _locations.find(loc_key) != _locations.end())
				throw Parser::LocationRepeatException();
			_locations.insert(_locations.end(), std::pair<std::string, Parser>(loc_key, Parser(loc)));
		}
		else
			break;
	}
	return ;
}

std::string Parser::getLocationkey(std::string &loc) {
	std::string res = loc.substr(0, loc.find('\n'));
	if (res[res.find("location") + 8] != ' ')
		throw Parser::LocationFieldException();
	res.erase(0, res.find(' ') + 1);
	int i = 0;
	while (res[i] == ' ')
		i++;
	if (i > 0)
		res.erase(0, i);
	if (res.find(' ') == std::string::npos)
		throw Parser::LocationFieldException();
	res.erase(res.find(' '), res.find('\n'));
	loc.erase(0, loc.find('\n'));
	return res;
}

std::string Parser::getLocationstr(std::string &str) {
	int start = str.find("location");
	int end = start + 8;
	int par = 0;
	int flag = 1;
	while (1) {
		if (str[end] == '{') {
			++par;
			flag = 0;
		}
		if (str[end] == '}')
			--par;
		++end;
		if (!par && !flag)
			break ;
	}
	end -= str.find("location");
	std::string res = str.substr(start, end + 1);
	str.erase(start, end + 1);
	return res;
}

void Parser::parsConf(std::string &str) {
	if (str.find("server") != std::string::npos)
		str.erase(0, str.find('\n'));
	while (1) {
		std::string tmp = str.substr(0, str.find('\n'));
		while (!tmp.size()) {
			str.erase(0, str.find('\n') + 1);
			tmp = str.substr(0, str.find('\n'));
		}
		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '\t' || str[i] == ' ' || str[i] == '}')
				continue;
			else if (str[i] == '\n') {
				str.erase(0, str.find('\n') + 1);
				tmp.clear();
				break;
			}
			else
				break;
		}
		if (!tmp.size() && !str.size())
			return ;
		else if (!tmp.size())
			continue;
		findkeyword(tmp);
		str.erase(0, str.find('\n'));
	}
}

void Parser::findkeyword(std::string &str) {
	for (int i = 0; i < 12; ++i) {
		if (str.find(_key_words[i]) != std::string::npos) {
			str.erase(0, str.find(_key_words[i]) + _key_words[i].size());
			if (str[0] != ' ' && str.size() > 0)
				throw Parser::ProblemWithConfigException();
			for (size_t j = 0; j < str.size(); ++j) {
				if (str[j] == ' ')
					continue;
				else {
					str = str.substr(j, str.find('\n'));
					break;
				}
			}
			if (_conf.size() > 0 && _conf.find(_key_words[i]) != _conf.end())
				throw Parser::ProblemWithConfigException();
			_conf.insert(_conf.end(), std::pair<std::string, std::string>(_key_words[i], str));
			return ;
		}
		if (i == 11)
			throw Parser::ProblemWithConfigException();
	}
}

std::string Parser::findkeywordbyref(std::string const &key, Parser &loc) {
	if (loc._conf.find(key) != loc._conf.end())
		return loc._conf[key];
	else
		return NOT_FOUND;

}

std::string Parser::findlocationrecursive(std::string const &path, std::string const &key, Parser &loc) {
	if (loc._locations.find(path) != loc._locations.end())
		return findkeywordbyref(key, loc._locations[path]);
	std::string tmp = path;
	std::string depth_path;
	while (1) {
		for (int i = tmp.size(); i > 0; --i) {
			if (tmp[i] == '/') {
				depth_path = tmp.substr(i, tmp.size()) + depth_path;
				tmp = tmp.erase(i, depth_path.size());
				break;
			}
			if (i == 1 && tmp[0] == '/') {
				tmp.erase(0, 1);
				if (loc._locations.find(tmp) != loc._locations.end())
					return findkeywordbyref(key, loc._locations[tmp]);
				tmp.clear();
			} else if (i == 1)
				tmp.clear();
		}
		if (!tmp.size())
			return NOT_FOUND;
		else if (loc._locations.find(tmp) != loc._locations.end())
			return findlocationrecursive(depth_path, key, loc._locations[tmp]);
	}
}

const char* Parser::LocationFieldException::what() const throw() {
	return "The location field in your config file is wrong!";
}

const char* Parser::LocationRepeatException::what() const throw() {
	return "There are two identical locations in your config file!";
}

const char* Parser::ProblemWithConfigException::what() const throw() {
	return "Problem with a field in your config file!";
}

const char* Parser::WrongBracketsException::what() const throw() {
	return "Brackets are incorrect!";
}


/*
** --------------------------------- ACCESSOR ---------------------------------
*/

std::string Parser::getServfield(std::string const &key) {
	return findkeywordbyref(key, *this);
}

std::string Parser::getLocfield(std::string const &path, std::string const &key) {
	return findlocationrecursive(path, key, *this);
}

/* ************************************************************************** */