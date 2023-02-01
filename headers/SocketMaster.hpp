#ifndef SOCKETMASTER_HPP
# define SOCKETMASTER_HPP

# include "webserv.h"

class RequestHandler;

class SocketMaster
{

public:

    SocketMaster(std::vector<Parser> &conf, fd_set &read_set, int &max_sock, char **env);
    SocketMaster( SocketMaster const & src );
    ~SocketMaster();

    const std::vector<RequestHandler>& getClientSockets() const;
    SocketMaster &		operator=( SocketMaster const & rhs );
    void				acceptNewClients(fd_set &tmp_read_set, fd_set &read_set, int &max_sock);
    void				check_clients(fd_set &tmp_read_set, fd_set &tmp_write_set, fd_set &read_set, fd_set &write_set);

private:

    std::vector<Parser> const		&_conf;
    std::vector<RequestHandler>		_client_sockets;
    std::map<int, int>				_listen_sockets;
    char                            **_env;

    SocketMaster();
    bool	getListenSocket(struct sockaddr_in &addr, int id, std::string const &host);
    bool	checkdups(int id);
    void	removeClient(std::vector<RequestHandler>::iterator it, fd_set &read_set, fd_set &write_set);

};

std::ostream &			operator<<( std::ostream & o, SocketMaster const & i );

#endif /* **************************************************** SOCKETMASTER_H */