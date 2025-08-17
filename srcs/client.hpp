#ifndef CLIENT_HPP
# define CLIENT_HPP


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
#include <dirent.h>
#include <string>
#include "webserv.hpp"
#include <cstdio>
// #include <print>
// #include "request.hpp"
// #include "HttpForms.hpp"
#include <csignal>

#define WAITING 0
#define READING 1
#define READINGDATA 2
#define WRITING 3
#define CLOSING 4
#define DONE 5
#define READINGPOSTDATA 6
#define READINGHEADER 7
#define EXECUTING 8
#define DELETE 9
#define SENDERROR 10


class HttpForms;
class Request;
class CGIHandler;

class client
{
	public:
		client(){};
		client( pollfd& pfdin, ServerConfig *server):cgi_handler(NULL), serv(server)
		{
			//  if (!pfd)
			// {
			// 	std::cerr << "Invalid pollfd pointer in client CONSTRUCTOR" << std::endl;
			// }
			this->cgiresgitered =false;
			this->keepalive = false;\
			this->cgi_handler = NULL;
			std::cerr<<"new client:"<<pfdin.fd<<std::endl;
			totalrec = 0;
			data="";
			status = WAITING;
			this->fd = pfdin.fd;
			this->_request = NULL;
			std::cerr << "1, "<< pfdin.fd;
			cgi_fd=-1;
		};
		~client();

		HttpForms *_formulaire;
		Request	*_request;
		
		//METHODS
		void tryLaunchCGI();
		bool handle_jesus(pollfd& pfdin);
		bool answerClient(pollfd& pfdin);
		int getStatus()const{return status;};
		int getFd()const{return fd;};
		int status, cgi_fd;
		bool keepalive, cgiresgitered;
		std::string cgi_buffer;
		CGIHandler* cgi_handler;

		// std::string executeCGI(const std::string& scriptPath, const std::string& method, const std::string& body, std::map<std::string, std::string> env);
		
		private:
			ServerConfig *serv;
			int fd, pollstatus;
			std::string data;
			ssize_t	totalrec;



};


#endif