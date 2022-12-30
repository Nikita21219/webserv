#include "ResponseHandler.hpp"

/*
** --------------------------------- METHODS ----------------------------------
*/

bool ResponseHandler::add_index_if_needed(std::string &resource_path) {
	struct stat s;
	if(stat(resource_path.c_str(), &s) == 0) {
		if(s.st_mode & S_IFDIR) {
			_path = "Forbidden";
			if (_conf->getLocfield(_location, "index") != NOT_FOUND)
				resource_path = resource_path + '/' + _conf->getLocfield(_location, "index");
			else if (_location == NOT_FOUND && _conf->getServfield("index") != NOT_FOUND)
				resource_path = resource_path + '/' + _conf->getServfield("index");
			else
				resource_path = resource_path + "/index.html";
			if (!access(resource_path.c_str(), F_OK))
				stat(resource_path.c_str(), &s);
		}
	} else {
		_status_code = 404;
		return false;
	}
	_last_modified = s.st_mtim.tv_sec;
	return true;
}

void ResponseHandler::read_binary_file(const std::string filename) {
	std::ifstream file(filename.c_str(), std::ios::binary);
	file.unsetf(std::ios::skipws);

	std::streampos file_size;
	file.seekg(0, std::ios::end);
	file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	_response_data.reserve(file_size);
	_response_data.insert(_response_data.begin(),
				std::istream_iterator<unsigned char>(file),
				std::istream_iterator<unsigned char>());
	file.close();
}

/*
Fields in response header

Accept-Ranges: bytes
Location: http://example.com/about.html#contacts
Server: webserv
Version: HTTP/1.1
Content-Type: text/html
Date: Tue, 27 Dec 2022 14:27:42 GMT
Last-Modified: Tue, 06 Dec 2022 18:32:27 GMT
Content-Length: 2222
Connection: keep-alive

*/

void ResponseHandler::createHTTPheader(std::string mimeType, bool flag) {
	std::stringstream http_header;

	http_header << "HTTP/1.1 " << _status_code << ' ' << _status_codes.find(_status_code)->second << "\r\n";
	http_header << "Accept-Ranges: bytes\r\n";
	http_header << "Version: HTTP/1.1\r\n";
	http_header << "Content-Type: " << mimeType << "; charset=utf-8\r\n";
	http_header << "Date: " << getDate(std::time(0)) << "\r\n";
	http_header << "Content-Length: " << _response_data.size() << "\r\n";
	http_header << "Server: webserv\r\n";
	if (flag)
		http_header << "Last-Modified: " << getDate(_last_modified) << "\r\n";
	http_header << "Connection: keep-alive";
	http_header << "\r\n\r\n";

	std::string tmp = http_header.str();
	_response_data.insert(_response_data.begin(), tmp.c_str(), tmp.c_str() + tmp.size());
}

std::string ResponseHandler::getDate(std::time_t t) {
	std::stringstream os;
	std::ostream::sentry s(os);
	std::string format = "%c";

	if (s) {
		std::tm const* tm = std::localtime(&t);
		std::ostreambuf_iterator<char> out(os);

		std::use_facet<std::time_put<char> >(os.getloc())
			.put(out, os, os.fill(),
				 tm, &format[0], &format[0] + format.size());
	}
	os.width(0);

	std::vector<std::string> arr = split(os.str(), " ");
	for (std::vector<std::string>::iterator it = arr.begin(); it != arr.end(); ++it) {
		if (it->size() == 0)
			arr.erase(it);
	}
	return arr[0] + ", " + arr[2] + ' ' + arr[1] + ' ' + arr[4] + ' ' + arr[3] + " GMT";
}

std::string ResponseHandler::setMimeType(std::string &path) {
	std::string extension = split(path, ".").back();
	if (extension == "css")
		return "text/css";
	else if (extension == "png")
		return "image/png";
	else if (extension == "jpeg" || extension == "jpg")
		return "image/jpeg";
	else if (extension == "gif")
		return "image/gif";
	else if (extension == "svg" || extension == "svgz")
		return "image/svg+xml";
	else
		return "text/html";
}

void ResponseHandler::generateHTML() {
	std::stringstream html;

	html << "<!DOCTYPE html>";
	html << "<html lang=\"en\">";
	html << "<head>";
	html << "	<meta charset=\"UTF-8\">";
	html << "	<title>" << _status_code << ' ' << _status_codes.find(_status_code)->second << "</title>";
	html << "</head>";
	html << "<body>";
	html << "	<center><h1>" << _status_code << ' ' << _status_codes.find(_status_code)->second << "</h1></center>";
	html << "	<hr><center>webserv</center>";
	html << "</body>";
	html << "</html>";

	std::string tmp = html.str();
	_response_data.insert(_response_data.begin(), tmp.c_str(), tmp.c_str() + tmp.size());
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

const std::map<std::string, std::string>& ResponseHandler::getHeader() const {
	return _header;
}

std::map<std::string, std::string>& ResponseHandler::setHeader() {
	return _header;
}

int& ResponseHandler::setStatus_code() {
	return _status_code;
}

void*& ResponseHandler::setData() {
	return _data;
}

ssize_t& ResponseHandler::setData_size() {
	return _data_size;
}

const std::string& ResponseHandler::getMethods() const {
	return _methods;
}

const std::string& ResponseHandler::getLocation() const {
	return _location;
}

const Parser* ResponseHandler::getConf() const {
	return _conf;
}

const std::string& ResponseHandler::getPath() const {
	return _path;
}
