#include "ResponseHandler.hpp"

const ResponseHandler::T ResponseHandler::_statusPairs[] = {
	{200, "OK"},
	{201, "Created"},
	{400, "Bad Request"},
	{404, "Not Found"},
	{403, "Forbidden"},
	{405, "Method Not Allowed"},
	{411, "Length Required"},
	{413, "Payload Too Large"},//close connection!!!
	{505, "HTTP Version Not Supported"},
	{501, "Not Implemented"},
	{507, "Insufficient Storage"},
	{1000, "Welcome page"}

//414 URI Too Long
//415 Unsupported Media Type
};

const std::map<int, std::string>  ResponseHandler::_status_codes(_statusPairs, _statusPairs + 9);

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

ResponseHandler::ResponseHandler(): _conf(0), _status_code(0),/* _data(0), _data_size(0),*/ _last_modified(0) {}

ResponseHandler::ResponseHandler( const ResponseHandler & src ): _conf(src._conf),\
					_status_code(src._status_code), _data(src._data), _last_modified(src._last_modified),\
					_path(src._path), _location(src._location), _root(src._root), _methods(src._methods) {
/*	if (src._data) {
		_data = new char[src._data_size];
		for (ssize_t i = 0; i < _data_size; ++i)
			reinterpret_cast<unsigned char *>(_data)[i] = reinterpret_cast<unsigned char *>(src._data)[i];
	}
	else
		_data = 0;*/

}

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
	if (_root == NOT_FOUND) {
		if (_path.size())
			_status_code = 404;
		else
			_status_code = 1000;
		return generateErrorPage();
	}
	if (_header.find("GET") != _header.end())
		return answerToGET();
	else if (_header.find("POST") != _header.end())
		return answerToPOST();
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
		return generateErrorPage();
	} else if (!add_index_if_needed(resource_path)) {
		return generateErrorPage();
	} else if (access(resource_path.c_str(), F_OK)) {
		if (_path == "Forbidden")
			_status_code = 403;
		else
			_status_code = 404;
		return generateErrorPage();
	} else if (access(resource_path.c_str(), R_OK)) {
		_status_code = 403;
		return generateErrorPage();
	}

	read_binary_file(resource_path);
	_status_code = 200;
	createHTTPheader(setMimeType(resource_path), NOT_FOUND, true);

	return RequestHandler::READY_TO_ASWER;
}

void ResponseHandler::answerToDELETE() {}

int ResponseHandler::generateErrorPage() {
	generateHTML();
	createHTTPheader("text/html", NOT_FOUND, false);

	return RequestHandler::READY_TO_ASWER;
}

/* ************************************************************************** */