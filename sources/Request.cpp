//
// Created by Nikita Madorsky on 13.12.2022.
//

#include "../headers/webserv.h"

Request::Request(std::map<int, fd_info>::iterator it, char *buf, Parser *conf, fd_set *write_set):
it(it), buf(buf), conf(conf), write_set(write_set) {}

Request::~Request() {
    delete conf;
}

bool Request::isBadRequest() {
    if (strlen(buf) < 7) {
        printErr("Bad request!");
        return true;
    }
    return false;
} //TODO need to fix

int Request::badRequest() {
    it->second.status = 400;
    FD_SET(it->first, write_set);
    it->second.response = "";
    *buf = 0;
    it->second.readyToWriting = true;
    return 0;
}

int Request::parse() {
    if (isBadRequest())
        return badRequest();
    std::string fline = split(buf, "\n").front();
    std::vector<std::string> arr = split(fline, " ");
    if (arr.empty())
        return 1;
    path = arr[1];
    method = arr[0];
    std::string requestMethod = arr[0];

    if (conf == NULL)
        return 1;
    return 0;
}

std::string Request::getLocURL() {
    std::string res;
    if (conf->getLocfield(path, "root") != NOT_FOUND)
        return path;
    std::vector<std::string> v = split(path, "/");
    if (conf->getLocfield("/" + v[1], "root") != NOT_FOUND)
        return "/" + v[1];
    return conf->getServfield("root");
}

int Request::redirect() {
    it->second.status = 301;
    it->second.redirectTo = path + "/";
    FD_SET(it->first, write_set);
    it->second.readyToWriting = true;
    return 0;
}

bool Request::isAllowMethod(std::string allowed_methods) {
    if (allowed_methods == NOT_FOUND)
        return true;
    std::vector<std::string> methods = split(allowed_methods, " ");
    std::vector<std::string>::iterator it = std::find(methods.begin(), methods.end(), method);
    return it != methods.end() ? true : false;
}

void Request::preparePathToOpen(std::string locURL, std::string rootDir) {
    if (path == locURL + "/" || path == "/")
        path += "index.html";
    path = rootDir + rtrim(path, "/");
    replaceOn(path, "//", "/");
}

void Request::setMimeType() {
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

int Request::renderErrorPage(int status) {
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
    FD_SET(it->first, write_set);
    return 0;
}

int Request::getRequest() {
    std::ifstream file(path.c_str()); // fix for ubuntu
    std::string s;
    if (file.is_open()) {
        while (std::getline(file, s))
            it->second.response += s + "\n";
        file.close();
        it->second.status = 200;
        it->second.readyToWriting = true;
        FD_SET(it->first, write_set);
        return 0;
    }
    return renderErrorPage(404);
}

int Request::mainLogic() {
    if (it->second.status == 400)
        return 0;
    std::string locURL = getLocURL();
    std::string rootDir = conf->getLocfield(locURL, "root");
    std::string methods;
    if (rootDir == NOT_FOUND) {
        rootDir = conf->getServfield("root");
        methods = conf->getServfield("methods");
    } else {
        if (path == locURL)
            return redirect();
        methods = conf->getLocfield(path.substr(0, path.length() - 1), "methods");
    }

    if (isAllowMethod(methods) == false)
        return renderErrorPage(405);

    preparePathToOpen(locURL, rootDir);
    setMimeType();

    if (method == "GET")
        return getRequest();

    return 0;
}
