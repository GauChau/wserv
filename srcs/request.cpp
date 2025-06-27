#include "request.hpp"

Request::~Request()
{ }

static std::string readFile(const std::string& file_path)
{
    std::ifstream file(file_path.c_str());
    if (!file.is_open())
        return ""; // cant open file

    std::stringstream buffer;
    buffer << file.rdbuf();  // Read entire file contents into buffer
    return buffer.str();     // Return as a std::string
} 
Request::Request(char *raw, const ServerConfig &servr, int socket)
    : _server(servr), _socket(socket)
{
    this->r_header = raw;
    std::istringstream iss(raw);
    std::string buffer;

    iss >> this->r_method >> this->r_location >> this->r_version;

    while (getline(iss, buffer)) {
        if (buffer.size() > 1) {
            std::string key, value;
            std::istringstream iss2(buffer);
            getline(iss2, buffer, ':');
            key = buffer;
            getline(iss2, buffer);
            value = buffer;
            this->http_params.insert(std::make_pair(key, value));
        }
    }
    check_allowed_methods(servr);
}



void Request::check_allowed_methods(const ServerConfig &server)
{
	std::vector<LocationConfig>::const_iterator it_loc = server.locations.begin();
    for(;it_loc != server.locations.end();it_loc++)
    {
		if(it_loc->path == this->r_location)
		{
			this->_loc = *it_loc;
			std::vector<std::string>::const_iterator it_meth = it_loc->allowed_methods.begin();
			for(;it_meth != it_loc->allowed_methods.end();it_meth++)
			{
			    if(this->r_method == *it_meth)
			    {
			        this->authorized = true;
                    this->execute(""); // <--- then execute it
					return ;
			    }
			}
			// 405
			this->execute("405"); // <--- then execute it
			return ;
		}
    }
	// 404 
	this->execute("404"); // <--- then execute it
}
// ________________EXECUTE METHOD____________________
void Request::execute(std::string s = "null")
{
	// std::cout<<"\033[48;5;236mREQUEST = '" << this->r_location<<"' ";
	if(s == "405") // 405 unallowed method
	{
		const std::string& 
				body = readFile("./www/errors/405.html"),
				contentType = "text/html";

		std::string bodi = body;
		// Append extra info to body before calculating Content-Length
		bodi += "<h2>" + this->_loc.root + " only handles:</h2>";
		for (size_t i = 0; i < this->_loc.allowed_methods.size(); ++i)
			bodi += "<h2> - " + this->_loc.allowed_methods[i] + " Method</h2>";
		std::stringstream response;
		response << "HTTP/1.1 405 Method Not Alloweds\r\n";
		response << "Content-Type: " << contentType << "\r\n";
		response << "Content-Length: " << bodi.size() << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n"; // End of headers
		response << bodi;
		response << "<p>"<<this->_loc.index + " only handles: </p>";
		for(unsigned long i = 0; i < this->_loc.allowed_methods.size(); i++)
			response << "<p> - " + this->_loc.allowed_methods[i] + " Method</p>";

		this->_ReqContent = ( response.str());
	}/*else if (s == "404") // 404 not found
	{
		// render custom 404 page
		const std::string& 
				body = readFile("./www/errors/404.html"),
				contentType = "text/html";
		std::stringstream response;
		response << "HTTP/1.1 404 Not Found\r\n";
		response << "Content-Type: " << contentType << "\r\n";
		response << "Content-Length: " << body.size() << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n"; // End of headers
		response << body;
		this->_ReqContent = ( response.str());
	}*/else
	{
		if(!this->authorized)
			return ;
		else if (this->r_method == "GET")
			this->Get();
		else if (this->r_method == "POST")
			this->Post();
		else if (this->r_method == "DELETE")
			this->Delete();		
	}
}

