//
// Created by Nikita Madorsky on 25.11.2022.
//

#include "../headers/webserv.h"

int Server::getMaxSock() {
    if (client_sockets.empty())
        return (--listen_socks.end())->first;
    else
        return (--client_sockets.end())->first;
}

Server::Server(std::vector<Parser> conf): conf(conf) {
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    client_sockets.clear();
    codeResponseInit();
}

Server::~Server() {}

void Server::codeResponseInit() {
    code_response.insert(std::pair<int, std::string>(200, "200 OK"));
    code_response.insert(std::pair<int, std::string>(301, "301 Moved Permanently"));
    code_response.insert(std::pair<int, std::string>(401, "401 Unauthorized"));
    code_response.insert(std::pair<int, std::string>(404, "404 Not Found"));
    code_response.insert(std::pair<int, std::string>(405, "405 Method Not Allowed"));
}

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

int Server::acceptNewConnection(fd_set *set, struct sockaddr_in *addr) {
    int new_sock = 0;
    socklen_t addrLen = sizeof(addr); //TODO delete line or return from func
    for (std::map<int, int>::iterator i = listen_socks.begin(); i != listen_socks.end(); ++i) {
        if (FD_ISSET(i->first, set)) {
            if ((new_sock = accept(i->first, (struct sockaddr *) addr, &addrLen)) < 0)
                return -1;
            if (new_sock > FD_SETSIZE)
                return close(new_sock);
            if (fcntl(new_sock, F_SETFL, O_NONBLOCK) < 0)
                return -1;
            fd_info fd_state;
            fd_state.readyToWriting = false;
            fd_state.mimeType = "text/html";
            fd_state.belongPort = i->second;
            client_sockets.insert(client_sockets.begin(), std::pair<int, fd_info>(new_sock, fd_state));
        }
    }
    return new_sock;
}

int Server::renderErrorPage(std::map<int, fd_info>::iterator it, int status) {
    std::string statusStr = itos(status);
    std::ifstream f("static/" + statusStr + ".html");
    if (!f.is_open()) {
        printErr("cant open " + statusStr + ".html");
        return 1;
    }
    std::string s;
    while (std::getline(f, s))
        it->second.response += s + "\n";
    f.close();
    it->second.status = status;
    it->second.readyToWriting = true;
    FD_SET(it->first, &write_set);
    return 0;
}

Parser *Server::getConfByPort(int port) {
    for (std::vector<Parser>::iterator i = conf.begin(); i != conf.end(); ++i) {
        int tmpPort = atoi((*i).getServfield("listen").c_str());
        if (tmpPort == port)
            return new Parser(*i);
    }
    return NULL;
}

bool Server::isAllowMethod(std::string method, std::string allowed_methods) {
    if (allowed_methods == NOT_FOUND)
        return true;
    std::vector<std::string> methods = split(allowed_methods, " ");
    std::vector<std::string>::iterator it = std::find(methods.begin(), methods.end(), method);
    return it != methods.end() ? true : false;
}

void Server::setMimeType(std::map<int, fd_info>::iterator it, std::string path) {
    std::string extension = split(path, ".").back();
    if (extension == "css")
        it->second.mimeType = "text/css";
    else if (extension == "png")
        it->second.mimeType = "image/png";
    else if (extension == "jpeg" || extension == "jpg")
        it->second.mimeType = "image/jpeg";
    else
        it->second.mimeType = "text/html";
}

int Server::redirect(std::string path, std::map<int, fd_info>::iterator it) {
    it->second.status = 301;
    it->second.redirectTo = path;
    FD_SET(it->first, &write_set);
    it->second.readyToWriting = true;
    return 0;
}

int Server::removeClient(std::map<int, fd_info>::iterator *it) {
    FD_CLR((*it)->first, &read_set);
    FD_CLR((*it)->first, &write_set);
    close((*it)->first);
    client_sockets.erase((*it)++->first);
    return 1;
}

std::string Server::getLocURL(std::string path, Parser *curConf) {
    std::string res;
    if (curConf->getLocfield(path, "root") != NOT_FOUND)
        return path;
    std::vector<std::string> v = split(path, "/");
    if (curConf->getLocfield("/" + v[1], "root") != NOT_FOUND)
        return "/" + v[1];
    return curConf->getServfield("root");
}

void Server::preparePathToOpen(std::string &path, std::string locURL, std::string rootDir) {
    if (path == locURL + "/" || path == "/")
        path += "index.html";
    path = rootDir + rtrim(path, "/");
    replaceOn(path, "//", "/");
}

