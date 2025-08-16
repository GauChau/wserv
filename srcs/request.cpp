#include "request.hpp"
#include "HttpForms.hpp"
#include "utils.hpp"


Request::~Request()
{ std::cout<<"REQUEST DESTROYED:";}

void Request::readRequest()
{
	std::cerr<<" read ";
	char buffer[4096];
	this->_bytes_rec = recv(this->_socket, buffer, sizeof(buffer),0);
	if (this->_bytes_rec > 0)
	{
		this->_datarec.append(buffer, this->_bytes_rec);
		this->r_body.append(buffer, this->_bytes_rec);
		this->_totalrec+=this->_bytes_rec;

	}
	else if (this->_bytes_rec == 0)
	{
		std::cerr<<" brec=0 ";
		this->_request_status = EXECUTING;
	}
	std::cerr<<" readDONE ";
}

void Request::sendResponse()
{
	ssize_t sent = 0;

	sent = send(this->_socket, this->_ReqContent.data() + this->_totalsent, this->_ReqContent.size() - this->_totalsent, 0);
	this->_totalsent += sent;
    
    if (sent < 0)
    {
        std::cerr << "\033[31m[x] send() failed: " << strerror(errno) << "\033[0m\n";
        this->_request_status = DONE;
    }
	else if(_totalsent >= this->_ReqContent.size())
	{
		this->_request_status = DONE;
	}
}
void Request::requestParser()
{
	this->r_header.append(this->_datarec.data(), this->_totalrec);
    std::istringstream iss(this->_datarec);
    std::string buffer;
    iss >> this->r_method >> this->r_location >> this->r_version;
	std::string::size_type pos = this->r_header.find("\r\n\r\n",0);
	if (pos != std::string::npos)
	{
		this->_totalrec -= pos + 4;
		this->r_body.clear();
		this->r_body.append(this->_datarec.data()+pos+4, this->_totalrec);

	}
	std::istringstream iss3(this->r_header.substr(0,pos));
    while (getline(iss3, buffer))
	{
        if (buffer.size() > 1)
		{
            std::string key, value;
            std::istringstream iss2(buffer);
            getline(iss2, buffer, ':');
			if(this->http_params.find(buffer)!= this->http_params.end())
			{
				HttpForms Badrequest(this->_socket, 400, 0, "","",this->_ReqContent);
				this->_request_status = WRITING;
				return ;
			}
            key = buffer;
            getline(iss2, buffer);
            value = buffer;
            this->http_params.insert(std::make_pair(key, value));
        }
    }
	if(this->http_params.find("Connection")!=this->http_params.end())
	{
		std::string connec;
		connec = this->http_params["Connection"];
		if (connec.find("keep-alive") !=std::string::npos)
			this->keepalive = true;
		else
			this->keepalive = false;
	}
    check_allowed_methods(this->_server);
}

int Request::checkPostDataOk()
{
	std::cerr<<" CHECKPOSTDATA. ";
	//CHECKS IF ALL DATA IS RECEIVED OR IF READING AGAIN IS NECESSARY
	if (this->_totalrec < this->_contlen)
	{
		std::cerr<<" NEEDMOREDATA rec: "<< this->_totalrec<< " ctlen: "<< this->_contlen ;
		this->_request_status = READINGDATA;
		return 0;
	}
	else //All data is received, executing post request
	{
		std::cerr<<" DATARECED. ";
		this->_request_status = EXECUTING;
		return 1;
	}
	return 1;
}

int Request::checkHeaderCompletion()
{
	std::cerr<<" CHECKHEADER ";
	std::string::size_type pos = this->_datarec.find("\r\n\r\n",0);
	if (pos != std::string::npos) //HEADER COMPLETE, PARSE IT AND CHOOSE STATE ACCORDINGLY
	{
		std::cerr<<" HEAD DONE. ";
		this->requestParser();
		// if(this->r_method == "POST")//CHECKS IF ALL DATA IS RECEIVED OR IF READING AGAIN IS NECESSARY
		// 	checkPostDataOk();
		// else//Method is get or delete, so no content. execute the request
		// {
			this->_request_status = EXECUTING;
		// }
		return true;
	}
	else //header incomplete
	{
		this->_request_status = READINGHEADER;
		if (this->_bytes_rec ==0)
		{	
			this->_request_status = WAITING;
			keepalive == false;
		}
		return false;
	}
}

