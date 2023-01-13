#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

# include "webserv.h"

class ResponseHandler
{

	public:

		ResponseHandler();
		ResponseHandler( ResponseHandler const & src );
		~ResponseHandler();

		const std::map<std::string, std::string>&	getHeader() const;
		std::map<std::string, std::string>&			setHeader();
		const std::string&							getMethods() const;
		const std::string&							getLocation() const;
		const Parser*								getConf() const;
		int&										setStatus_code();
		std::vector<unsigned char>&					setData();

		const std::string&							getPath() const;

		ResponseHandler &		operator=( ResponseHandler const & rhs );
		int		prepareAnswer();
		void	extract_info(const Parser *conf);
		void	sendResponseToClient(int fd);

	private:

		struct T {
			int			num;
			const char	*name;

			operator std::map<int, std::string>::value_type() const {
				return std::pair<int, std::string>(num, name);
			}
		};
		static const T							_statusPairs[];
		static const std::map<int, std::string>	_status_codes;

		std::map<std::string, std::string>	_header;
		const Parser						*_conf;
		int									_status_code;
		std::vector<unsigned char>			_data;
		std::time_t							_last_modified;

		std::string							_path;
		std::string							_location;
		std::string							_root;
		std::string							_methods;
		std::vector<unsigned char>			_response_data;

		int			answerToGET();
		int			generateErrorPage();
		int			answerToPOST();
		int			answerToDELETE();
		bool		folderIsNotEmpty(std::string &resource_path) const;
		std::string	getResourse_path() const;
		void		findLocation();
		bool		add_index_if_needed(std::string &resource_path);
		void		read_binary_file(const std::string filename);
		void		createHTTPheader(std::string mimeType, std::string location, std::string allow, bool flag);
		std::string	setMimeType(std::string &path);
		std::string	getDate(std::time_t t);
		void		generateHTML();

		std::string	create_filename() const;
		void		successful_response_html(std::string s);

};

std::ostream &			operator<<( std::ostream & o, ResponseHandler const & i );

#endif /* ************************************************* RESPONSEHANDLER_H */