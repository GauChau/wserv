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
#include <dirent.h>
#include <string>

struct file_id
{
    std::string name, fname, type;
};

class Request
{
    public:
      Request(char* buffer, const ServerConfig &serv, int socket, ssize_t bytes_rec);
      ~Request();
      void check_allowed_methods(const ServerConfig &serv);
      void execute(std::string s);
      std::string _get_ReqContent();

    private:
        const ServerConfig &_server;
		    int _socket;
        std::map<std::string,std::string> http_params;
		    std::string r_method, r_location,
                    r_version, r_boundary,
                    r_body, r_header;
        LocationConfig _loc;
        ssize_t _bytes_rec, _contlen, ret;
        file_id file;
        bool authorized;
        void Post();
        void Get();
        void Delete();
        void writeData();
        int checkbound(std::istringstream& s);
        std::string _ReqContent;

};

#endif