Request::Request(const ServerConfig &serv, int socket, int status): _socket(socket),_server(serv)
{
	this->_totalrec = 0;
	this->_totalsent = 0;
	this->_request_status = status;
	this->_datarec = "";
	std::cerr<<"\n REQUEST: ";
	this->readRequest();

	//CHECKS IF HEADER IS COMPLETE
	checkHeaderCompletion();//header complete

	if(this->_request_status == EXECUTING)
	{
		this->execute(this->exec_code);
		
	}
	else
		return ;
	


}





void Request::check_allowed_methods(const ServerConfig &server)
{
	LocationConfig location_target;
	if(match_location(this->r_location, server.locations, location_target))
	{
		this->_loc = location_target;
		std::vector<std::string>::const_iterator it_meth = location_target.allowed_methods.begin();
		for(;it_meth != location_target.allowed_methods.end();it_meth++)
		{
			if(this->r_method == *it_meth)
			{
				if(location_target.root.size() > 0)
				{
					this->location_filename = this->r_location;
					this->location_filename.erase(0, this->_loc.path.size());
					// if (this->_loc.upload_store.size() >= 1)
				}
					// 	this->location_filename.insert(0, this->_loc.upload_store.substr(1));
					this->location_filename.insert(0, this->_loc.root);
				this->authorized = true;
				this->exec_code = ""; // <--- then execute it
				this->_request_status = WRITING;
				return ;
			}
		}
		// 405
		if(location_target.allowed_methods.size() == 0)
			this->exec_code = ""; // <--- then execute it
		else
			this->exec_code = "405"; // <--- then execute it
		// this->_request_status = WRITING;
		return ;
	}
	this->exec_code = "404"; // <--- then execute it
}


void Request::execute(std::string s = "null")
{
	std::cerr<<" EXEC ";
	if(this->exec_code == "405")
	{
		const std::string&
				body = nonblocking_read("./www/errors/405.html"),
				contentType = "text/html";
		std::string bodi = body;
		bodi += "<h2>" + this->_loc.root + " only handles:</h2>";
		for (size_t i = 0; i < this->_loc.allowed_methods.size(); ++i)
			bodi += "<h2> - " + this->_loc.allowed_methods[i] + " Method</h2>";
		bodi += "<p>" + this->_loc.index + " only handles: </p>";

		for(unsigned long i = 0; i < this->_loc.allowed_methods.size(); i++)
			bodi += "<p> - " + this->_loc.allowed_methods[i] + " Method</p>";

		HttpForms notallowed(this->_socket,405,this->keepalive,contentType,bodi,this->_ReqContent);
	}else
	{
		// std::cout<<this->_bytes_rec << " vs " << this->_contlen << " ret: "<< this->ret<<std::endl;
		if (this->r_method == "GET")
			this->Get();
		else if (this->r_method == "POST")
			this->Post();
		else if (this->r_method == "DELETE")
			this->Delete();
	}
	std::cerr<<" AFTER EXEC ";
}


void	Request::writeData()
{
	bool parsestate = false;
	if (this->r_boundary =="void") // boundary void = instant data mode?
		parsestate = true;
	std::istringstream s(this->r_body, std::ios::binary);
	std::string buf;
	while(getline(s,buf))
	{
		if (buf == this->r_boundary + "--\r")
			break;
		else if (buf == this->r_boundary+'\r' || buf == this->r_boundary)
			parsestate = !parsestate;
		else if (parsestate && this->r_boundary != "void")
		{
			this->file.fname = extract_field_path(buf, "filename=\"");
			if(this->file.fname.size() == 0)
				this->file.fname = extract_field_path(buf, "name=\"");

			getline(s,buf);
			std::string::size_type pos = buf.find("Content-Type: ");
			if (pos != std::string::npos)
				this->file.type = buf.substr(pos+14);
			parsestate = !parsestate;

			getline(s,buf);
			std::string 
					safe_name = sanitize_filename(this->file.fname),
					full_path = this->_loc.upload_store + "/" + safe_name;
			this->file.name = full_path;
			int socket_fd = open(full_path.c_str(), O_WRONLY | O_NONBLOCK | O_CREAT | O_TRUNC,0644);
			close(socket_fd);
		}
		else if (parsestate && this->r_boundary=="void")
		{
			this->file.fname = extract_field_path(this->r_header, "filename=\"");
			if(this->file.fname.size() ==0)
				this->file.fname = extract_field_path(this->r_header, "name=\"");

			std::string::size_type pos;
			pos = this->r_header.find("Content-Type: ");
			if (pos != std::string::npos)
				this->file.type = this->r_header.substr(pos+14);

			parsestate = !parsestate;
			std::string 
					safe_name = sanitize_filename(this->file.fname),
					full_path = this->_loc.upload_store + "/" + safe_name;
			this->file.name = full_path;
			// nonblocking_write(full_path, this->r_body.data(), this->r_body.size());
			std::ofstream outjoe(full_path.c_str(), std::ios::app | std::ios::binary);
			outjoe << this->r_body;
			break;
		}
		else
		{
			std::string& filename = this->file.name;
			const std::string& content = buf+'\n';
			// nonblocking_write(filename.c_str(), content.data(), content.size());
			std::ofstream outjoe(filename.c_str(), std::ios::app | std::ios::binary);
			outjoe << content;
		}

	}
}