int Server::getRequest(std::string path, std::map<int, fd_info>::iterator it) {
    std::ifstream file(path.c_str()); // fix for ubuntu
    std::string s;
    if (file.is_open()) {
        while (std::getline(file, s))
            it->second.response += s + "\n";
        file.close();
        it->second.status = 200;
        it->second.readyToWriting = true;
        FD_SET(it->first, &write_set);
        return 0;
    }
    return renderErrorPage(it, 404);
}

int Server::recieve(std::map<int, fd_info>::iterator *it, char **buf) {
    ssize_t recv_res = recv((*it)->first, *buf, BUF_SZ, 0);
    if (recv_res < 0) {
        printErr("strerror from recieve: " + std::string(strerror(errno))); //TODO delete errno
        return removeClient(it);
    }
    if (recv_res == 0) {
        printWar("Client go away");
        return removeClient(it);
    } //TODO join if res <= 0 then removeClient
    *(*buf + recv_res) = 0;

    std::string fline = split(std::string(*buf), "\n").front();
    std::vector<std::string> arr = split(fline, " ");
    std::string path = arr[1];
    std::string requestMethod = arr[0];

    Parser *curConf = getConfByPort((*it)->second.belongPort);
    if (curConf == NULL)
        return 1;

    std::string locURL = getLocURL(path, curConf);
    std::string rootDir = curConf->getLocfield(locURL, "root");
    std::string methods;
    if (rootDir == NOT_FOUND) {
        rootDir = curConf->getServfield("root");
        methods = curConf->getServfield("methods");
    } else {
        if (path == locURL) {
            delete curConf;
            return redirect(path + '/', *it);
        }
        methods = curConf->getLocfield(path.substr(0, path.length() - 1), "methods");
    }
    delete curConf;

    if (isAllowMethod(arr[0], methods) == false)
        return renderErrorPage(*it, 405);

    preparePathToOpen(path, locURL, rootDir);
    setMimeType(*it, path);

    if (requestMethod == "GET")
        return getRequest(path, *it);
    return 0;
}

void Server::formResponse(std::map<int, fd_info>::iterator it) {
    it->second.headers = "HTTP/1.1 " + code_response.find(it->second.status)->second + "\r\n";
    it->second.headers += "Version: HTTP/1.1\r\n";
    it->second.headers += "Content-Type: " + it->second.mimeType + "; charset=utf-8\r\n";
    if (it->second.status == 301)
        it->second.headers += "Location: " + it->second.redirectTo + "\r\n";
    it->second.headers += "Content-Length: " + itos(it->second.response.length());
    it->second.headers += "\r\n\r\n";
}

int Server::sendResponse(std::map<int, fd_info>::iterator *it) {
    std::stringstream response_body;
    std::stringstream response;

    formResponse(*it);
    response << (*it)->second.headers << (*it)->second.response;

    if (send((*it)->first, response.str().c_str(), response.str().length(), 0) <= 0)
        return removeClient(it);
    FD_CLR((*it)->first, &write_set);
    (*it)->second.readyToWriting = false;
    (*it)->second.response.clear();
    return 0;
}

void Server::mainLoop() {
    char *buf = new char[BUF_SZ + 1];
    fd_set tmp_read_set, tmp_write_set;
    struct sockaddr_in clientAddr = {};
    int sock, port;

    for (std::vector<Parser>::iterator i = conf.begin(); i != conf.end(); i++) {
        port = atoi(i->getServfield("listen").c_str());
        if ((sock = getListenSocket(getAddr(port))) != -1) {
            listen_socks.insert(std::pair<int, int>(sock, port));
            FD_SET(sock, &read_set);
        }
    }

    while (true) {
        for (std::map<int, fd_info>::iterator i = client_sockets.begin(); i != client_sockets.end(); i++)
            FD_SET(i->first, &read_set);
        tmp_read_set = read_set;
        tmp_write_set = write_set;
        if (select(getMaxSock() + 1, &tmp_read_set, &tmp_write_set, NULL, NULL) <= 0)
            continue;
        if (acceptNewConnection(&tmp_read_set, &clientAddr) < 0)
            continue;

        std::map<int, fd_info>::iterator it = client_sockets.begin();
        std::map<int, fd_info>::iterator end = client_sockets.end();
        while (it != end) {
            if (FD_ISSET(it->first, &tmp_read_set))
                if (recieve(&it, &buf))
                    continue;
            if (it->second.readyToWriting && FD_ISSET(it->first, &tmp_write_set))
                if (sendResponse(&it))
                    continue;
            it++;
        }
    }
}
