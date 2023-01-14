#ifndef DIRECTIVEPARSER_HPP
# define DIRECTIVEPARSER_HPP

# include <string>
# include <map>
# include <vector>
# include <stdexcept>
# include <iostream>
# include <fstream>
# include <sstream>
# include <iterator>
# include <algorithm>

# define KEY_WORDS_SIZE 13

class Parser;

class DirectiveParser
{

	public:

		DirectiveParser();
		DirectiveParser(std::string &str);
		DirectiveParser( DirectiveParser const & src );
		~DirectiveParser();

		DirectiveParser &			operator=( DirectiveParser const & rhs );
		std::map<std::string, std::string> const&	getContext() const;
		static std::string const	_key_words[KEY_WORDS_SIZE];

		class WrongBracketsException: public std::exception {
			public:
				virtual const char *what() const throw();
		};

	private:

		void								stringProcessing(std::string &str);
		void								saveContext(std::string &str);
		std::string							saveLocations(std::string &str);
		void								skipEmptyChars(std::string &tmp, std::string &str);
		void								divide_listen(std::string str);

		class ProblemWithDirectiveException: public std::exception {
			public:
				virtual const char *what() const throw();
		};
		class NestedLocationException: public std::exception {
			public:
				virtual const char *what() const throw();
		};

	protected:

		std::map<std::string, std::string>			_contexts;
		std::string									skipBlockDirective(std::string &str);

};

std::ostream &			operator<<( std::ostream & o, DirectiveParser const & i );

#endif /* ************************************************* DIRECTIVEPARSER_H */