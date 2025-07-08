#ifndef HTTPFORMS_HPP
# define HTTPFORMS_HPP

# include <iostream>
# include <string>

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
#include <sys/wait.h>
class HttpForms
{
	public:
		// Constructors
		HttpForms(int socket,int code);
		HttpForms(int socket,int code, std::string ctype, std::string body);
		HttpForms(int socket,int code, std::string ctype, std::string body, std::string &res);

		// Destructor
		~HttpForms();

		//method
		void _sendclose(void);
		void _send(void);

		// Operators
		HttpForms & operator=(const HttpForms &assign);

	private:
		int _socket;
		ssize_t _contentlen;
		std::map<int,std::string> _code_forms;
		std::string _content_type, _body, _connection;
		std::string badrequest, _error, _not_found, _ok;
		std::string _response;

};

#endif