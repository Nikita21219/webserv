#include "ResponseHandler.hpp"

const ResponseHandler::T ResponseHandler::_statusPairs[] = {
        {200, "OK"},
        {201, "Created"},
        {204, "No Content"},
        {301, "Moved Permanently"},
        {400, "Bad Request"},
        {404, "Not Found"},
        {403, "Forbidden"},
        {405, "Method Not Allowed"},
        {409, "Conflict"},
        {411, "Length Required"},
        {413, "Payload Too Large"},
        {414, "URI Too Long"},
        {415, "Unsupported Media Type"},
        {500, "Internal Server Error"},
        {505, "HTTP Version Not Supported"},
        {501, "Not Implemented"},
        {507, "Insufficient Storage"},
        {1000, "Welcome page"}
};

const std::map<int, std::string>  ResponseHandler::_status_codes(_statusPairs, _statusPairs + 18);

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

ResponseHandler::ResponseHandler(char **env): _conf(0), _status_code(0), _last_modified(0), _env(env) {}

ResponseHandler::ResponseHandler( const ResponseHandler & src ): _header(src._header), _conf(src._conf),\
					_status_code(src._status_code), _data(src._data), _last_modified(src._last_modified),\
					_path(src._path), _location(src._location), _root(src._root), _methods(src._methods), \
                    _env(src._env), _client_socket(src._client_socket) {}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

ResponseHandler::~ResponseHandler() {}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/

ResponseHandler &				ResponseHandler::operator=( ResponseHandler const & rhs ) {
    if ( this != &rhs ) {
        _status_code = rhs._status_code;
        _data = rhs._data;
        _last_modified = rhs._last_modified;
        _path = rhs._path;
        _location = rhs._location;
        _root = rhs._root;
        _methods = rhs._methods;
    }
    return *this;
}

std::ostream &			operator<<( std::ostream & o, ResponseHandler const & i ) {
    (void)i;
    //o << "Value = " << i.getValue();
    return o;
}

/*
** --------------------------------- METHODS ----------------------------------
*/

int ResponseHandler::prepareAnswer() {
    if (_status_code)
        return generateErrorPage();
    if (_root == NOT_FOUND && _conf->getLocfield(_location, "redirection") == NOT_FOUND) {
        if (_path.size() == 1 || _path == "/index.html")
            _status_code = 1000;
        else
            _status_code = 404;
        return generateErrorPage();
    }
    if (_header.find("GET") != _header.end())
        return answerToGET();
    else if (_header.find("POST") != _header.end())
        return answerToPOST();
    else
        return answerToDELETE();
}

void ResponseHandler::sendResponseToClient(int fd) {
    ssize_t size;
    if (_response_data.size() <= BUF_SZ)
        size = send(fd, _response_data.data(), _response_data.size(), 0);
    else
        size = send(fd, _response_data.data(), BUF_SZ, 0);
    if (size <= 0)
        throw 1;

    _response_data.erase(_response_data.begin(), _response_data.begin() + size);
    if (!_response_data.size()) {
        if (_status_code == 413) {
            sleep(1);
            throw 1;
        }
        _header.clear();
        _data.clear();
        _response_data.clear();
        _location.clear();
        _status_code = 0;
    }
}

void ResponseHandler::extract_info(const Parser *conf, int fd) {
    _conf = conf;
    _client_socket = fd;
    if (_status_code)
        return;
    if (_header.find("GET") != _header.end())
        _path = _header["GET"];
    else if (_header.find("POST") != _header.end())
        _path = _header["POST"];
    else if (_header.find("DELETE") != _header.end())
        _path = _header["DELETE"];
    else {
        _status_code = 501;
        return;
    }
    if (_path.size() > 2048) {
        _status_code = 414;
        return;
    }
    findLocation();
    if (_location != NOT_FOUND) {
        _root = _conf->getLocfield(_location, "root");
        _methods = _conf->getLocfield(_location, "methods");
    } else {
        _root = _conf->getServfield("root");
        _methods = _conf->getServfield("methods");
    }
}

