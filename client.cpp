#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

int exitFail(std::string s) {
    std::cerr << s;
    exit(EXIT_FAILURE);
}


int main(int argc, char const* argv[])
{
    struct sockaddr_in addr;
    int sock, client_sock;
    char buf[1024];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exitFail("Socket error\n");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) < 0)
        exitFail("Inet_pton error\n");

    if ((client_sock = connect(sock, (struct sockaddr *) &addr, sizeof(addr))) < 0)
        exitFail("Connetion error\n");

    if (send(sock, "hello from client\n", strlen("hello from client\n"), 0) < 0)
        exitFail("Send error\n");

    if (read(sock, buf, 1024) < 0)
        exitFail("Read error\n");

    std::cout << buf << "\n";

    close(client_sock);

    return 0;
}