void Request::Post_data_write()
{
	this->_request_status = WRITING;

	try
	{
		if(this->location_filename.size()> this->_loc.root.size())
		{
			HttpForms notok(this->_socket, 404,this->keepalive,"","",this->_ReqContent);
			this->_request_status = WRITING;
		}
		else
		{
			this->_request_status = WRITING;
			this->writeData();
			HttpForms ok(this->_socket, 200,this->keepalive,"","",this->_ReqContent);
			std::cout << "\033[32m[✓] POST request handled successfully!\033[0m" << std::endl;
		}
	}
	catch(const std::ofstream::failure& e)
	{
		this->_request_status = WRITING;
		HttpForms forbid(this->_socket, 403, this->keepalive, "", "", this->_ReqContent);
		std::cerr << e.what() << '\n';
	}
}

void Request::Post()
{
	if (this->_request_status == EXECUTING)
	// handle-boundary
	{
		if (this->http_params.find("Content-Type")!= this->http_params.end())
		{
			std::string::size_type pos = this->http_params.find("Content-Type")->second.find("boundary=");
			if (pos != std::string::npos)
			{
				this->r_boundary = this->http_params.find("Content-Type")->second.substr(pos+9);
				if (*this->r_boundary.rbegin() == '\r')
					this->r_boundary.resize(this->r_boundary.size() - 1);
				this->r_boundary = "--" + this->r_boundary;
				if (*this->r_boundary.rbegin() == ' ')
					this->r_boundary.resize(this->r_boundary.size() - 1);
			}
			else
				this->r_boundary = "void";
		}
		else
			this->r_boundary = "void";

		if (this->http_params.find("Content-Length") != this->http_params.end())
		{
			this->_contlen = atol(this->http_params["Content-Length"].c_str());
			if (this->_contlen > this->_server.client_max_body_size * 1024 * 1024) // 10 MB limit
			{
				HttpForms toolarge(this->_socket, 413, false, "", "Payload Toolarge", this->_ReqContent);
				this->_request_status = WRITING;
				return ;
			}
		}
		if(this->checkPostDataOk())
		{
			Post_data_write();
		}
		else
			this->_request_status == READINGDATA;
	}
	

}



