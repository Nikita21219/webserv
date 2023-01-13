#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

# include "webserv.h"

class ResponseHandler
{

	public:

		ResponseHandler(char **env);
		ResponseHandler( ResponseHandler const & src );
		~ResponseHandler();

		const std::map<std::string, std::string>&	getHeader() const;
		std::map<std::string, std::string>&			setHeader();
		const std::string&							getMethods() const;
		const std::string&							getLocation() const;
		const Parser*								getConf() const;
		int&										setStatus_code();
		void*&										setData();
		ssize_t&									setData_size();

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
		void								*_data;
		ssize_t								_data_size;
		std::time_t							_last_modified;

		std::string							_path;
		std::string							_location;
		std::string							_root;
		std::string							_methods;
		char    							**_env;
		std::vector<unsigned char>			_response_data;

		int			answerToGET();
		void		generateErrorPage();
		void		answerToPOST();
		void		answerToDELETE();
		void		findLocation();
		bool		add_index_if_needed(std::string &resource_path);
		void		read_binary_file(const std::string filename);
		void        createHTTPheader(std::string mimeType, bool flag, std::string redirectTo = NOT_FOUND);
		std::string	setMimeType(std::string &path);
		std::string	getDate(std::time_t t);
		void		generateHTML();
        int         handleCgi();
};

std::ostream &			operator<<( std::ostream & o, ResponseHandler const & i );

#endif /* ************************************************* RESPONSEHANDLER_H */