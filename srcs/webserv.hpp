#ifndef WEBSERV_HPP

# define WEBSERV_HPP

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

#define PORT 8080
#define BACKLOG 10
#define MAX_EVENTS 10


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
        allowed_methods,
        index_files;
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
        void handle_client(int client_socket, const ServerConfig &serv);
        std::vector<ServerConfig> servers;
    public:
        Webserv(void);
        ~Webserv();
        void init(void);
        void start(void);
        bool parseConfigFile(const std::string& filename);
};

template <typename T>
void fill_ctn_data(T& ctn, std::string line)//take each word and put in the container. whitespace separator
{
	std::istringstream iss(line);
	std::string word;
	while (iss >> word)
	{
        // std::cerr<<"word: "<<word<<std::endl;
        if(word == "methods")continue ;
        if(*word.rbegin()== ';')
            word.erase(word.end() -1);

		ctn.push_back(word);
	}
}
#endif