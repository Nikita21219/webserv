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

#include "Utils.hpp"
#include "TempFile.hpp"
#include "Parser.hpp"
#include "CgiEnv.hpp"
#include "Cgi.hpp"
#include "Request.hpp"
#include "Server.hpp"

// #include <string.h>// hello ubuntu

#endif //WEBSERV_WEBSERV_H
