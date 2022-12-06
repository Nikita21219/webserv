#include "Parser.hpp"

std::string Parser::_key_words[12] = {"listen", "server_name", "root", "autoindex", "methods", \
		"max_body_size", "directory", "index", "bin_path", "redirection", "alias", "cgi_pass"};

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Parser::Parser(): _error(0)
{
}

Parser::Parser(std::string &data): _error(0) {
	if (parsLocations(data))
		return ;
	if (parsConf(data))
		return ;
}

Parser::Parser( const Parser & src )
{
	_error = src.geterror();
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
		_error = rhs.geterror();
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

bool Parser::parsLocations(std::string &str) {
	while (1) {
		if (str.find("location") != std::string::npos) {
			std::string loc = getLocationstr(str);
			std::string loc_key = getLocationkey(loc);
			if (loc_key.find("error") != std::string::npos && loc_key.find("error") == 0) {
				_error = 1;
				return true;
			}
			if (_locations.size() > 0 && _locations.find(loc_key) != _locations.end()) {
				_error = 1;
				return true;
			}
			_locations.insert(_locations.end(), std::pair<std::string, Parser>(loc_key, Parser(loc)));
			if (_locations[loc_key].geterror()) {
				_error = 1;
				return true;
			}
		}
		else
			break;
	}
	return false;
}

std::string Parser::getLocationkey(std::string &loc) {
	std::string res = loc.substr(0, loc.find('\n'));
	if (res[res.find("location") + 8] != ' ')
		return "error";
	res.erase(0, res.find(' ') + 1);
	int i = 0;
	while (res[i] == ' ')
		i++;
	if (i > 0)
		res.erase(0, i);
	if (res.find(' ') == std::string::npos)
		return "error";
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

bool Parser::parsConf(std::string &str) {
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
			return false;
		else if (!tmp.size())
			continue;
		if (findkeyword(tmp)) {
			_error = 1;
			return true;
		}
		str.erase(0, str.find('\n'));
	}
	return false;
}

bool Parser::findkeyword(std::string &str) {
	for (int i = 0; i < 12; ++i) {
		if (str.find(_key_words[i]) != std::string::npos) {
			str.erase(0, str.find(_key_words[i]) + _key_words[i].size());
			for (size_t j = 0; j < str.size(); ++j) {
				if (str[j] == ' ')
					continue;
				else {
					str = str.substr(j, str.find('\n'));
					break;
				}
			}
			if (_conf.size() > 0 && _conf.find(_key_words[i]) != _conf.end())
				return true;
			_conf.insert(_conf.end(), std::pair<std::string, std::string>(_key_words[i], str));
			return false;
		}
		if (i == 11)
			return true;
	}
	return false;
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


/*
** --------------------------------- ACCESSOR ---------------------------------
*/

bool Parser::geterror() const {
	return _error;
}

std::string Parser::getServfield(std::string const &key) {
	return findkeywordbyref(key, *this);
}

std::string Parser::getLocfield(std::string const &path, std::string const &key) {
	return findlocationrecursive(path, key, *this);
}

/* ************************************************************************** */