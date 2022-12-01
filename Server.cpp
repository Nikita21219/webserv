//
// Created by Nikita Madorsky on 25.11.2022.
//

#include "Server.hpp"
#define BUF_SZ 2048

void Server::printErr(std::string s) {std::cout << ERROR << s << TERM_RESET;}

void Server::printWar(std::string s) {std::cout << WARNING << s << TERM_RESET;}

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
    socklen_t addrLen = sizeof(addr); //TODO delete line or return from func
    if (FD_ISSET(sock, set)) {
        if ((new_sock = accept(sock, (struct sockaddr *) addr, &addrLen)) < 0)
            return -1;
        if (new_sock > FD_SETSIZE)
            return close(new_sock);
        if (fcntl(new_sock, F_SETFL, O_NONBLOCK) < 0)
            return -1;
        fd_info fd_state;
        fd_state.readyToWriting = false;
        client_sockets.insert(client_sockets.begin(), std::pair<int, fd_info>(new_sock, fd_state));
    }
    return new_sock;
}

int Server::recieve(std::map<int, fd_info>::iterator it, char **buf) {
    ssize_t recv_res = recv(it->first, *buf, BUF_SZ, 0);
    if (recv_res < 0) {
        printErr("strerror from recieve: " + std::string(strerror(errno)) + "\n");
        return 1;
    }
    if (recv_res == 0) {
        FD_CLR(it->first, &read_set);
        FD_CLR(it->first, &write_set);
        close(it->first);
        client_sockets.erase(it);
        printWar("Client go away\n");
        return 0;
    }
    *(*buf + recv_res) = 0;

    std::ifstream file("templates/index.html");
    std::string s;
    if (file.is_open()) {
        while (std::getline(file, s)) {
            it->second.response += s + "\n";
        }
        file.close();
        it->second.readyToWriting = true;
    } else {
        printErr("error open index.html\n");
        return 1;
    }

    return 0;
}

int Server::sendResponse(std::map<int, fd_info>::iterator it, std::string filename) {
    (void) filename;
    std::stringstream response_body;
    std::stringstream response;

    response_body << it->second.response;

    // response_body << "<title>webserv</title>\n"
    //               << "<h1>Test page on first server bclarind</h1>\n"
    //               << "<p>This is body of the test page...</p>\n"
    //               << "<em><small>Test C++ Http Server</small></em>\n";

    response << "HTTP/1.1 200 OK\r\n"
             << "Version: HTTP/1.1\r\n"
             << "Content-Type: text/html; charset=utf-8\r\n"
             << "Content-Length: " << response_body.str().length()
             << "\r\n\r\n"
             << response_body.str();

    if (send(it->first, response.str().c_str(), response.str().length(), 0) < 0)
        return -1;
    FD_CLR(it->first, &write_set);
    it->second.readyToWriting = false;
    it->second.response = "";
    return 0;
}

void Server::mainLoop() {
    int max;
    char *buf = new char[BUF_SZ];
    fd_set tmp_read_set, tmp_write_set;
    struct sockaddr_in clientAddr = {};
    int listen_sock = getListenSocket(getAddr(8080));

    FD_SET(listen_sock, &read_set);
    while (true) {
        for (std::map<int, fd_info>::iterator i = client_sockets.begin(); i != client_sockets.end(); i++) {
            FD_SET(i->first, &read_set);
        }
        tmp_read_set = read_set;
        tmp_write_set = write_set;
        if (client_sockets.empty())
            max = listen_sock;
        else
            max = (--client_sockets.end())->first;
        if (select(max + 1, &tmp_read_set, &tmp_write_set, NULL, NULL) <= 0)
            continue;
        if (acceptNewConnection(listen_sock, &tmp_read_set, &clientAddr) < 0)
            continue;

        for (std::map<int, fd_info>::iterator it = client_sockets.begin(); it != client_sockets.end(); it++) {
            if (FD_ISSET(it->first, &tmp_read_set)) {
                if (recieve(it, &buf)) {
                    printErr("Recieve error\n");
                    continue;
                }
                FD_SET(it->first, &write_set);
            }
            if (it->second.readyToWriting && FD_ISSET(it->first, &tmp_write_set))
                if (sendResponse(it, "templates/index.html"))
                    continue;
        }
    }
}
