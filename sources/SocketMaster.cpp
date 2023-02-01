#include "SocketMaster.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

SocketMaster::SocketMaster(std::vector<Parser> &conf, fd_set &read_set, int &max_sock, char **env): _conf(conf), _env(env) {
    struct sockaddr_in	addr;
    int					port;
    int					err = 0;
    addr.sin_family = AF_INET;
    for (unsigned long i = 0; i < conf.size(); ++i) {
        if (_conf[i].getServfield("port") == NOT_FOUND) {
            port = 8080;
            conf[i].changefield("port", "8080");
        }
        else
            port = atoi(_conf[i].getServfield("port").c_str());
        addr.sin_port = htons(port);
        if (_conf[i].getServfield("host") == NOT_FOUND) {
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            conf[i].changefield("host", "0.0.0.0");
        } else if (!(inet_aton(_conf[i].getServfield("host").c_str(), &(addr.sin_addr))))
            throw std::runtime_error("host not found in " + _conf[i].getServfield("listen") + " of the listen directive in config!");
        if (checkdups(i))
            continue;
        if (_conf[i].getServfield("listen") == NOT_FOUND)
            err = getListenSocket(addr, i, "0.0.0.0:8080");
        else
            err = getListenSocket(addr, i, _conf[i].getServfield("listen"));
        if (!err) {
            FD_SET(_listen_sockets[i], &read_set);
            if (_listen_sockets[i] >= max_sock)
                max_sock = _listen_sockets[i] + 1;
        }
    }
}

SocketMaster::SocketMaster( const SocketMaster & src ): _conf(src._conf), _client_sockets(src._client_sockets),\
					 _listen_sockets(src._listen_sockets) {}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

SocketMaster::~SocketMaster() {}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/

SocketMaster &				SocketMaster::operator=( SocketMaster const & rhs ) {
    if ( this != &rhs ) {
        _listen_sockets = rhs._listen_sockets;
        _client_sockets = rhs._client_sockets;
    }
    return *this;
}

std::ostream &			operator<<( std::ostream & o, SocketMaster const & i ) {
    (void)i;
    //o << "Value = " << i.getValue();
    return o;
}

/*
** --------------------------------- METHODS ----------------------------------
*/

bool SocketMaster::getListenSocket(struct sockaddr_in &addr, int id, std::string const &host) {
    int opt = 1;
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        return 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        return 1;
    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
        return 1;
    if (bind(sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)))
        throw std::runtime_error("Error: bind() to " + host + " failed!");
    if (listen(sock, MAX_QUEUE))
        return 1;
    _listen_sockets.insert(_listen_sockets.end(), std::pair<int, int>(id, sock));
    return 0;
}

void SocketMaster::acceptNewClients(fd_set &tmp_read_set, fd_set &read_set, int &max_sock) {
    int					client_sock;
    struct sockaddr_in	client;
    socklen_t			size = sizeof(client);
    for (unsigned long i = 0; i < _conf.size(); ++i) {
        if (_listen_sockets.find(i) != _listen_sockets.end() && FD_ISSET(_listen_sockets[i], &tmp_read_set)) {
            if ((client_sock = accept(_listen_sockets[i], reinterpret_cast<struct sockaddr *>(&client), &size)) < 0)
                continue;
            if (client_sock > FD_SETSIZE || fcntl(client_sock, F_SETFL, O_NONBLOCK) < 0) {
                close(client_sock);
                continue;
            }
            if (max_sock <= client_sock)
                max_sock = client_sock + 1;
            FD_SET(client_sock, &read_set);
            _client_sockets.push_back(RequestHandler(_conf, client_sock, i, RequestHandler::NEW, _env));
        }
    }
}

void	SocketMaster::check_clients(fd_set &tmp_read_set, fd_set &tmp_write_set, fd_set &read_set, fd_set &write_set) {

    for (std::vector<RequestHandler>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); ) {
        if (FD_ISSET(it->get_sock(), &tmp_read_set)) {
            try {
                it->serve_client(write_set);
            } catch (int) {
                removeClient(it, read_set, write_set);
                continue;
            }
        }
        if (FD_ISSET(it->get_sock(), &tmp_write_set) && (it->setStatus() == RequestHandler::READY_TO_ASWER ||\
					it->setStatus() == RequestHandler::ERROR_IN_REQUEST)) {
            try {
                it->sendResponse(write_set);
            } catch (int) {
                removeClient(it, read_set, write_set);
                continue;
            }
        }
        ++it;
    }
}

void SocketMaster::removeClient(std::vector<RequestHandler>::iterator it, fd_set &read_set, fd_set &write_set) {
    FD_CLR(it->get_sock(), &read_set);
    FD_CLR(it->get_sock(), &write_set);
    close(it->get_sock());
    _client_sockets.erase(it);
}

bool SocketMaster::checkdups(int id) {
    for (int j = 0; j < id; ++j) {
        if (_conf[j].getServfield("host") == _conf[id].getServfield("host") &&\
				_conf[j].getServfield("port") == _conf[id].getServfield("port"))
            return true;
    }
    return false;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

const std::vector<RequestHandler>& SocketMaster::getClientSockets() const {
    return _client_sockets;
}

/* ************************************************************************** */