void Request::Get()
{
    std::string full_path = this->_loc.root;
    std::string file_path;
	this->_request_status = WRITING;

    struct stat st;

	std::cerr<<" GETMETH_status: " << this->_request_status<<"";
	if (stat(full_path.c_str(), &st) == 0)
    {
		if (S_ISDIR(st.st_mode) && (!this->_loc.index.empty() ||
		((&(this->_loc.cgi_extension) != NULL && !this->_loc.cgi_extension.empty())) ))
        {
			file_path = full_path + "/" + this->_loc.index;
			if(this->location_filename.size()> this->_loc.root.size())
			{
				file_path = this->location_filename;
				if (is_directory(file_path.c_str()))
					file_path = "[AUTOINDEX]";
				else
				{
					std::ifstream file(file_path.c_str());
					if(!file.is_open())
						file_path = "[404]";
				}
			}
        }
        else
        {
            if (this->_loc.autoindex)
                file_path = "[AUTOINDEX]";
            else
                file_path = "[403]";
        }
    }
    else
    {
        file_path = "[404]";
    }
	if(file_path == "[404]")
		if( this->_loc.redirection.size() > 0)
			file_path = "[REDIRECTION]";

    //  autoindex
    if (file_path == "[AUTOINDEX]" )
    {
		std::stringstream listing;
		DIR* dir = opendir(full_path.c_str());
		if (!dir) {
			listing << "<li><em>Directory not found: " << full_path << "</em></li>";
		} else {
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL)
			{
				std::string name = entry->d_name;
				if (name == "." || name == "..")
					continue;
				listing << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
			}
			closedir(dir);
		}

		std::string body = nonblocking_read("./www/errors/autoindex.html");
		size_t pos = body.find("<!--CONTENT-->");
		if (pos != std::string::npos) {
			body.replace(pos, std::string("<!--CONTENT-->").length(), listing.str());
		}
		HttpForms notfound(this->_socket,this->keepalive, 200,"text/html", body,this->_ReqContent);
	}
	// 404 no
	else if (file_path == "[404]")
	{
		std::string path_404 = "./www/default/404.html";
		std::vector<std::pair<unsigned int, std::string> >::const_iterator it;
		it = this->_server.error_pages.begin();
		for (; it != this->_server.error_pages.end(); ++it)
		{
			if (it->first == 404)
			{
				path_404 = it->second;
				break;
			}
		}
		const std::string&
				body = nonblocking_read(path_404),
				contentType = "text/html";
		HttpForms notfound(this->_socket, 404,this->keepalive,contentType, body,this->_ReqContent);
	}
	// 403 forbidden
	else if (file_path == "[403]")
	{
		std::string path_403 = "./www/default/403.html";
		std::vector<std::pair<unsigned int, std::string> >::const_iterator it;
		it = this->_server.error_pages.begin();
		for (; it != this->_server.error_pages.end(); ++it)
		{
			if (it->first == 403)
			{
				path_403 = it->second;
				break;
			}
		}
		const std::string&
				body = nonblocking_read(path_403),
				contentType = "text/html";
		HttpForms forbid(this->_socket, 403,this->keepalive, contentType, body,this->_ReqContent);
	}
	else if (file_path == "[REDIRECTION]")
	{
		std::stringstream res;
		res << "HTTP/1.1 301 Moved Permanently" << "\r\n";
		res << "Location: "<< this->_loc.redirection <<"\r\n";
		res << "Content-Length: " << 0 <<"\r\n";
		res	<< "Connection: close" << "\r\n";
		res << "\r\n";
		this->_ReqContent = res.str();
	}
	else
	{
		// default get
		if(&(this->_loc.cgi_extension) == NULL || this->_loc.cgi_extension.empty())
		{
			const std::string&
				body = nonblocking_read(file_path);
				std::string contentType=file_path.substr(file_path.rfind(".")+1);
			HttpForms ok(this->_socket, 200,this->keepalive, contentType, body,this->_ReqContent);
		}
		// cgi
		else
		{
			std::string script_path;
			char cwd[PATH_MAX];
			if (getcwd(cwd, sizeof(cwd)) == NULL) {
				std::cerr << "getcwd failed" << std::endl;
				return ;
			}
			script_path = std::string(cwd);
			script_path += "/cgi-bin/";
			script_path += findfrstWExtension(script_path, this->_loc.cgi_extension);

			std::map<std::string, std::string> env;
			env["REQUEST_METHOD"] = this->r_method;
			std::stringstream ss; ss << this->r_body.size();
			env["CONTENT_LENGTH"] = ss.str();
			env["SCRIPT_FILENAME"] = script_path;
			env["CONTENT_TYPE"] = this->http_params["Content-Type"];
			env["GATEWAY_INTERFACE"] = "CGI/1.1";
			env["SERVER_PROTOCOL"] = "HTTP/1.1";
			env["REDIRECT_STATUS"] = "200";


			std::string 
					cgi_output = executeCGI(script_path, this->r_method, this->r_body, env),
					contentType = "text/plain",
					body;

			std::string::size_type header_end = cgi_output.find("\r\n\r\n");
			if (header_end == std::string::npos)
				header_end = cgi_output.find("\n\n");
			if (header_end != std::string::npos)
			{
				std::string headers = cgi_output.substr(0, header_end);
				body = cgi_output.substr(header_end + 4); // skip "\r\n\r\n" && "\n\n"
				std::istringstream headerStream(headers);
				std::string line;
				while (std::getline(headerStream, line))
				{
					if (line.find("Content-Type:") != std::string::npos)
					{
						contentType = line.substr(line.find(":") + 1);
						while (contentType[0] == ' ') contentType.erase(0, 1); // trim spaces
					}
				}
			}
			else
				body = cgi_output; // no headers? treat all as body
			HttpForms ok(this->_socket, 200,this->keepalive, contentType, body,this->_ReqContent);
		}
	}
}




