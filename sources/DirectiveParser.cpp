#include "DirectiveParser.hpp"

std::string const DirectiveParser::_key_words[13] = {"listen", "server_name", "root", "autoindex", "methods", \
		"max_body_size", "directory", "index", "bin_path", "redirection", "alias", "cgi_pass", "location"};

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

DirectiveParser::DirectiveParser()
{
}

DirectiveParser::DirectiveParser(std::string &str) {
	stringProcessing(str);
}

DirectiveParser::DirectiveParser( const DirectiveParser & src )
{
	_contexts = src._contexts;
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

DirectiveParser::~DirectiveParser()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

DirectiveParser &				DirectiveParser::operator=( DirectiveParser const & rhs )
{
	if ( this != &rhs )
	{
		_contexts = rhs._contexts;
	}
	return *this;
}

std::ostream &			operator<<( std::ostream & o, DirectiveParser const & i )
{
	(void)i;
	//o << "Value = " << i.getValue();
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/

void DirectiveParser::stringProcessing(std::string &str) {
	std::string saveloc;
	if (str.find("server") != std::string::npos)
		str.erase(0, str.find('\n') + 1);
	while (1) {
		saveloc += saveLocations(str);
		std::string tmp = str.substr(0, str.find('\n'));
		while (!tmp.size() && !str.empty()) {
			str.erase(0, str.find('\n') + 1);
			saveloc += saveLocations(str);
			tmp = str.substr(0, str.find('\n'));
		}
		skipEmptyChars(tmp, str);
		if (!tmp.size() && !str.size())
			break;
		else if (!tmp.size())
			continue;
		saveContext(tmp);
		str.erase(0, str.find('\n') + 1);
	}
	str = saveloc;
}

void DirectiveParser::skipEmptyChars(std::string &tmp, std::string &str) {
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
}

void DirectiveParser::saveContext(std::string &str) {
	for (int i = 0; i < 13; ++i) {
		if (str.find(_key_words[i]) != std::string::npos) {
			str.erase(0, str.find(_key_words[i]) + _key_words[i].size());
			if (str[0] != ' ' && str.size() > 0)
				throw DirectiveParser::ProblemWithDirectiveException();
			for (size_t j = 0; j < str.size(); ++j) {
				if (str[j] == ' ')
					continue;
				else {
					str = str.substr(j, str.find('\n'));
					break;
				}
			}
			if (_contexts.size() > 0 && _contexts.find(_key_words[i]) != _contexts.end())
				throw DirectiveParser::ProblemWithDirectiveException();
			_contexts.insert(_contexts.end(), std::pair<std::string, std::string>(_key_words[i], str));
			return ;
		}
		if (i == 12)
			throw DirectiveParser::ProblemWithDirectiveException();
	}
}

std::string DirectiveParser::saveLocations(std::string &str) {
	std::string res;
	while (str.substr(0, str.find('\n')).find(_key_words[12]) != std::string::npos)
		res += skipBlockDirective(str);
	return res;
}

std::string DirectiveParser::skipBlockDirective(std::string &str) {
	int end = 8;
	int par = 0;
	int flag = 1;
	while (1) {
		if (str[end] == '{' && par)
			throw DirectiveParser::NestedLocationException();
		else if (str[end] == '{') {
			++par;
			flag = 0;
		} else if (str[end] == '}' && flag)
			throw DirectiveParser::WrongBracketsException();
		else if (str[end] == '}')
			--par;
		++end;
		if (!par && !flag)
			break ;
	}
	std::string res = str.substr(0, end + 1);
	str.erase(0, end + 1);
	return res;
}

const char* DirectiveParser::ProblemWithDirectiveException::what() const throw() {
	return "Problem with a field in your config file!";
}

const char* DirectiveParser::NestedLocationException::what() const throw() {
	return "Error: nested location!";
}

const char* DirectiveParser::WrongBracketsException::what() const throw() {
	return "Brackets are incorrect!";
}


/*
** --------------------------------- ACCESSOR ---------------------------------
*/

std::map<std::string, std::string> const& DirectiveParser::getContext() const {
	return _contexts;
}


/* ************************************************************************** */