#include "ResponseHandler.hpp"

/*
** --------------------------------- METHODS ----------------------------------
*/

int ResponseHandler::answerToPOST() {

	std::string file_name;
	try {
		file_name = create_filename();
	} catch (int) {
		return generateErrorPage();
	}
	if (!access(file_name.c_str(), F_OK)) {
		_status_code = 409;
		return generateErrorPage();
	}
	std::ofstream f_out(file_name.c_str(), std::ios::out | std::ios::binary);
	if (!f_out.is_open()) {
		_status_code = 403;
		return generateErrorPage();
	}
	f_out.write(reinterpret_cast<char *>(_data.data()), _data.size());
	if (f_out.bad()) {
		f_out.close();
		_status_code = 507;
		return generateErrorPage();
	}
	f_out.close();
	_data.clear();
	_status_code = 201;
	if (_header.find("Content-Type") != _header.end())
		createHTTPheader(_header.find("Content-Type")->second, file_name, false);
	else
		createHTTPheader("text/plain", file_name, false);
	return RequestHandler::READY_TO_ASWER;
}

std::string ResponseHandler::create_filename() {
	std::string name;
	if (_header.find("Content-Disposition") != _header.end() &&\
		_header.find("Content-Disposition")->second.find("filename") != std::string::npos) {
		name = _header.find("Content-Disposition")->second;
		name = name.substr(name.find("filename") + 9, name.size() - name.find("filename") + 9);
		name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
	} else {
		if (_header.find("Content-Type") == _header.end()) {
			_status_code = 400;
			throw 1;
		}
		srand (time(0));
		std::stringstream r;
		r << rand() % 100000 + 1;
		name = _header.find("Content-Type")->second;
		name = name.substr(name.find('/') + 1, name.size() - name.find('/') + 1);
		name = r.str() + '.' + name;
	}
	check_path_errors();
	return _root + _path + '/' + name;
}

void ResponseHandler::check_path_errors() {
	struct stat s;
	if(stat((_root + _path).c_str(), &s) == 0) {
		if(!(s.st_mode & S_IFDIR)) {
			_status_code = 400;
			throw 1;
		}
	} else {
		_status_code = 404;
		throw 1;
	}
}
