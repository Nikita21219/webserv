#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int exitFail(std::string s) {
    std::cerr << s;
    exit(EXIT_FAILURE);
}

int main() {
    int listen_sock, new_sock, read_res;
    int opt = 1;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    char buf[1024];

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exitFail("Socket error\n");

    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        exitFail("Setsockopt SO_REUSEADDR error\n");
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
        exitFail("Setsockopt SO_REUSEPORT error\n");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(listen_sock, (struct sockaddr*) &addr, sizeof(addr)))
        exitFail("Bind error\n");

    if (listen(listen_sock, 3))
        exitFail("Listen error\n");

    if ((new_sock = accept(listen_sock, (struct sockaddr*) &addr, (socklen_t*)&addrlen)) < 0)
        exitFail("Accept error\n");

    read_res = read(new_sock, buf, 1024);
    std::cout << buf << "\n";

    send(new_sock, "hello from server\n", strlen("hello from server\n"), 0);

    close(new_sock);
    shutdown(listen_sock, SHUT_RDWR);
    return 0;
}
