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
Request::Request(char *raw, const ServerConfig &servr, int socket):_socket(socket)
{
	this->r_header = raw;
	std::istringstream iss(raw);
	std::string buffer;

	// std::cout << this->r_full_request << std::endl;
	iss>> this->r_method>> this->r_location >> this->r_version;

	while (getline(iss,buffer))
	{
		if(buffer.size()>1)
    	{
			std::string key, value;
			std::istringstream iss2(buffer);
			getline(iss2,buffer,':'); key = buffer;
			getline(iss2,buffer); value = buffer;
			this->http_params.insert(std::make_pair(key, value));
		}
	}
	check_allowed_methods(servr);
}



void Request::check_allowed_methods(const ServerConfig &server)
{
	//std::cout << "server" << std::endl;
	std::vector<LocationConfig>::const_iterator it_loc = server.locations.begin();
    for(;it_loc != server.locations.end();it_loc++)
    {
		//std::cout << it_loc->path << " vs " <<  this->r_location << std::endl;
		if(it_loc->path == this->r_location)
		{
			//std::cout << "found" << std::endl;
			this->_loc = *it_loc;
			std::vector<std::string>::const_iterator it_meth = it_loc->allowed_methods.begin();
			for(;it_meth != it_loc->allowed_methods.end();it_meth++)
			{
				//std::cout << "qq chose" << std::endl;
				//std::cout << *it_meth << std::endl;
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
	std::cout<<"\033[48;5;236mREQUEST = '" << this->r_location<<"' ";
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
	}else if (s == "404") // 404 not found
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
	}else
	{
		if(!this->authorized)
			return ;
		else if (this->r_method == "GET")
			this->Get();
		else if (this->r_method == "POST")
			this->Post();
	}
}


bool writeToFile( std::string& filename, const std::string& content)
{
    std::ofstream outFile(filename.c_str(),std::ios::app);  // Creates the file if it doesn't exist

    if (!outFile)
	{
		throw std::ofstream::failure("Failed to open file2");
    }
    outFile<<content;//<<"\n";
	// outFile.close();

    return true;
}
std::string extract_field_path(const std::string& buf, const std::string& field, const std::string& upload_store)
{
	std::string::size_type pos = buf.find(field);
	if (pos == std::string::npos)
		return "";

	pos += field.length();
	std::string::size_type epos = buf.find("\"", pos);
	if (epos == std::string::npos)
		return "";

	std::string result = upload_store + '/';
	if (!result.empty() && result[0] == '/')
		result.erase(result.begin());

	result += buf.substr(pos, epos - pos);
		return result;
}
//PROBLEM: ECRIT UN \n DE TROP A LA FIN DU FICHIER
void Request::writeData()
{
	bool parsestate = false;
	int counter = 0;
	if (this->r_boundary =="void")
	{
		return ;
	}
	else
	{
		std::cerr<<"R BODY header: |*"<<this->r_header<<"*| R dear END"<<std::endl;
		std::cerr<<"R BODY START: |*"<<this->r_body<<"*| R BODY END"<<std::endl;
		std::istringstream s(this->r_body);
		std::string buf, trash;
		while(getline(s,buf,'\r'))
		{
			std::cerr<<"gline buf: |"<<buf<<"|\n";
			if (buf=='\n'+this->r_boundary + "--")
			{
				// writeToFile(this->file.name, "\r\0");
				break;
			}
			else if (buf==this->r_boundary || buf=='\n'+this->r_boundary)
			{
				// writeToFile(this->file.name, "\r\0");
				parsestate = !parsestate;
			}
			else if (parsestate)
			{
				// buf+="\n";
				this->file.name = extract_field_path(buf, "name=\"", this->_loc.upload_store);
				this->file.fname = extract_field_path(buf, "filename=\"", this->_loc.upload_store);

				// std::cerr<<"file name: "<<this->file.name<<std::endl;
				// std::cerr<<"file FName: "<<this->file.fname<<std::endl;
				//GET CONTENTYPE LINE
				getline(s,buf);
				std::string::size_type pos;
				pos = buf.find("Content-Type: ");
				if (pos != std::string::npos)
					this->file.type = buf.substr(pos+14);
				//return to data mode
				parsestate = !parsestate;

				//SKIP EMPTY LINE
				getline(s,trash);
				getline(s,trash);
				std::ofstream outFile(this->file.name.c_str(),std::ios::trunc);
				// outFile.
				if (!outFile)
					throw std::ofstream::failure("Failed to open file1");
				outFile.close();
			}
			else
			{
				// std::cerr<<"buf: |"<<buf<<"|\n";
				writeToFile(this->file.name, buf + "\r");
			}

		}

	}
}
// ________________POST METHOD____________________
//PROBLEME POSSIBLE DE LOCATION
void Request::Post()
{
	std::cout<<"|thisSOCKET:"<<this->_socket <<std::endl;
	std::cout<<"POST | EXECUTED !!> \033[0m"<<std::endl;

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
	ssize_t  content_length=0;
	if(this->http_params.find("Content-Length") != this->http_params.end())
	{
		content_length = atol(this->http_params.find("Content-Length")->second.c_str());
		// std::cout<<"ctn len:" <<this->http_params.find("Content-Length")->second<<std::endl;
	}

	//EXTRACT DATA INTO THIS->R_FULL_REQUEST
	char buffer[1024];
	long bytes_received = 0;
	while (bytes_received < content_length)
	{
		ssize_t ret = recv(this->_socket, buffer, sizeof(buffer), 0);
		if (ret == 0)
			break;
		if (ret < 0)
		{
			std::cerr << "\033[31m Error receiving data from client! Socket: " << this->_socket << "\033[0m" << std::endl;
			close(this->_socket);
			return;
		}
		this->r_body.append(buffer, ret);
		bytes_received += ret;
	}
	try
	{
		this->writeData();
	}
	catch(const std::ofstream::failure& e)
	{
		std::cerr << e.what() << '\n';
	}


}

// ______________________GET METHOD____________________________
void Request::Get()
{
   std::cout << "|GET EXECUTED !!| \033[0m" << std::endl;

    std::string full_path = this->_loc.root; //+ this->r_location;
    std::string file_path;

    struct stat st;
    // std::cout << full_path << std::endl;

    if (stat(full_path.c_str(), &st) == 0) // 🛠️ REQUIRED!
    {
        if (S_ISDIR(st.st_mode) && !this->_loc.index.empty())
        {
            std::cout << "directory found" << std::endl;
            file_path = full_path + "/" + this->_loc.index;
        }
        else
        {
            std::cout << "invalid" << std::endl;
            if (this->_loc.autoindex)
                file_path = "[AUTOINDEX]";
            else
                file_path = "[403]";
        }
    }
    else
    {
        std::cerr << "stat() failed: path does not exist or access denied" << std::endl;
        file_path = "[404]";
    }


	std::cout << file_path << std::endl;
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
	}
	// 403 forbidden
	else if (file_path == "[403]")
	{
		const std::string&
				body = readFile("./www/errors/403.html"),
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

		// return file content in a sort of http kind idk
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


// ////PROBLEM: ECRIT UN \n DE TROP A LA FIN DU FICHIER
// void Request::writeData()
// {
// 	bool parsestate = false;
// 	int counter = 0;
// 	if (this->r_boundary =="void")
// 	{
// 		return ;
// 	}
// 	else
// 	{
// 		std::cerr<<"R BODY header: |*"<<this->r_header<<"*| R dear END"<<std::endl;
// 		std::cerr<<"R BODY START: |*"<<this->r_body<<"*| R BODY END"<<std::endl;
// 		std::istringstream s(this->r_body);
// 		std::string buf, trash;
// 		while(getline(s,buf))
// 		{

// 			if (buf==this->r_boundary + "--\r")
// 			{
// 				// writeToFile(this->file.name, "\r\0");
// 				break;
// 			}
// 			else if (buf==this->r_boundary+'\r')
// 			{
// 				// writeToFile(this->file.name, "\r\0");
// 				parsestate = !parsestate;
// 			}
// 			else if (parsestate)
// 			{
// 				// buf+="\n";
// 				this->file.name = extract_field_path(buf, "name=\"", this->_loc.upload_store);
// 				this->file.fname = extract_field_path(buf, "filename=\"", this->_loc.upload_store);

// 				std::cerr<<"file name: "<<this->file.name<<std::endl;
// 				std::cerr<<"file FName: "<<this->file.fname<<std::endl;
// 				//GET CONTENTYPE LINE
// 				getline(s,buf);
// 				std::string::size_type pos;
// 				pos = buf.find("Content-Type: ");
// 				if (pos != std::string::npos)
// 					this->file.type = buf.substr(pos+14);
// 				//return to data mode
// 				parsestate = !parsestate;

// 				//SKIP EMPTY LINE
// 				getline(s,trash);
// 				std::ofstream outFile(this->file.name.c_str(),std::ios::trunc);
// 				// outFile.
// 				if (!outFile)
// 					throw std::ofstream::failure("Failed to open file1");
// 				outFile.close();
// 			}
// 			else
// 			{
// 				std::cerr<<"\n\nbuf: |"<<buf<<"|\n";
// 				writeToFile(this->file.name, buf);
// 			}

// 		}

// 	}
// }