#include "HttpMessage.hpp"
#define HEADER_PARSER 0
#define BOUNDARY_EXTRACT_AFTER_HEADER 1
#define DATA_HEADER 2
#define DATA_BODY 3


std::istringstream&  strdelimgetline(std::istringstream &s,std::string &buffer, std::string delim)
{
	std::istringstream::pos_type pos = s.tellg(),pos2;
	if(pos == -1)
		{
			return s;
		}
	std::string::size_type spos(pos);
	std::string sstr=s.str();
	sstr = sstr.substr(spos);
	spos = sstr.find(delim);
	if (spos == 0)
	{
		pos2 = delim.length();
		s.seekg(pos + pos2);
	}
	if(spos != std::string::npos )
	{
		sstr = sstr.substr(0, spos);
		pos2=spos;
		pos2+=delim.length();
		s.seekg(pos + pos2);
		if(s.peek() == s.eof())
		{
			return s;
		}
		buffer = sstr;
		return s;
	}
	return s;

}


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
	this->_contentlen = 1;
	ssize_t bytes_read = 0;
	int state = HEADER_PARSER;
	while (bytes_read < this->_contentlen)
	{
		ssize_t ret=recv(this->_socket, buf, sizeof(buf),0);
		if(ret<=0)
		{
			return ;
		}

		switch (state)
		{
			case HEADER_PARSER:
			{
				std::string buffer(buf);
				std::string::size_type pos = buffer.find("\r\n\r\n");
				std::map<std::string, std::string> http_params;

				if (pos != std::string::npos)
				{
					this->_header += buffer.substr(0,pos);
					this->_data = buffer.substr(pos+4);
					std::istringstream iss(_header);
					if(bytes_read = 0)
						iss >> this->_method >> this->_location >> this->_version;
					while (strdelimgetline(iss, buffer,"\r\n"))
					{
						if (buffer.size() > 1) {
							std::string key, value;
							std::istringstream iss2(buffer);
							getline(iss2, buffer, ':');
							key = buffer;
							strdelimgetline(iss2, buffer,"\r\n");
							value = buffer;
							http_params.insert(std::make_pair(key, value));
						}
					}
					this->_messagevector.push_back(std::make_pair(http_params, "HEADER TERRITORY"));

					if (http_params.find("Content-Length") != http_params.end())
					{
						this->_contentlen = atol(http_params["Content-Length"].c_str());
						if (this->_contentlen > 10 * 1024 * 1024) { // 10 MB limit
							std::string res =
							"HTTP/1.1 413 Payload Too Large\r\n"
							"Content-Length: 0\r\n\r\n";
							send(this->_socket, res.c_str(), res.size(), 0);
							close(this->_socket);
							return;
						}
					}

					state = BOUNDARY_EXTRACT_AFTER_HEADER;
				}
				else
				{
					this->_header+=buf;
					break;
				}
			}

			case BOUNDARY_EXTRACT_AFTER_HEADER:
			{
				this->extract_Boundary();
				state = DATA_HEADER;
			}

			case DATA_HEADER:
			{
				std::string	buffer;
				if (this->_contentlen == 0)
				{
					buffer = this->_data;
				}
				else
				{
					buffer = buf;
				}
				std::istringstream is(buffer);

				while (strdelimgetline(is,buffer,"\r\n"))
				{
					if(buffer.length()==0)
					{

						break;
					}
					else
					{
						std::map<std::string, std::string> http_params;
						std::string key, value;
						std::istringstream iss2(buffer);
						getline(iss2, buffer, ':');
						key = buffer;
						strdelimgetline(iss2, buffer,"\r\n");
						value = buffer;
						http_params.insert(std::make_pair(key, value));
						this->_messagevector.push_back(std::make_pair(http_params, "HEADER TERRITORY"));
					}

				}
				state = DATA_BODY;


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



