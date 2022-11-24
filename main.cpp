#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

#define BUF_SIZE 1024

int exitFail(std::string s) {
    std::cerr << s;
    exit(EXIT_FAILURE);
}

int main() {
    int listen_sock, new_sock, read_res;
    int opt = 1;
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

    if (bind(listen_sock, (struct sockaddr*) &addr, sizeof(addr)))
        exitFail("Bind error\n");

    if (listen(listen_sock, SOMAXCONN))
        exitFail("Listen error\n");

    while (true) {
        if ((new_sock = accept(listen_sock, (struct sockaddr*) &addr, (socklen_t*)&addrlen)) < 0)
            exitFail("Accept error\n");

        read_res = recv(new_sock, buf, BUF_SIZE, 0);
        if (read_res == -1)
            exitFail("Recv error\n");
        if (read_res == 0)
            exitFail("Connection closed\n");
        buf[read_res] = 0;

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

        if (send(new_sock, response.str().c_str(), response.str().length(), 0) < 0)
            exitFail("Send error\n");

        close(new_sock);
    }
    shutdown(listen_sock, SHUT_RDWR);
    return 0;
}
