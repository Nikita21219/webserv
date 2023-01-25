#include "webserv.h"
#include <dirent.h>

std::vector<std::string> getListFiles(std::string path) {
    std::vector<std::string> result;
    DIR *dir = opendir(path.c_str());
    struct dirent *ent;
    while(dir && (ent = readdir(dir)) != NULL)
        result.push_back(ent->d_name);
    return result;
}

std::string getHTMLPage(std::string path, std::string location) {
    std::string result;
    std::vector<std::string> listDirs = getListFiles(path);
    result += "<h1>Index of " + location + "</h1>\n";
    result += "<hr>\n";
    result += "<pre>\n";
    for (std::vector<std::string>::iterator i = listDirs.begin(); i != listDirs.end(); i++)
        result += "<a href=\"" + *i + "\">" + *i + "</a>\n";
    result += "</pre>\n";
    result += "<hr>\n";
    return result;
}

int main(int argc, char **argv, char **env) {

    printWar(getHTMLPage(".", "/")); // TODO tmp line

    std::vector<Parser> conf;
    const char *file_conf;
    if (argc > 2) {
        std::cerr << "wrong number of arguments!\n";
        return 1;
    } else if (argc == 2)
        file_conf = argv[1];
    else
        file_conf = "/etc/webserv.conf";
    if (get_conf(file_conf, conf))
        return 1;

    try {
        Server serv = Server(conf, env);
        serv.mainLoop();
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