void Request::Delete()
{
	char buf[PATH_MAX];
	std::string f_path;
	this->_request_status = WRITING;

	if(this->location_filename.size()>this->_loc.root.size()&& getcwd(buf, sizeof(buf)))
	{
		f_path = this->location_filename;
		if(*this->location_filename.begin()=='.')
			this->location_filename.erase(0,1);
		this->location_filename = buf + this->location_filename;
	}
	else if (this->http_params.find("X-Filename") != this->http_params.end() &&
		this->http_params["X-Filename"].length() != 0 && getcwd(buf, sizeof(buf)))
	{
	    struct stat buffer;
		const std::string &full_path = std::string(buf)
				+ this->_loc.upload_store.substr(1)
				+ "/"
				+ trim(this->http_params["X-Filename"]);
		if(stat(full_path.c_str(), &buffer) != 0)
		{
			std::cerr << "\033[31m[not found]: " << full_path << "\033[0m"<< std::endl;
			HttpForms notfound(this->_socket, 404,this->keepalive,"","",this->_ReqContent);
			return;
		}else{
			std::cerr << "\033[32m[successfully found]: " << full_path << "\033[0m"<< std::endl;
			if(this->location_filename.size()>this->_loc.root.size())
				f_path = this->location_filename;
			else
				f_path = (full_path);
		}
	}
	else
	{
		HttpForms Badreq(this->_socket, 400,this->keepalive,"","",this->_ReqContent);
		return;
	}
    // Try delete the file
    if (std::remove(f_path.c_str()) == 0)
    	HttpForms ok(this->_socket,200,this->keepalive, "text/plain","success",this->_ReqContent);
    else
    	HttpForms notfound(this->_socket,404,this->keepalive,"","",this->_ReqContent);
}


std::string Request::_get_ReqContent()
{
	return this->_ReqContent;
}




Request::Request(std::string raw, const ServerConfig &servr, int socket, ssize_t bytes_received)
: _socket(socket),_server(servr),  _bytes_rec(bytes_received)
{
	this->_contlen = 0;
    this->r_header.append(raw.data(), this->_bytes_rec);
    std::istringstream iss(raw);
    std::string buffer;
    iss >> this->r_method >> this->r_location >> this->r_version;
	std::string::size_type pos = this->r_header.find("\r\n\r\n",0);
	if (pos != std::string::npos)
	{
		this->_bytes_rec -= pos + 4;
		std::string body;
		body.append(raw.data()+pos+4, this->_bytes_rec);
		this->r_body = body;

	}
	std::istringstream iss3(this->r_header.substr(0,pos));
    while (getline(iss3, buffer))
	{
        if (buffer.size() > 1)
		{
            std::string key, value;
            std::istringstream iss2(buffer);
            getline(iss2, buffer, ':');
			if(this->http_params.find(buffer)!= this->http_params.end())
			{
				HttpForms Badrequest(this->_socket, 400, 0, "","",this->_ReqContent);
				this->_request_status = WRITING;
				return ;
			}
            key = buffer;
            getline(iss2, buffer);
            value = buffer;
            this->http_params.insert(std::make_pair(key, value));
        }
    }
	if(this->http_params.find("Connection")!=this->http_params.end())
	{
		std::string connec;
		connec = this->http_params["Connection"];
		if (connec.find("keep-alive") !=std::string::npos)
			this->keepalive = true;
		else
			this->keepalive = false;
		std::cout<<"Keep:"<<this->keepalive<<std::endl;
	}
    check_allowed_methods(servr);
}
