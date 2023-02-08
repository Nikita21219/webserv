#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

# include "webserv.h"

# define BUF_SZ 4096

class ResponseHandler;

class RequestHandler
{

public:

    enum request_status {
        NEW,
        MUST_KEEP_READING,
        READY_TO_ASWER,
        ERROR_IN_REQUEST
    };

    RequestHandler(std::vector<Parser> const &conf, int sock, unsigned long id, request_status s, char **env);
    RequestHandler( RequestHandler const & src );
    ~RequestHandler();

    RequestHandler &		operator=( RequestHandler const & rhs );
    int						get_sock() const;
    request_status&			setStatus();
    void					serve_client(fd_set &write_set);
    void					sendResponse(fd_set &write_set);

private:

    std::vector<Parser> const			&_conf;
    unsigned long						_serv_id;
    int									_client_socket;
    request_status						_status;
    ResponseHandler						*_answer;
    unsigned char						_buf[BUF_SZ + 1];

    std::vector<unsigned char>			_chunk;
    int                                 _chunk_size;

    RequestHandler();
    void	new_reading();
    void	continue_reading();
    void    chanked_handler(ssize_t size, ssize_t start);
    void	parse_string(std::string str);
    int		select_serv(std::string const str);
    void	download_data(ssize_t size, ssize_t header_size);
    bool	multipart_parser(ssize_t &header_size);

};

std::ostream &			operator<<( std::ostream & o, RequestHandler const & i );

#endif /* ************************************************** REQUESTHANDLER_H */