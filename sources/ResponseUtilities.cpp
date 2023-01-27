#include "ResponseHandler.hpp"

/*
** --------------------------------- METHODS ----------------------------------
*/

std::vector<std::string> ResponseHandler::getListFiles(std::string path) {
    std::vector<std::string> result;
    DIR *dir = opendir(path.c_str());
    struct dirent *ent;
    while(dir && (ent = readdir(dir)) != NULL)
        result.push_back(ent->d_name);
    return result;
}

std::string ResponseHandler::getHTMLPage(std::string path, std::string location) {
    std::string result;
    std::vector<std::string> listDirs = getListFiles(path);
    result += "<h1>Index of " + location + "</h1>\n";
    result += "<hr>\n";
    result += "<pre>\n";
    for (std::vector<std::string>::iterator i = listDirs.begin(); i != listDirs.end(); i++) {
        if (location != "/")
            result += "<a href=\"" + location + "/" + *i + "\">" + *i + "</a>\n";
        else
            result += "<a href=\"" + *i + "\">" + *i + "</a>\n";
    }
    result += "</pre>\n";
    result += "<hr>\n";
    return result;
}

bool ResponseHandler::add_index_if_needed(std::string &resource_path) {
    struct stat s;
    if(stat(resource_path.c_str(), &s) == 0) {
        std::string autoindex;
        if (_conf->getLocfield(_location, "autoindex") != NOT_FOUND)
            autoindex = _conf->getLocfield(_location, "autoindex");
        else if (_conf->getServfield("autoindex") != NOT_FOUND)
            autoindex = _conf->getServfield("autoindex");
        else
            autoindex = "off";
        if(s.st_mode & S_IFDIR) {
            if (_conf->getLocfield(_location, "index") != NOT_FOUND)
                resource_path = resource_path + '/' + _conf->getLocfield(_location, "index");
            else if (_location == NOT_FOUND && _conf->getServfield("index") != NOT_FOUND)
                resource_path = resource_path + '/' + _conf->getServfield("index");
            else if (autoindex == "on") {
                std::string tmp = getHTMLPage(resource_path, _path);
                _response_data.insert(_response_data.begin(), tmp.c_str(), tmp.c_str() + tmp.size());
                return false;
            }
            else
                resource_path = resource_path + "/index.html";
            if (!access(resource_path.c_str(), F_OK))
                stat(resource_path.c_str(), &s);
        }
    } else {
        _status_code = 404;
        return false;
    }
    _last_modified = 1;
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

void ResponseHandler::createHTTPheader(std::string mimeType, std::string file_loc, bool flag) {
    std::stringstream http_header;

    http_header << "HTTP/1.1 " << _status_code << ' ' << _status_codes.find(_status_code)->second << "\r\n";
    http_header << "Accept-Ranges: bytes\r\n";
    http_header << "Version: HTTP/1.1\r\n";
    http_header << "Content-Type: " << mimeType << "; charset=utf-8\r\n";
    http_header << "Date: " << getDate(std::time(0)) << "\r\n";
    http_header << "Content-Length: " << _response_data.size() << "\r\n";
    http_header << "Server: webserv\r\n";
    if (_status_code == 405)
        http_header << "Allow: " << _methods << "\r\n";
    if (file_loc != NOT_FOUND)
        http_header << "Location: " << file_loc << "\r\n";
    if (flag)
        http_header << "Last-Modified: " << getDate(_last_modified) << "\r\n";
    if (_status_code == 413)
        http_header << "Connection: close";
    else
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

bool ResponseHandler::isCgi() {return _conf->getLocfield(_location, "bin_path") != NOT_FOUND;}

std::string ResponseHandler::setMimeType(std::string &path) {
    std::string extension = split(path, ".").back();
    if (extension == "html" || isCgi())
        return "text/html";
    else if (extension == "css")
        return "text/css";
    else if (extension == "txt")
        return "text/plain";
    else if (extension == "js")
        return "application/javascript";
    else if (extension == "png")
        return "image/png";
    else if (extension == "jpeg" || extension == "jpg")
        return "image/jpeg";
    else if (extension == "gif")
        return "image/gif";
    else if (extension == "svg" || extension == "svgz")
        return "image/svg+xml";
    else if (extension == "bmp")
        return "image/x-ms-bmp";
    else if (extension == "eot")
        return "application/vnd.ms-fontobject";
    else if (extension == "woff")
        return "font/woff";
    else if (extension == "woff2")
        return "font/woff2";
    else if (extension == "zip")
        return "application/zip";
    else if (extension == "pdf")
        return "application/pdf";
    else {
        _status_code = 415;
        return NOT_FOUND;
    }
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

void ResponseHandler::genereteWelcomePage() {
    std::stringstream w_page;

    w_page << "<!DOCTYPE html>";
    w_page << "<html>";
    w_page << "<head>";
    w_page << "<title>Welcome to webserv!</title>";
    w_page << "<style>";
    w_page << "html { color-scheme: light dark; }";
    w_page << "body { width: 35em; margin: 0 auto;";
    w_page << "font-family: Tahoma, Verdana, Arial, sans-serif; }";
    w_page << "</style>";
    w_page << "</head>";
    w_page << "<body>";
    w_page << "<h1>Welcome to webserv!</h1>";
    w_page << "<p>If you see this page, the webserv web server is successfully installed and";
    w_page << " working. Further configuration is required.</p>";
    w_page << "<p><em>Thank you for using webserv.</em></p>";
    w_page << "</body>";
    w_page << "</html>";

    std::string tmp = w_page.str();
    _response_data.insert(_response_data.begin(), tmp.c_str(), tmp.c_str() + tmp.size());
}

std::string ResponseHandler::getResourse_path() const {
    if (_root.size() > 1 && _path.size() > 1)
        return _root + _path;
    else if (_root.size() > 1)
        return _root;
    else
        return _path;
}

bool ResponseHandler::folderIsNotEmpty(std::string &resource_path) const {
    DIR* dir = opendir(resource_path.c_str());
    if (dir) {
        struct dirent* ent;
        int count = 0;
        while ((ent = readdir(dir)) != NULL) {
            count++;
        }
        closedir(dir);
        if (count > 2) {
            return true;
        } else
            return false;
    } else
        return false;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

std::map<std::string, std::string>& ResponseHandler::setHeader() {
    return _header;
}

int& ResponseHandler::setStatus_code() {
    return _status_code;
}

std::vector<unsigned char>& ResponseHandler::setData() {
    return _data;
}

std::string& ResponseHandler::setMethods() {
    return _methods;
}

std::string& ResponseHandler::setLocation() {
    return _location;
}

std::string& ResponseHandler::setPath() {
    return _path;
}

std::string& ResponseHandler::setRoot() {
    return _root;
}

const Parser*& ResponseHandler::setConf() {
    return _conf;
}

std::time_t& ResponseHandler::setLast_modified() {
    return _last_modified;
}

std::vector<unsigned char>& ResponseHandler::setResponse_data() {
    return _response_data;
}
