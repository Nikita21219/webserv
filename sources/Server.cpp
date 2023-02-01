//
// Created by Nikita Madorsky on 25.11.2022.
//

#include "Server.hpp"

Server::Server(std::vector<Parser> &conf, char **env): _conf(conf), _max_sock(0) {
    FD_ZERO(&_read_set);
    FD_ZERO(&_write_set);
    _sockets = new SocketMaster(conf, _read_set, _max_sock, env);
}

Server::Server( const Server & src ): _conf(src._conf), _read_set(src._read_set),\
				_write_set(src._write_set), _max_sock(src._max_sock) {
    _sockets = new SocketMaster(*src._sockets);
}

Server::~Server() {
    delete _sockets;
}

Server &				Server::operator=( Server const & rhs ) {
    if ( this != &rhs ) {
        _read_set = rhs._read_set;
        _write_set = rhs._write_set;
        _max_sock = rhs._max_sock;
        delete _sockets;
        _sockets = new SocketMaster(*rhs._sockets);
    }
    return *this;
}

void Server::mainLoop() {
    fd_set tmp_read_set, tmp_write_set;

    while (true) {
        tmp_read_set = _read_set;
        tmp_write_set = _write_set;
        if (select(_max_sock, &tmp_read_set, &tmp_write_set, NULL, NULL) <= 0)
            continue;

        _sockets->acceptNewClients(tmp_read_set, _read_set, _max_sock);

        _sockets->check_clients(tmp_read_set, tmp_write_set, _read_set, _write_set);

//        if (!_sockets->getClientSockets().size())
//            break;

    }
}
