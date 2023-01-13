#include "ResponseHandler.hpp"

/*
** --------------------------------- METHODS ----------------------------------
*/

int ResponseHandler::answerToPOST() {

	std::string file_name = create_filename();
	if (file_name == "_error") {
		_status_code = 400;
		return generateErrorPage();
	} else if (!access(file_name.c_str(), F_OK)) {
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
		createHTTPheader(_header.find("Content-Type")->second, file_name, NOT_FOUND, false);
	else
		createHTTPheader("text/plain", file_name, NOT_FOUND, false);
	return RequestHandler::READY_TO_ASWER;
}

std::string ResponseHandler::create_filename() const {
	std::string name;
	if (_header.find("Content-Disposition") != _header.end() &&\
		_header.find("Content-Disposition")->second.find("filename") != std::string::npos) {
		name = _header.find("Content-Disposition")->second;
		name = name.substr(name.find("filename") + 9, name.size() - name.find("filename") + 9);
		name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
	} else {
		if (_header.find("Content-Type") == _header.end())
			return "_error";
		srand (time(0));
		std::stringstream r;
		r << rand() % 100000 + 1;
		name = _header.find("Content-Type")->second;
		name = name.substr(name.find('/') + 1, name.size() - name.find('/') + 1);
		name = r.str() + '.' + name;
	}
	if (_root.at(_root.size() - 1) != '/')
		return _root + '/' + name;
	else
		return _root + name;
}

/*void ResponseHandler::successful_response_html(std::string s) {
	std::stringstream html;

	html << "<!DOCTYPE html>";
	html << "<html lang=\"en\">";
	html << "<head>";
	html << "	<meta charset=\"UTF-8\">";
	html << "</head>";
	html << "<body>";
	html << "	<right><h2>" << "File " << s << "</h2></right>";
	html << "	<hr><center>webserv</center>";
	html << "</body>";
	html << "</html>";

	std::string tmp = html.str();
	_response_data.insert(_response_data.begin(), tmp.c_str(), tmp.c_str() + tmp.size());
}*/
