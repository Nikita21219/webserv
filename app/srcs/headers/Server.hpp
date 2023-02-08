//
// Created by Nikita Madorsky on 25.11.2022.
//

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

#include "webserv.h"

#define MAX_QUEUE 50

class SocketMaster;

class Server {
public:
    Server(std::vector<Parser> &conf, char **env);
    Server( Server const & src );
    ~Server();
    Server &	operator=( Server const & rhs );
    void		mainLoop();

private:
    std::vector<Parser> const	&_conf;
    SocketMaster				*_sockets;
    fd_set						_read_set;
    fd_set						_write_set;
    int							_max_sock;
    Server();
};

#endif //WEBSERV_SERVER_HPP