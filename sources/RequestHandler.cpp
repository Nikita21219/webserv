#include "RequestHandler.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

RequestHandler::RequestHandler(std::vector<Parser> const &conf, int sock, unsigned long id, request_status s): _conf(conf),\
				_serv_id(id), _client_socket(sock), _status(s) {
	_answer = new ResponseHandler();
}

RequestHandler::RequestHandler( const RequestHandler & src ): _conf(src._conf),\
				_serv_id(src._serv_id), _client_socket(src._client_socket), _status(src._status) {
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
		_answer = new ResponseHandler(*rhs._answer);
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
	if (_status == NEW)
		new_reading();
	else if (_status == MUST_KEEP_READING)
		continue_reading();
	if (_status == READY_TO_ASWER)
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
		_answer->setStatus_code() = 505;//answer: 505 http not suported!
		_status = static_cast<request_status>(_answer->prepareAnswer());//is it right?
		return;
	}

//	std::cout << str;

	while (1) {
		end = str.find('\n') + 1;
		header_size += end;
		std::string tmp = str.substr(0, end);
		if (tmp[0] == '\n' || tmp[0] == '\r')
			break ;
		parse_string(tmp);
		str.erase(0, end);
	}
	if (_answer->getHeader().find("Host") != _answer->getHeader().end())
		_answer->extract_info(&_conf[select_serv(_answer->getHeader().find("Host")->second)]);
	else
		_answer->extract_info(&_conf[_serv_id]);
	if (_answer->getHeader().find("POST") != _answer->getHeader().end())
		download_data(size, header_size);
	if (_status != MUST_KEEP_READING)
		_status = static_cast<request_status>(_answer->prepareAnswer());
}

void RequestHandler::download_data(ssize_t size, ssize_t header_size) {
	ssize_t max_body = -1;
	if (_answer->setStatus_code())
		return;
	else if (_answer->getMethods().find("POST") == std::string::npos) {
		_answer->setStatus_code() = 405;
		return;
	} else if (_answer->getHeader().find("Content-Length") == _answer->getHeader().end()) {
		_answer->setStatus_code() = 411;
		return;
	} else if (_answer->getLocation() != NOT_FOUND && _answer->getConf()->getLocfield(_answer->getLocation(), "max_body_size") != NOT_FOUND) {
		max_body = atoi(_answer->getConf()->getLocfield(_answer->getLocation(), "max_body_size").c_str());
	} else if (_answer->getConf()->getServfield("max_body_size") != NOT_FOUND)
		max_body = atoi(_answer->getConf()->getServfield("max_body_size").c_str());
	ssize_t content_lenght = atoi(_answer->setHeader().find("Content-Length")->second.c_str());
	if (max_body != -1 && content_lenght > max_body) {
		_answer->setStatus_code() = 413;
		return;
	}
	if (!multipart_parser(header_size)) {
		_answer->setStatus_code() = 400;
		return;
	}

	_answer->setData().insert(_answer->setData().begin(), _buf + header_size, _buf + size);
	if (static_cast<ssize_t>(_answer->setData().size()) < content_lenght)
		_status = MUST_KEEP_READING;
}

bool RequestHandler::multipart_parser(ssize_t &header_size) {
	if (_answer->getHeader().find("Content-Type") == _answer->getHeader().end() ||\
		_answer->getHeader().find("Content-Type")->second.find("multipart/form-data") == std::string::npos)
		return true;
	std::string boundary;
	boundary = _answer->getHeader().find("Content-Type")->second;
	boundary = boundary.substr(boundary.find("boundary") + 9, boundary.size() - boundary.find("boundary") + 9);
	boundary.erase(std::remove(boundary.begin(), boundary.end(), '-'), boundary.end());
	_answer->setHeader().erase("Content-Type");

	std::stringstream stream(reinterpret_cast<char *>(_buf + header_size));
	std::string tmp;
	std::getline(stream, tmp);
	ssize_t multipart_size = tmp.size() + 1;
	tmp.erase(std::remove(tmp.begin(), tmp.end(), '-'), tmp.end());
	tmp.erase(std::remove(tmp.begin(), tmp.end(), '\r'), tmp.end());
	if (boundary != tmp)
		return false;

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
	ssize_t content_lenght = atoi(_answer->setHeader().find("Content-Length")->second.c_str());
	if (static_cast<ssize_t>(_answer->setData().size() + size) <= content_lenght)
		_answer->setData().insert(_answer->setData().end(), _buf, _buf + size);
	else
		_answer->setData().insert(_answer->setData().end(), _buf, _buf + (content_lenght - _answer->setData().size()));

	if (static_cast<ssize_t>(_answer->setData().size()) == content_lenght)
		_status = static_cast<request_status>(_answer->prepareAnswer());
}

void RequestHandler::sendResponse(fd_set &write_set) {
	_answer->sendResponseToClient(_client_socket);
	FD_CLR(_client_socket, &write_set);
	_status = NEW;
}

void RequestHandler::parse_string(std::string str) {
	std::string key;
	std::string value;
	if (str.find(':') != std::string::npos)
		key = str.substr(0, str.find(':'));
	else {
		key = str.substr(0, str.find(' '));
		str.erase(str.find("HTTP/1.1") - 1, str.find('\n'));
	}
	str.erase(0, str.find(' ') + 1);
	value = str.substr(0, str.find('\n'));
	if (value.find('\r') != std::string::npos)//check this
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