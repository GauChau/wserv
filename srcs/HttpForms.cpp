#include "HttpForms.hpp"
#include "webserv.hpp"
#include "request.hpp"

// Constructors
HttpForms::HttpForms(int socket,int code): _socket(socket)
{
	this->_code_forms.insert(std::make_pair(200, "HTTP/1.1 200 OK\r\n"));
	this->_code_forms.insert(std::make_pair(403, "HTTP/1.1 403 Forbidden\r\n"));
	this->_code_forms.insert(std::make_pair(404, "HTTP/1.1 404 Not Found\r\n"));
	this->_code_forms.insert(std::make_pair(405, "HTTP/1.1 405 Method Not Alloweds\r\n"));
	this->_code_forms.insert(std::make_pair(413, "HTTP/1.1 413 Payload Too Large\r\n"));
	this->_code_forms.insert(std::make_pair(500, "HTTP/1.1 500 Internal Server Error\r\n"));

	this->_body = "";
	this->_content_type = "";
	this->_connection = "close";
	this->_contentlen = 0;

	std::stringstream res(this->_response);
	res << this->_code_forms.find(code)->second;
	res << "Content-Type: "<< this->_content_type<<"\r\n";
	res << "Content-Length" << this-> _contentlen<<"\r\n";
	res	<< "Connection: " << this->_connection<<"\r\n";
	res << "\r\n";
	res << this->_body;



}

HttpForms::HttpForms(int socket,int code, std::string ctype, std::string body): _socket(socket)
{
	this->_code_forms.insert(std::make_pair(200, "HTTP/1.1 200 OK\r\n"));
	this->_code_forms.insert(std::make_pair(403, "HTTP/1.1 403 Forbidden\r\n"));
	this->_code_forms.insert(std::make_pair(404, "HTTP/1.1 404 Not Found\r\n"));
	this->_code_forms.insert(std::make_pair(405, "HTTP/1.1 405 Method Not Alloweds\r\n"));
	this->_code_forms.insert(std::make_pair(413, "HTTP/1.1 413 Payload Too Large\r\n"));

	this->_body = body;
	this->_content_type = ctype;
	this->_connection = "close";
	this->_contentlen = this->_content_type.size();

	std::stringstream res;
	res << this->_code_forms.find(code)->second;
	res << "Content-Type: "<< this->_content_type<<"\r\n";
	res << "Content-Length" << this-> _contentlen<<"\r\n";
	res	<< "Connection: " << this->_connection<<"\r\n";
	res << "\r\n";
	res << this->_body;
	this->_response = res.str();
}

HttpForms::HttpForms(int socket,int code, std::string ctype, std::string body, std::string &req): _socket(socket)
{
	this->_code_forms.insert(std::make_pair(200, "HTTP/1.1 200 OK\r\n"));
	this->_code_forms.insert(std::make_pair(403, "HTTP/1.1 403 Forbidden\r\n"));
	this->_code_forms.insert(std::make_pair(404, "HTTP/1.1 404 Not Found\r\n"));
	this->_code_forms.insert(std::make_pair(405, "HTTP/1.1 405 Method Not Alloweds\r\n"));
	this->_code_forms.insert(std::make_pair(413, "HTTP/1.1 413 Payload Too Large\r\n"));

	this->_body = body;
	this->_content_type = ctype;
	this->_connection = "close";
	this->_contentlen = this->_content_type.size();

	std::stringstream res;
	res << this->_code_forms.find(code)->second;
	res << "Content-Type: "<< this->_content_type<<"\r\n";
	res << "Content-Length" << this-> _contentlen<<"\r\n";
	res	<< "Connection: " << this->_connection<<"\r\n";
	res << "\r\n";
	res << this->_body;
	// std::cerr<<"\n\nINSIDE HTTP: "<<res.str()<<std::endl;
	this->_response = res.str();
	req = this->_response;
}
void HttpForms::_sendclose(void)
{
	send(this->_socket, this->_response.data(), this->_response.size(),0);
	close(this->_socket);
}
void HttpForms::_send(void)
{
	send(this->_socket, this->_response.data(), this->_response.size(),0);
}

HttpForms::HttpForms(const HttpForms &copy)
{
	(void) copy;
}


// Destructor
HttpForms::~HttpForms()
{
}


// Operators
HttpForms & HttpForms::operator=(const HttpForms &assign)
{
	(void) assign;
	return *this;
}