std::string extract_field_path(const std::string& buf, const std::string& field)
{
	std::string::size_type pos = buf.find(field);
	if (pos == std::string::npos)
		return "";

	pos += field.length();
	std::string::size_type epos = buf.find("\"", pos);
	if (epos == std::string::npos)
		return "";

	std::string result = "/";
	if (!result.empty() && result[0] == '/')
		result.erase(result.begin());

	result += buf.substr(pos, epos - pos);
		return result;
}

// sanitize_filename  eviter:  ../../../etc/passwd 
static std::string sanitize_filename(const std::string& filename) 
{
	std::string clean;
    for (size_t i = 0; i < filename.size(); ++i) {
        char c = filename[i];
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-') {
            clean += c;
        }
    }
    if (clean.empty()) clean = "upload.bin";
    return clean;
}

//PROBLEM: ECRIT UN \n DE TROP A LA FIN DU FICHIER
void	Request::writeData()
{
	bool parsestate = false;
	if (this->r_boundary =="void")
	{
		return;
	}
	else
	{
		std::istringstream s(this->r_body);
		std::string buf;
		while(getline(s,buf))
		{
			
			if (buf==this->r_boundary + "--\r")
				break;
			else if (buf==this->r_boundary+'\r')
				parsestate = !parsestate;
			else if (parsestate)
			{
				this->file.fname = extract_field_path(buf, "filename=\"");

				//GET CONTENTYPE LINE
				getline(s,buf);
				std::string::size_type pos;
				pos = buf.find("Content-Type: ");
				if (pos != std::string::npos)
					this->file.type = buf.substr(pos+14);
				//return to data mode
				parsestate = !parsestate;

				//SKIP EMPTY LINE
				getline(s,buf);
				std::string safe_name = sanitize_filename(this->file.fname);
				std::string full_path = this->_loc.upload_store + "/" + safe_name;
				this->file.name = full_path;

				std::ofstream outFile(full_path.c_str(), std::ios::trunc);
				// std::ofstream outFile(this->file.name.c_str(),std::ios::trunc);

				// outFile.
				if (!outFile)
					throw std::ofstream::failure("Failed to open file");
			}
			else
			{
				
				std::string& filename = this->file.name;
				const std::string& content = buf+'\n';
				
				std::ofstream outFile(filename.c_str(),std::ios::app);  // Creates the file if it doesn't exist
				if (!outFile)
				{
					throw std::ofstream::failure("Failed to open file");
				}
				outFile<< content;//<<"\n";
				// writeToFile(this->file.name, buf + \n);
			}

		}
	}
}
// ________________POST METHOD____________________
//PROBLEME POSSIBLE DE LOCATION
void Request::Post()
{
	//EXTRACT BOUNDARY
	std::string::size_type pos = this->http_params.find("Content-Type")->second.find("boundary=");
    if (pos != std::string::npos)
	{
        this->r_boundary = this->http_params.find("Content-Type")->second.substr(pos+9);
		this->r_boundary.resize(this->r_boundary.size()-1);
		this->r_boundary = "--" + this->r_boundary;
    }
	else
		this->r_boundary = "void";

	//calculate data length
	ssize_t  content_length = 0;
	if (this->http_params.find("Content-Length") != this->http_params.end())
	{
		content_length = atol(this->http_params["Content-Length"].c_str());
		if (content_length > 10 * 1024 * 1024) { // 10 MB limit
			std::string res =
				"HTTP/1.1 413 Payload Too Large\r\n"
				"Content-Length: 0\r\n\r\n";
			send(this->_socket, res.c_str(), res.size(), 0);
			close(this->_socket);
			return;
		}
	}


	//EXTRACT DATA INTO THIS->R_FULL_REQUEST
	char buffer[2048];
	long bytes_received = 0;
	while (bytes_received < content_length)
	{
		ssize_t ret = recv(this->_socket, buffer, sizeof(buffer), 0);
		if (ret == 0)
			break;
		if (ret < 0)
		{
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				continue; // retry the recv
			else {
				std::cerr << "\033[31m [x] post fatal recv err socket: " << this->_socket << " (" << strerror(errno) << ")\033[0m\n";

				std::string response =
					"HTTP/1.1 500 Internal Server Error\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Length: 30\r\n"
					"\r\n"
					"Failed to receive POST data.\n";

				send(this->_socket, response.c_str(), response.size(), 0);
				close(this->_socket);
				return;
			}
		}
		this->r_body.append(buffer, ret);
		bytes_received += ret;
	}
	try
	{
		this->writeData();
		std::string res =
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: 0\r\n"
			"Connection: close\r\n"
			"\r\n";
		send(this->_socket, res.c_str(), res.size(), 0);

		std::cout << "\033[32m[✓] POST request handled successfully!\033[0m" << std::endl;
	}
	catch(const std::ofstream::failure& e)
	{
		std::cerr << e.what() << '\n';
	}
}

