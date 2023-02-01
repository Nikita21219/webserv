#include "Parser.hpp"


/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Parser::Parser()
{
}

Parser::Parser(std::string &data): DirectiveParser(data) {
	while (1) {
		if (data.find("location") != std::string::npos) {
			std::string loc = skipBlockDirective(data);
			std::string loc_key = getLocationkey(loc);
			if (_locations.size() > 0 && _locations.find(loc_key) != _locations.end())
				throw Parser::LocationRepeatException();
			_locations.insert(_locations.end(), std::pair<std::string, DirectiveParser>(loc_key, DirectiveParser(loc)));
		}
		else
			break;
	}
}

Parser::Parser( const Parser & src ): DirectiveParser(src)
{
	_contexts = src._contexts;
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
		_contexts = rhs._contexts;
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

std::string Parser::findkeywordbyref(std::string const &key, DirectiveParser const &dir) const {

	if (dir.getContext().find(key) != dir.getContext().end())
		return dir.getContext().find(key)->second;
	else
		return NOT_FOUND;

}

const char* Parser::LocationFieldException::what() const throw() {
	return "The location field in your config file is wrong!";
}

const char* Parser::LocationRepeatException::what() const throw() {
	return "There are two identical locations in your config file!";
}


/*
** --------------------------------- ACCESSOR ---------------------------------
*/

std::string Parser::getServfield(std::string const &key) const {
	return findkeywordbyref(key, *this);
}

std::string Parser::getLocfield(std::string const &path, std::string const &key) const {
	if (_locations.find(path) != _locations.end())
		return findkeywordbyref(key, _locations.find(path)->second);
	else
		return NOT_FOUND;
}

void Parser::changefield(std::string const &key, std::string const &val) {
	if (_contexts.find(key) != _contexts.end())
		_contexts[key] = val;
	else
		_contexts.insert(_contexts.end(), std::pair<std::string, std::string>(key, val));
}

bool Parser::isThereLocation(std::string const &path) const {
	if (_locations.find(path) != _locations.end())
		return true;
	else
		return false;
}

/* ************************************************************************** */