void ResponseHandler::findLocation() {
    if (_conf->isThereLocation(_path)) {
        _location = _path;
        _path = '/';
        return;
    } else if (_path.size() > 1) {
        std::vector<std::string> arr = split(_path, "/");
        if (_conf->isThereLocation('/' + arr[1])) {
            _path = _path.substr(arr[1].size() + 1, _path.size() - arr[1].size() + 1);
            _location = '/' + arr[1];
            return;
        }
    }
    if (_conf->isThereLocation("/")) {
        _location = "/";
        return;
    }
    _location = NOT_FOUND;
}

int ResponseHandler::answerToGET() {
    if (!_path.size()) {
        _status_code = 400;
        return generateErrorPage();
    }
    std::string resource_path = getResourse_path();
    std::string redirectTo = NOT_FOUND;
    if ((redirectTo = _conf->getLocfield(_location, "redirection")) != NOT_FOUND) {
        _status_code = 301;
        createHTTPheader("text/html", redirectTo, true);
        return RequestHandler::READY_TO_ASWER;
    }
    if (_methods != NOT_FOUND && _methods.find("GET") == std::string::npos) {
        _status_code = 405;
        return generateErrorPage();
    } else if (!add_index_if_needed(resource_path)) {
        if (_response_data.size()) {
            _status_code = 200;
            createHTTPheader("text/html", NOT_FOUND, false);
            return RequestHandler::READY_TO_ASWER;
        } else
            return generateErrorPage();
    } else if (access(resource_path.c_str(), F_OK)) {
        _status_code = 404;
        return generateErrorPage();
    } else if (access(resource_path.c_str(), R_OK)) {
        _status_code = 403;
        return generateErrorPage();
    }

    std::string mime_type;
    int cgiResult = 0;
    if (_conf->getLocfield(_location, "bin_path") != NOT_FOUND) {
        if ((cgiResult = handleCgi()))
            return cgiResult;
    } else {
        mime_type = setMimeType(resource_path);
        if (mime_type == NOT_FOUND)
            return generateErrorPage();
        read_binary_file(resource_path);
        _status_code = 200;
    }
    createHTTPheader(mime_type, NOT_FOUND, true);

    return RequestHandler::READY_TO_ASWER;
}

int ResponseHandler::answerToDELETE() {
    std::string resource_path = getResourse_path();
    if (_methods.find("DELETE") == std::string::npos) {
        _status_code = 405;
        return generateErrorPage();
    }
    if(access(resource_path.c_str(), F_OK)) {
        _status_code = 404;
        return generateErrorPage();
    } else if (resource_path == _root) {
        _status_code = 403;
        return generateErrorPage();
    } else if (folderIsNotEmpty(resource_path)) {
        _status_code = 409;
        return generateErrorPage();
    }

    if (std::remove(resource_path.c_str())) {
        _status_code = 403;
        return generateErrorPage();
    }
    _status_code = 204;
    createHTTPheader("text/plain", NOT_FOUND, false);
    return RequestHandler::READY_TO_ASWER;
}

int ResponseHandler::handleCgi() {
    std::string resultFile = _root + "/cgi_out" + itos(_client_socket);
    TempFile tmpFile = TempFile(resultFile);
    if (!tmpFile.isOpen())
        return 1;
    Cgi cgi = Cgi(_root + _path, _conf->getLocfield(_location, "bin_path"));
    if (cgi.launch(_env, tmpFile.getFd())) {
        _status_code = 500;
        return generateErrorPage();
    }
    read_binary_file(resultFile);
    _status_code = 200;
    return 0;
}

int ResponseHandler::generateErrorPage() {
    if (_status_code == 1000)
        genereteWelcomePage();
    std::string e_page;
    if (_location != NOT_FOUND && !_location.empty())
        e_page = _conf->getLocfield(_location, "error_pages");
    else if (_location == NOT_FOUND)
        e_page = _conf->getServfield("error_pages");
    std::stringstream code;
    code << _status_code;
    if (e_page.find(code.str()) != std::string::npos) {
        e_page = _root + '/' + code.str() + ".html";
        if (access(e_page.c_str(), R_OK))
            generateHTML();
        else
            read_binary_file(e_page);
    }
    else
        generateHTML();

    createHTTPheader("text/html", NOT_FOUND, false);
    return RequestHandler::ERROR_IN_REQUEST;
}

/* ************************************************************************** */