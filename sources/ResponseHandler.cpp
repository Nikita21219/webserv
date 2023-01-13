#include "ResponseHandler.hpp"

const ResponseHandler::T ResponseHandler::_statusPairs[] = {
	{200, "OK"},
	{404, "Not Found"},
	{403, "Forbidden"},
	{405, "Method Not Allowed"},
	{411, "Length Required"},
	{413, "Payload Too Large"},//close connection!!!
	{505, "HTTP Version Not Supported"},
	{501, "Not Implemented"},
	{1000, "Welcome page"}
};

const std::map<int, std::string>  ResponseHandler::_status_codes(_statusPairs, _statusPairs + 9);

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

ResponseHandler::ResponseHandler(char **env): _conf(0), _status_code(0), _data(0), _data_size(0), _last_modified(0), _env(env) {}

ResponseHandler::ResponseHandler( const ResponseHandler & src ): _conf(src._conf),\
					_status_code(src._status_code), _data_size(src._data_size), _last_modified(src._last_modified),\
					_path(src._path), _location(src._location), _root(src._root), _methods(src._methods), _env(src._env) {
	if (src._data) {
		_data = new char[src._data_size];
		for (ssize_t i = 0; i < _data_size; ++i)
			reinterpret_cast<unsigned char *>(_data)[i] = reinterpret_cast<unsigned char *>(src._data)[i];
	}
	else
		_data = 0;
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

ResponseHandler::~ResponseHandler() {
	if (_data)
		delete[] reinterpret_cast<unsigned char *>(_data);
}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/

ResponseHandler &				ResponseHandler::operator=( ResponseHandler const & rhs ) {
	(void)rhs;
	//if ( this != &rhs )
	//{
		//this->_value = rhs.getValue();
	//}
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
        generateErrorPage();
	if (_root == NOT_FOUND) {
        if (_path.size())
            _status_code = 404;
		else
			_status_code = 1000;
        generateErrorPage();
	}
    if (_header.find("GET") != _header.end())
		return answerToGET();
	else if (_header.find("POST") != _header.end())
		answerToPOST();
	else if (_header.find("DELETE") != _header.end())
		answerToDELETE();
	return RequestHandler::ERROR_IN_REQUEST;
}

void ResponseHandler::sendResponseToClient(int fd) {

	if (send(fd, _response_data.data(), _response_data.size(), 0) <= 0)
		throw 1;

	_header.clear();
	_response_data.clear();
	_status_code = 0;
}

void ResponseHandler::extract_info(const Parser *conf) {
	_conf = conf;
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
		_path.clear();
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

int ResponseHandler::handleCgi() {
    std::string resultFile = _root + "/cgi_out";
    TempFile tmpFile = TempFile(resultFile/* + itos(it->first)*/);
    if (!tmpFile.isOpen()) {
        printErr("File not opened"); //TODO tmp line
        return 1;
    }
    Cgi cgi = Cgi(_root + _path, "/usr/local/bin/python3");
    if (cgi.launch(_env, tmpFile.getFd())) {
        _status_code = 500;
        return 1;
    }
    read_binary_file(resultFile);
    _status_code = 200;
    return 0;
}

int ResponseHandler::answerToGET() {
	std::string resource_path;
    if (_root.size() > 1 && _path.size() > 1)
        resource_path = _root + _path;
	else if (_root.size() > 1)
		resource_path = _root;
	else
		resource_path = _path;

	if (_methods != NOT_FOUND && _methods.find("GET") == std::string::npos) {
        _status_code = 405;
        generateErrorPage();
	} else if (!add_index_if_needed(resource_path)) {
        generateErrorPage();
	} else if (access(resource_path.c_str(), F_OK)) {
        if (_path == "Forbidden")
			_status_code = 403;
		else
            _status_code = 404;
		generateErrorPage();
	} else if (access(resource_path.c_str(), R_OK)) {
		_status_code = 403;
		generateErrorPage();
    }

    if (resource_path.find("cgi") != std::string::npos) {
        handleCgi();
    } else {
        read_binary_file(resource_path);
        _status_code = 200;
    }
	createHTTPheader(setMimeType(resource_path), true);

	return RequestHandler::READY_TO_ASWER;
}

void ResponseHandler::answerToPOST() {}

void ResponseHandler::answerToDELETE() {}

void ResponseHandler::generateErrorPage() {
	generateHTML();
	createHTTPheader("text/html", false);

	throw std::runtime_error("error page created!");
}

/* ************************************************************************** */