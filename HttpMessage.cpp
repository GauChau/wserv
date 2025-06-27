#include "HttpMessage.hpp"

// Constructors
HttpMessage::HttpMessage(int socket):_socket(socket)
{
}

void HttpMessage::extract_Boundary()
{
	std::map<std::string, std::string> http_params = this->_messagevector.begin()->first;
	std::map<std::string,std::string>::iterator it = http_params.find("Content-Type");
	if(it == http_params.end())
		return ;
	std::string::size_type pos = it->second.find("boundary=");
    if (pos != std::string::npos)
	{
        this->_boundary = http_params.find("Content-Type")->second.substr(pos+9);
		this->_boundary.resize(this->_boundary.size()-1);
		this->_boundary = "--" + this->_boundary;
    }
	else
		this->_boundary = "void";
}

void HttpMessage::saveData()
{
	char	buf[4096];
	ssize_t content_len = 1, bytes_read = 0;
	int state = 0;
	while (bytes_read < content_len)
	{
		ssize_t ret=recv(this->_socket, buf, sizeof(buf),0);
		if(ret<=0)
		{
			return ;
		}

		switch (state)
		{
			case 0:
			{
				std::string buffer(buf);
				std::string::size_type pos = buffer.find("\r\n\r\n",0);
				std::map<std::string, std::string> http_params;

				if (pos != std::string::npos)
				{
					this->_header += buffer.substr(0,pos);
					std::istringstream iss(buf);
					iss >> this->_method >> this->_location >> this->_version;
					while (getline(iss, _header))
					{
						if (_header.size() > 1) {
							std::string key, value;
							std::istringstream iss2(_header);
							getline(iss2, _header, ':');
							key = _header;
							getline(iss2, _header);
							value = _header;
							http_params.insert(std::make_pair(key, value));

						}
					}
					this->_messagevector.push_back(std::make_pair(http_params, buffer.substr(pos)));
					state= 1;
				}
				else
				{
					this->_header+=buf;
				}
				break;
			}

			case 1:
			{
				this->extract_Boundary();
				state = 2;
			}

			case 2:
			{
				std::string	buffer(buf);
				std::string::size_type	pos = buffer.find(this->_boundary+'\r', 0),
										epos = buffer.find(this->_boundary + "--\r", 0),
										jpos = buffer.find("\r\n\r\n",0);
				if(epos != std::string::npos)
				{
					state = 3;
					break;
				}
				if (pos != std::string::npos)
				{
					if (jpos != std::string::npos)
					{

					}
					break ;
				}
			}

			default:
				break;
		}

		bytes_read += ret;

	}
}

HttpMessage::HttpMessage(const HttpMessage &copy)
{
	(void) copy;
}


// Destructor
HttpMessage::~HttpMessage()
{
}


// Operators
HttpMessage & HttpMessage::operator=(const HttpMessage &assign)
{
	(void) assign;
	return *this;
}