static std::string trim(const std::string& str)
{
    const std::string whitespace = " \t\n\r";
    const size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    const size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

void Request::Delete()
{
	// make sure that delete only runs into the upload/ path
	char buf[PATH_MAX];
	std::string f_path;
	if (this->http_params.find("X-Filename") != this->http_params.end() &&
		this->http_params["X-Filename"].length() != 0 && getcwd(buf, sizeof(buf)))
	{
	    struct stat buffer;
		const std::string &full_path = std::string(buf) 
				// + this->_loc.root.substr(1)
				+ this->_loc.upload_store.substr(1)
				+ "/"
				+ trim(this->http_params["X-Filename"]);
		if(stat(full_path.c_str(), &buffer) != 0)
		{
			std::string response =
				"HTTP/1.1 404 Not Found\r\n"
				"Content-Length: 0\r\n"
				"Connection: close\r\n\r\n";
			std::cout << "\033[31m[not found]: " << full_path << "\033[0m"<< std::endl;
			send(this->_socket, response.c_str(), response.size(), 0);
			return;
		}else{
			std::cout << "\033[32m[successfully found]: " << full_path << "\033[0m"<< std::endl;
			f_path = (full_path);
		}
	}else
	{
		std::string response = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send(this->_socket, response.c_str(), response.size(), 0);
    	close(this->_socket);
		return;	
	}
    // e.g., "/uploads/file.txt" or "/index.html"
	// std::string root = this->_loc.root;   // e.g. "/var/www/ur_site"
    // std::string path = this->_loc.upload_store;  // e.g. "/uploads/myfle.txt"

    // // prevent path traversal attacks (../)
    // // could do some sanitization here:
    // if (path.find("..") != std::string::npos)
    // {
    //     // return some error indicator, or sanitize path
	// 	std::string response = 
    //         "HTTP/1.1 404 Not Found\r\n"
    //         "Content-Length: 0\r\n"
    //         "Connection: close\r\n\r\n";
    //     send(this->_socket, response.c_str(), response.size(), 0);
    // 	close(this->_socket);
	// 	return;
	// }

    // // force root to nd w a slash
	// if (!root.empty() && root[root.size() - 1] != '/') 
    //    root += "/";

    // // rmv leading slash path to avoid dbl slash
    // if (!path.empty() && path[0] == '/')
    //     path.erase(0, 1);

    // std::string full_path = root + path; // "/var/www/uploads/myfile.txt"

    // Try to delete the file
    if (std::remove(f_path.c_str()) == 0)
    {
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send(this->_socket, response.c_str(), response.size(), 0);
		return;
    }
    else
    {
        // file deletion failed, send 404 or 403
        std::string response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send(this->_socket, response.c_str(), response.size(), 0);
    }
    close(this->_socket);
}


// ______________________GET METHOD____________________________
void Request::Get()
{
	// std::cout << "|GET EXECUTED !!| \033[0m" << std::endl;
    std::string full_path = this->_loc.root; //+ this->r_location;
    std::string file_path;

    struct stat st;
    // std::cout << full_path << std::endl;

    if (stat(full_path.c_str(), &st) == 0) // 🛠️ REQUIRED!
    {
        if (S_ISDIR(st.st_mode) && !this->_loc.index.empty())
        {
            file_path = full_path + "/" + this->_loc.index;
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


	// std::cout << file_path << std::endl;
    //  autoindex
    if (file_path == "[AUTOINDEX]" )
    {
		// Generate directory listing
		std::stringstream listing;
		DIR* dir = opendir(full_path.c_str());
		if (!dir) {
			listing << "<li><em>Directory not found: " << full_path << "</em></li>";
		} else {
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL) {
				std::string name = entry->d_name;
				if (name == "." || name == "..") continue;
				listing << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
			}
			closedir(dir);
		}

		std::string body = readFile("./www/errors/autoindex.html");
		size_t pos = body.find("<!--CONTENT-->");
		if (pos != std::string::npos) {
			body.replace(pos, std::string("<!--CONTENT-->").length(), listing.str());
		}
		std::stringstream response;
		response << "HTTP/1.1 200 Ok\r\n";
		response << "Content-Type: text/html\r\n";
		response << "Content-Length: " << body.size() << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n";
		response << body;
		this->_ReqContent = response.str();
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
				// std::cout << "Found 404 page for " << it->first << ": " << it->second << std::endl;
				path_404 = it->second;
				break;
			}
		}
		const std::string& 
				body = readFile(path_404),
				contentType = "text/html";
		std::stringstream response;
		response << "HTTP/1.1 404 Not Found\r\n";
		response << "Content-Type: " << contentType << "\r\n";
		response << "Content-Length: " << body.size() << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n"; // End of headers
		response << body;
		this->_ReqContent = ( response.str());
	}
	// 403 forbidden
	else if (file_path == "[403]")
	{
		/*
		std::string path_403 = "./www/default/403.html";
		std::vector<std::pair<unsigned int, std::string> >::const_iterator it;
		it = this->_server.error_pages.begin();
		for (; it != this->_server.error_pages.end(); ++it)
		{
			if (it->first == 403 || it->first == 404)
			{
				std::cout << "Found error page for " << it->first << ": " << it->second << std::endl;
				path_403 = it->second;
				break;
			}
		}
		const std::string& 
				body = readFile(path_403),
				contentType = "text/html";
		*/
		std::string path_403 = "./www/default/403.html";
		std::vector<std::pair<unsigned int, std::string> >::const_iterator it;
		it = this->_server.error_pages.begin();
		for (; it != this->_server.error_pages.end(); ++it)
		{
			if (it->first == 403)
			{
				// std::cout << "Found 403 page for " << it->first << ": " << it->second << std::endl;
				path_403 = it->second;
				break;
			}
		}
		const std::string& 
				body = readFile(path_403),
				contentType = "text/html";
		std::stringstream response;
		response << "HTTP/1.1 403 Forbidden\r\n";
		response << "Content-Type: " << contentType << "\r\n";
		response << "Content-Length: " << body.size() << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n"; // End of headers
		response << body;
		this->_ReqContent = ( response.str());
	}
	else if (file_path == "[REDIRECTION]")
	{
		// todo
		// handle redirection
		
	}
	else
	{
		const std::string& 
			body = readFile(file_path),
			contentType = "text/html";

		std::stringstream response;
		response << "HTTP/1.1 200 OK\r\n";
		response << "Content-Type: " << contentType << "\r\n";
		response << "Content-Length: " << body.size() << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n"; // End of headers

		response << body;
		this->_ReqContent = ( response.str());
	}
	// std::cout << "\033[1;48;5;236m"<< this->_ReqContent << "\033[0m"<< std::endl;
}

std::string Request::_get_ReqContent()
{
	return this->_ReqContent;
}
// ______________________________________________
