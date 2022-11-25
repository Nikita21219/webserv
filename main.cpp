#include <iostream>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <vector>

#include "Server.hpp"

#define BUF_SIZE 1024

int exitFail(std::string s) {
    std::cerr << s;
    exit(EXIT_FAILURE);
}

int main() {

    Server serv = Server("127.0.0.1", 8080);

    int listen_sock, read_res, max;
    int opt = 1;
    int num_socks = 0;
    int new_sock = -1;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    char buf[BUF_SIZE];

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if ((listen_sock = socket(addr.sin_family, SOCK_STREAM, 0)) < 0)
        exitFail("Socket error\n");
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        exitFail("Setsockopt SO_REUSEADDR error\n");
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
        exitFail("Setsockopt SO_REUSEPORT error\n");
    if (fcntl(listen_sock, F_SETFL, O_NONBLOCK) < 0)
        exitFail("Fcntl error\n");

    if (bind(listen_sock, (struct sockaddr*) &addr, sizeof(addr)))
        exitFail("Bind error\n");

    if (listen(listen_sock, SOMAXCONN))
        exitFail("Listen error\n");


    fd_set read_set;
    fd_set tmp_read_set;
    FD_ZERO(&read_set);
    FD_SET(listen_sock, &read_set);
    num_socks++;
    std::vector<int> clients;
    while (true) {
        for (std::vector<int>::iterator i = clients.begin(); i != clients.end(); i++) {
            FD_SET(*i, &read_set);
            num_socks++;
        }

        if (clients.size() < 1)
            max = listen_sock;
        else
            max = *std::max_element(clients.begin(), clients.end());

        tmp_read_set = read_set;
        if (select(max + 1, &tmp_read_set, NULL, NULL, NULL) <= 0)
            exitFail("Select error\n");

        if (FD_ISSET(listen_sock, &tmp_read_set)) {
            if ((new_sock = accept(listen_sock, (struct sockaddr *) &addr, (socklen_t *) &addrlen)) < 0)
                exitFail("Accept error\n");
            if (fcntl(new_sock, F_SETFL, O_NONBLOCK) < 0)
                exitFail("Fcntl error\n");
            clients.insert(clients.begin(), new_sock);
        }

        for (std::vector<int>::iterator it = clients.begin(); it != clients.end(); it++) {
            if (FD_ISSET(*it, &tmp_read_set)) {
                read_res = recv(*it, buf, BUF_SIZE, 0);
                if (read_res < 0)
                    exitFail("Recv error\n");
                if (read_res == 0) {
                    close(*it);
                    FD_CLR(*it, &read_set);
                    clients.erase(it);
                    continue;
                }
                buf[read_res] = 0;

                std::cout << "BUF CONTENT:\n" << buf << "\n";

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
                    exitFail("Send error\n");
            }
        }
    }
    return 0;
}
