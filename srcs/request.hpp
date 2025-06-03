#ifndef REQUEST_HPP

# define REQUEST_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>  // for std::invalid_argument
#include <vector>
#include <map>
#include <stack>
#include <sstream>
#include <arpa/inet.h>
#include <filesystem>
#include <sys/stat.h>
#include <algorithm>
#include "webserv.hpp"

class Request
{
    public:
      Request(char* buffer);
      Request(char* buffer, const ServerConfig &serv, int socket);
      ~Request();
      void check_allowed_methods(const ServerConfig &serv);
      void execute();
      std::string _get_ReqContent();

    private:
        int _socket;
        std::map<std::string,std::string> http_params;

        std::string r_method, r_location, r_version, r_full_request;
        LocationConfig _loc;
        bool authorized;
        void Post();
        void Get();
        void Put();
        void Patch();
        void Delete();
        void Head();
        void Options();
        std::string _ReqContent=
        			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 134\r\n"
			"Connection: close\r\n"
			"\r\n"
			"<html>\r\n"
			"<head><title>WEBSERV - 404</title></head>\r\n"
			"<body>\r\n"
			"<h1>ERROR 404</h1>\r\n"
			"<h2>cheh</h2>\r\n"
			"<p>The requested URL was found on this server </p>\r\n"
			"</body>\r\n"
			"</html>";

};

#endif