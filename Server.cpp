//
// Created by Nikita Madorsky on 25.11.2022.
//

#include "Server.hpp"
#define BUF_SZ 1024

Server::Server(std::string ip, int port): ip(ip), port(port) {
    (void) this->port; //TODO tmp line
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    client_sockets.clear();
}

Server::~Server() {}

int Server::getListenSocket(struct sockaddr_in addr) {
    int listen_sock;
    int opt = 1;
    if ((listen_sock = socket(addr.sin_family, SOCK_STREAM, 0)) < 0)
        return -1;

    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        return -1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
        return -1;

    if (fcntl(listen_sock, F_SETFL, O_NONBLOCK) < 0)
        return -1;

    if (bind(listen_sock, (struct sockaddr*) &addr, sizeof(addr)))
        return -1;

    if (listen(listen_sock, SOMAXCONN))
        return -1;
    return listen_sock;
}

struct sockaddr_in Server::getAddr(int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    return addr;
}

int Server::acceptNewConnection(int sock, fd_set *set, struct sockaddr_in *addr) {
    int new_sock = 0;
    int addrLen = sizeof(addr); //TODO delete line or return from func
    if (FD_ISSET(sock, set)) {
        if ((new_sock = accept(sock, (struct sockaddr *) addr, (socklen_t *) &addrLen)) < 0)
            return -1;
        if (fcntl(new_sock, F_SETFL, O_NONBLOCK) < 0)
            return -1;
        client_sockets.insert(client_sockets.begin(), new_sock);
    }
    return new_sock;
}

void Server::mainLoop() {
    int max, recv_res;
    char buf[BUF_SZ];
    fd_set tmp_read_set;
    struct sockaddr_in clientAddr;
    struct sockaddr_in addr = getAddr(8080);
    int listen_sock = getListenSocket(addr);

    FD_SET(listen_sock, &read_set);
    while (true) {
        for (std::vector<int>::iterator i = client_sockets.begin(); i != client_sockets.end(); i++)
            FD_SET(*i, &read_set);
        tmp_read_set = read_set;
        if (client_sockets.size() < 1)
            max = listen_sock;
        else
            max = *std::max_element(client_sockets.begin(), client_sockets.end());
        if (select(max + 1, &tmp_read_set, NULL, NULL, NULL) <= 0)
            continue;
        if (acceptNewConnection(listen_sock, &tmp_read_set, &clientAddr) < 0)
            continue;

        for (std::vector<int>::iterator it = client_sockets.begin(); it != client_sockets.end(); it++) {
            if (FD_ISSET(*it, &read_set)) {
                recv_res = recv(*it, buf, BUF_SZ, 0);
                if (recv_res < 0)
                    continue;
                if (recv_res == 0) {
                    FD_CLR(*it, &read_set);
                    close(*it);
                    client_sockets.erase(it);
                    continue;
                }
                buf[recv_res] = 0;
                std::stringstream response_body;
                std::stringstream response;
                response_body << "<title>Test C++ HTTP Server</title>\n"
                              << "<h1>Test page on first server bclarind</h1>\n"
                              << "<p>This is body of the test page...</p>\n"
                              << "<h2>Request headers</h2>\n"
                              << "<pre>" << buf << "</pre>\n"
                              << "<em><small>Test C++ Http Server</small></em>\n";

                response << "HTTP/1.1 200 OK\r\n"
                         << "Version: HTTP/1.1\r\n"
                         << "Content-Type: text/html; charset=utf-8\r\n"
                         << "Content-Length: " << response_body.str().length()
                         << "\r\n\r\n"
                         << response_body.str();

                if (send(*it, response.str().c_str(), response.str().length(), 0) < 0)
                    continue;
            }
        }
    }
}
