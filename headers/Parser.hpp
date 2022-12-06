#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <iterator>
# include <algorithm>
# include <vector>
# include <map>

# define NOT_FOUND "not found"

class Parser
{

	public:

		Parser();
		Parser(std::string &data);
		Parser( Parser const & src );
		~Parser();

		bool				geterror() const;
		std::string			getServfield(std::string const &key);
		std::string			getLocfield(std::string const &path, std::string const &key);
		Parser &			operator=( Parser const & rhs );

		static std::string	_key_words[12];

	private:

		bool								_error;
		std::map<std::string, std::string>	_conf;
		std::map<std::string, Parser>		_locations;

		bool								parsLocations(std::string &str);
		bool								parsConf(std::string &str);
		std::string							getLocationkey(std::string &loc);
		std::string							getLocationstr(std::string &str);
		bool								findkeyword(std::string &str);
		std::string							findkeywordbyref(std::string const &key, Parser &loc);
		std::string							findlocationrecursive(std::string const &path, std::string const &key, Parser &loc);

};

bool			get_conf(char *file, std::vector<Parser> &conf);
std::string		get_one_serv(std::ifstream &in_file);

std::ostream &	operator<<( std::ostream & o, Parser const & i );

#endif /* ********************************************************** PARSER_H */