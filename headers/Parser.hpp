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

		std::string					getServfield(std::string const &key);
		std::string					getLocfield(std::string const &path, std::string const &key);
		Parser &					operator=( Parser const & rhs );

		static std::string const	_key_words[12];

		class WrongBracketsException: public std::exception {
			public:
				virtual const char *what() const throw();
		};

	private:

		std::map<std::string, std::string>	_conf;
		std::map<std::string, Parser>		_locations;

		void								parsLocations(std::string &str);
		void								parsConf(std::string &str);
		std::string							getLocationkey(std::string &loc);
		std::string							getLocationstr(std::string &str);
		void								findkeyword(std::string &str);
		std::string							findkeywordbyref(std::string const &key, Parser &loc);
		std::string							findlocationrecursive(std::string const &path, std::string const &key, Parser &loc);

		class LocationFieldException: public std::exception {
			public:
				virtual const char *what() const throw();
		};
		class LocationRepeatException: public std::exception {
			public:
				virtual const char *what() const throw();
		};
		class ProblemWithConfigException: public std::exception {
			public:
				virtual const char *what() const throw();
		};
/*		class WrongBracketsException: public std::exception {
			public:
				virtual const char *what() const throw();
		};*/

};

bool			get_conf(char *file, std::vector<Parser> &conf);
bool			get_one_serv(std::ifstream &in_file, std::string &serv);

std::ostream &	operator<<( std::ostream & o, Parser const & i );

#endif /* ********************************************************** PARSER_H */