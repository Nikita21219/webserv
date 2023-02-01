#ifndef PARSER_HPP
# define PARSER_HPP

# include "DirectiveParser.hpp"

# define NOT_FOUND "not found"

class Parser: public DirectiveParser
{

	public:

		Parser();
		Parser(std::string &data);
		Parser( Parser const & src );
		virtual ~Parser();

		std::string					getServfield(std::string const &key) const;
		std::string					getLocfield(std::string const &path, std::string const &key) const;
		void						changefield(std::string const &key, std::string const &val);
		bool						isThereLocation(std::string const &path) const;
		Parser &					operator=( Parser const & rhs );

	private:

		std::map<std::string, DirectiveParser>	_locations;

		std::string							getLocationkey(std::string &loc);
		std::string							findkeywordbyref(std::string const &key, DirectiveParser const &dir) const;

		class LocationFieldException: public std::exception {
			public:
				virtual const char *what() const throw();
		};
		class LocationRepeatException: public std::exception {
			public:
				virtual const char *what() const throw();
		};

};

bool			get_conf(const char *file, std::vector<Parser> &conf);
bool			get_one_serv(std::ifstream &in_file, std::string &serv);

std::ostream &	operator<<( std::ostream & o, Parser const & i );

#endif /* ********************************************************** PARSER_H */