#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

# include "webserv.h"

class ResponseHandler
{

public:

    ResponseHandler(char **env);
    ResponseHandler( ResponseHandler const & src );
    ~ResponseHandler();

    std::map<std::string, std::string>&			setHeader();
    std::string&								setMethods();
    std::string&								setLocation();
    std::string&								setRoot();
    std::string&								setPath();
    const Parser*&								setConf();
    int&										setStatus_code();
    std::time_t&								setLast_modified();
    std::vector<unsigned char>&					setData();
    std::vector<unsigned char>&					setResponse_data();

    ResponseHandler &		operator=( ResponseHandler const & rhs );
    int		prepareAnswer();
    void	extract_info(const Parser *conf, int fd);
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
    struct S {
        const char  *key;
        const char  *val;

        operator std::map<std::string, std::string>::value_type() const {
            return std::pair<std::string, std::string>(key, val);
        }
    };
    static const S                      _typePairs[];
    static const std::map<std::string, std::string> _mime_types;

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
    char    							**_env;
    int									_client_socket;

    int			                        answerToGET();
    int			                        generateErrorPage();
    int			                        answerToPOST();
    int			                        answerToDELETE();
    bool		                        folderIsNotEmpty(std::string &resource_path) const;
    std::string	                        getResourse_path() const;
    void		                        findLocation();
    bool		                        add_index_if_needed(std::string &resource_path);
    void		                        read_binary_file(const std::string filename);
    void		                        createHTTPheader(std::string mimeType, std::string file_loc, bool flag);
    std::string	                        setMimeType(std::string &path);
    std::string	                        getDate(std::time_t t);
    void		                        generateHTML();
    void		                        genereteWelcomePage();
    int                                 handleCgi();
    bool                                isCgi();
    std::vector<std::string>            getListFiles(std::string path);
    std::string                         getHTMLPage(std::string path, std::string location);

    std::string	create_filename();
    void		check_path_errors();
    void		successful_response_html(std::string s);

};

std::ostream &			operator<<( std::ostream & o, ResponseHandler const & i );

#endif /* ************************************************* RESPONSEHANDLER_H */