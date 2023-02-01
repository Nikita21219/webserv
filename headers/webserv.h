//
// Created by Nikita Madorsky on 06.12.2022.
//

#ifndef WEBSERV_WEBSERV_H
#define WEBSERV_WEBSERV_H

#include "color.hpp"

#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <ctime>
#include <locale>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <memory>

#include "Utils.hpp"
#include "Parser.hpp"
#include "TempFile.hpp"
#include "CgiEnv.hpp"
#include "Cgi.hpp"
#include "ResponseHandler.hpp"
#include "RequestHandler.hpp"
#include "SocketMaster.hpp"
#include "Server.hpp"

#endif //WEBSERV_WEBSERV_H
