#ifndef HTTPMESSAGE_HPP
# define HTTPMESSAGE_HPP

# include <iostream>
# include <string>
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


class HttpMessage
{
	public:
		// Constructors
		HttpMessage(int socket);
		HttpMessage(const HttpMessage &copy);

		// Destructor
		~HttpMessage();

		//method
		void saveData();
		void extract_Boundary();
		// Operators
		HttpMessage & operator=(const HttpMessage &assign);

	private:
	int _socket;
	std::string _method, _location, _version, _header, _boundary, _data;
	ssize_t _contentlen;
	std::vector<std::pair
					<std::map<std::string,std::string>,
					std::string>> _messagevector;

};

#endif