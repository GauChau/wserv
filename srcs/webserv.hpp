#ifndef WEBSERV_HPP

# define WEBSERV_HPP

#include <dirent.h>
#include <sys/types.h>
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
#include <fcntl.h>
#include <set>

#define PORT 8080
#define BACKLOG 10
#define MAX_EVENTS 10

/*
    routes examples: 
        route / {
            methods GET POST;
            root /var/www/html;
            index index.html;
            autoindex off;
        }

        route /uploads {
            methods POST;
            root /var/www/uploads;
            upload_store /var/www/uploads;
        }

    struct sockaddr_in {
        sa_family_t    sin_family;   // Address family: ex AF_INET -> IPv4
        in_port_t      sin_port;     // Port number: ex 443(HTTPs)
        struct in_addr sin_addr;     // Internet address (struct in_addr)
        char           sin_zero[8];  // Padding to make the structure size 16 bytes
    };
*/

struct LocationConfig 
{
    std::string 
        path,
        root,
        cgi_extension,
        cgi_path,
        index,
        upload_store;
    std::vector<std::string> 
        allowed_methods;
    std::string redirect_url;
    bool autoindex;
};

struct ServerConfig
{
    std::string 
        host,
        server_name;
    int 
        port,
        server_socket,
        client_socket;
    struct sockaddr_in 
        server_addr, 
        client_addr;
    socklen_t 
        client_addr_len;
    size_t 
        client_max_body_size;
    std::vector<LocationConfig>
        locations;
    std::vector<std::pair<unsigned int, std::string> > 
        error_pages;
};

class Webserv
{
    private:
        std::vector<ServerConfig> servers;
    public:
        Webserv(void);
        ~Webserv();
        void init(void);
        void start(void);   
        bool parseConfigFile(const std::string& filename);
};

#endif