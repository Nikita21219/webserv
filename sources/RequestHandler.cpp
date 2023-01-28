#include "RequestHandler.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

RequestHandler::RequestHandler(std::vector<Parser> const &conf, int sock, unsigned long id, request_status s, char **env):
_conf(conf), _serv_id(id), _client_socket(sock), _status(s), _chunk_size(0) {
    _answer = new ResponseHandler(env);
}

RequestHandler::RequestHandler( const RequestHandler & src ): _conf(src._conf),\
				_serv_id(src._serv_id), _client_socket(src._client_socket), _status(src._status), _chunk_size(src._chunk_size) {
    _answer = new ResponseHandler(*src._answer);
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

RequestHandler::~RequestHandler() {
    delete _answer;
}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/

RequestHandler &				RequestHandler::operator=( RequestHandler const & rhs ) {
    if ( this != &rhs ) {
        _client_socket = rhs._client_socket;
        _serv_id = rhs._serv_id;
        _status = rhs._status;
        _chunk_size = rhs._chunk_size;
        _answer->setConf() = rhs._answer->setConf();
        _answer->setData() = rhs._answer->setData();
        _answer->setHeader() = rhs._answer->setHeader();
        _answer->setLast_modified() = rhs._answer->setLast_modified();
        _answer->setLocation() = rhs._answer->setLocation();
        _answer->setMethods() = rhs._answer->setMethods();
        _answer->setPath() = rhs._answer->setPath();
        _answer->setRoot() = rhs._answer->setRoot();
        _answer->setStatus_code() = rhs._answer->setStatus_code();
        _answer->setResponse_data() = rhs._answer->setResponse_data();
    }
    return *this;
}

std::ostream &			operator<<( std::ostream & o, RequestHandler const & i ) {
    o << "_client_socket " << i.get_sock();
    return o;
}

/*
** --------------------------------- METHODS ----------------------------------
*/

void RequestHandler::serve_client(fd_set &write_set) {
    if (_status == READY_TO_ASWER) {
        _answer->setHeader().clear();
        _answer->setResponse_data().clear();
        _answer->setLocation().clear();
        _answer->setStatus_code() = 0;
        _status = NEW;
    }
    if (_status == NEW)
        new_reading();
    else if (_status == MUST_KEEP_READING)
        continue_reading();
    if (_status == READY_TO_ASWER || _status == ERROR_IN_REQUEST)
        FD_SET(_client_socket, &write_set);
}

void RequestHandler::new_reading() {
    ssize_t end;
    ssize_t header_size = 0;
    ssize_t size = recv(_client_socket, _buf, BUF_SZ, 0);
    if (size <= 0)
        throw 1;
    _buf[size] = 0;
    std::string str = reinterpret_cast<char *>(_buf);
    if (str.substr(0, str.find('\n')).find("HTTP/1.1") == std::string::npos) {
        _answer->setStatus_code() = 505;
        _status = static_cast<request_status>(_answer->prepareAnswer());
        return;
    }
    while (1) {
        end = str.find('\n') + 1;
        header_size += end;
        std::string tmp = str.substr(0, end);
        if (tmp[0] == '\n' || tmp[0] == '\r' || !tmp.size())
            break ;
        parse_string(tmp);
        str.erase(0, end);
    }
    if (_answer->setHeader().find("Host") != _answer->setHeader().end())
        _answer->extract_info(&_conf[select_serv(_answer->setHeader().find("Host")->second)], _client_socket);
    else
        _answer->extract_info(&_conf[_serv_id], _client_socket);
/*    if (_answer->setStatus_code()) {
        _status = static_cast<request_status>(_answer->prepareAnswer());
        return;
    }*/
    if (_answer->setHeader().find("GET") == _answer->setHeader().end())
        download_data(size, header_size);
    else if (_answer->setHeader().find("Content-Length") != _answer->setHeader().end() &&\
			atoi(_answer->setHeader().find("Content-Length")->second.c_str())) {
        _answer->setStatus_code() = 413;
    }
    if (_status != MUST_KEEP_READING)
        _status = static_cast<request_status>(_answer->prepareAnswer());
}

void RequestHandler::download_data(ssize_t size, ssize_t header_size) {
    ssize_t max_body = -1;

    std::string m;
    if (_answer->setHeader().find("POST") != _answer->setHeader().end())
        m = "POST";
    else
        m = "DELETE";
    if (_answer->setMethods().find(m) == std::string::npos)
        _answer->setStatus_code() = 405;
    if (_answer->setHeader().find("Transfer-Encoding") != _answer->setHeader().end() &&\
        _answer->setHeader().find("Transfer-Encoding")->second.find("chunked") != std::string::npos) {
        chanked_handler(size, header_size);
        return;
    } else if (_answer->setHeader().find("Content-Length") == _answer->setHeader().end()) {
        if (m == "DELETE")
            return;
        _answer->setStatus_code() = 411;
        return;
    } else if (_answer->setLocation() != NOT_FOUND && _answer->setConf()->getLocfield(_answer->setLocation(), "max_body_size") != NOT_FOUND) {
        max_body = atoi(_answer->setConf()->getLocfield(_answer->setLocation(), "max_body_size").c_str());
    } else if (_answer->setConf()->getServfield("max_body_size") != NOT_FOUND)
        max_body = atoi(_answer->setConf()->getServfield("max_body_size").c_str());
    ssize_t content_lenght = atoi(_answer->setHeader().find("Content-Length")->second.c_str());
    if (max_body != -1 && content_lenght > max_body) {
        _answer->setStatus_code() = 413;
        return;
    }
    if (!multipart_parser(header_size))
        _answer->setStatus_code() = 400;

    _answer->setData().insert(_answer->setData().begin(), _buf + header_size, _buf + size);
    content_lenght = atoi(_answer->setHeader().find("Content-Length")->second.c_str());
    if (static_cast<ssize_t>(_answer->setData().size()) < content_lenght)
        _status = MUST_KEEP_READING;
}

bool RequestHandler::multipart_parser(ssize_t &header_size) {
    if (_answer->setHeader().find("Content-Type") == _answer->setHeader().end() ||\
		_answer->setHeader().find("Content-Type")->second.find("multipart/form-data") == std::string::npos)
        return true;
    std::string boundary;
    boundary = _answer->setHeader().find("Content-Type")->second;
    boundary = boundary.substr(boundary.find("boundary") + 9, boundary.size() - boundary.find("boundary") + 9);
    boundary.erase(std::remove(boundary.begin(), boundary.end(), '-'), boundary.end());
    _answer->setHeader().erase("Content-Type");

    std::stringstream stream(reinterpret_cast<char *>(_buf + header_size));
    std::string tmp;
    std::getline(stream, tmp);
    ssize_t multipart_size = tmp.size() + 1;
    tmp.erase(std::remove(tmp.begin(), tmp.end(), '-'), tmp.end());
    tmp.erase(std::remove(tmp.begin(), tmp.end(), '\r'), tmp.end());
    if (boundary != tmp) {
        return false;
    }

    std::getline(stream, tmp);
    multipart_size += tmp.size() + 1;
    while (tmp[0] != '\r' && tmp[0] != '\n') {
        tmp += '\n';
        parse_string(tmp);
        std::getline(stream, tmp);
        multipart_size += tmp.size() + 1;
    }
    std::stringstream t;
    t << atoi(_answer->setHeader().at("Content-Length").c_str()) - multipart_size;
    _answer->setHeader().at("Content-Length") = t.str();
    header_size += multipart_size;
    return true;
}

void RequestHandler::continue_reading() {
    ssize_t size = recv(_client_socket, _buf, BUF_SZ, 0);
    if (size <= 0)
        throw 1;
    if (_answer->setHeader().find("Transfer-Encoding") != _answer->setHeader().end() &&\
            _answer->setHeader().find("Transfer-Encoding")->second.find("chunked") != std::string::npos) {
        chanked_handler(size, 0);
        return;
    }
    ssize_t content_lenght = atoi(_answer->setHeader().find("Content-Length")->second.c_str());
    if (static_cast<ssize_t>(_answer->setData().size() + size) <= content_lenght)
        _answer->setData().insert(_answer->setData().end(), _buf, _buf + size);
    else
        _answer->setData().insert(_answer->setData().end(), _buf, _buf + (content_lenght - _answer->setData().size()));

    if (static_cast<ssize_t>(_answer->setData().size()) == content_lenght)
        _status = static_cast<request_status>(_answer->prepareAnswer());
}

void    RequestHandler::chanked_handler(ssize_t size, ssize_t start) {
    
    if (_chunk_size) {
        int cur_read = _chunk_size - _chunk.size();
        if (size - start >= cur_read) {
            _chunk.insert(_chunk.end(), _buf + start, _buf + start + cur_read);
            _answer->setData().insert(_answer->setData().end(), _chunk.begin(), _chunk.end());
            _chunk.clear();
            _chunk_size = 0;
            start = start + cur_read;
        } else {
            _chunk.insert(_chunk.end(), _buf + start, _buf + start + size);
            return;
        }
    }

    std::string tmp(reinterpret_cast<char *>(_buf + start), size - start);
    std::stringstream stream(tmp);
    tmp.clear();
    std::getline(stream, tmp);
    while (tmp.size() >= 1 && tmp[0] == '\r')
        std::getline(stream, tmp);
    if (!tmp.size())
        return;
    stream.str(std::string());
    stream << std::hex << tmp;
    stream >> _chunk_size;
    if (!_chunk_size) {
        if (size - start < 5) {
            _chunk_size = 0;
            return;
        }
        _status = static_cast<request_status>(_answer->prepareAnswer());
        return;
    }

    int chunk_pos = start + tmp.size() + 3;
    if (size - chunk_pos >= _chunk_size) {
        _answer->setData().insert(_answer->setData().end(), _buf + chunk_pos, _buf + chunk_pos + _chunk_size);
        _chunk_size = 0;
        chanked_handler(size, chunk_pos + _chunk_size);
        return;
    } else {
        _chunk.insert(_chunk.begin(), _buf + chunk_pos, _buf + size);
        _status = MUST_KEEP_READING;
    }
}

void RequestHandler::sendResponse(fd_set &write_set) {
    _answer->sendResponseToClient(_client_socket);
    if (!_answer->setResponse_data().size()) {
        FD_CLR(_client_socket, &write_set);
        _status = NEW;
    }
}

void RequestHandler::parse_string(std::string str) {
    std::string key;
    std::string value;
    if (str.find(':') != std::string::npos)
        key = str.substr(0, str.find(':'));
    else {
        if (str.find("HTTP/1.1") == std::string::npos)
            return;
        key = str.substr(0, str.find(' '));
        str.erase(str.find("HTTP/1.1") - 1, str.find('\n'));
    }
    str.erase(0, str.find(' ') + 1);
    value = str.substr(0, str.find('\n'));
    if (value.find('\r') != std::string::npos)
        value = value.substr(0, value.find('\r'));
    _answer->setHeader().insert(_answer->setHeader().end(), std::pair<std::string, std::string>(key, value));
}

int RequestHandler::select_serv(std::string const str) {
    std::vector<std::string> arr = split(str, ":");
    for (unsigned int i = 0; i < _conf.size(); ++i) {
        if (i == _serv_id) {
            if (_conf[i].getServfield("server_name") == arr[0])
                return i;
            else
                continue;
        }
        if (_conf[i].getServfield("host") == _conf[_serv_id].getServfield("host") &&\
				_conf[i].getServfield("port") == _conf[_serv_id].getServfield("port") &&\
				_conf[i].getServfield("server_name") == arr[0])
            return i;
    }
    return _serv_id;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

int RequestHandler::get_sock() const {
    return _client_socket;
}

RequestHandler::request_status& RequestHandler::setStatus() {
    return _status;
}

/* ************************************************************************** */