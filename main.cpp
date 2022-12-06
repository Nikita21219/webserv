#include "Server.hpp"

int main() {

    Server serv = Server("127.0.0.1", 8080);
    serv.mainLoop();
    return 0;
}
