#include "request.hpp"
#include "HttpForms.hpp"

static std::string readFile(const std::string& file_path)
{
    std::ifstream file(file_path.c_str());
    if (!file.is_open())
        return ""; // cant open file

    std::stringstream buffer;
    buffer << file.rdbuf();  // Read entire file contents into buffer
    return buffer.str();     // Return as a std::string
}

static std::string extract_field_path(const std::string& buf, const std::string& field)
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


static std::string findfrstWExtension(const std::string& dirPath, const std::string& ext)
{
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        std::cerr << "Failed to open directory: " << dirPath << std::endl;
        return "";
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string filename(entry->d_name);
        // skip "." and ".."
        if (filename == "." || filename == "..")
            continue;
        if (filename.size() >= ext.size() &&
            filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0)
        {
            closedir(dir);
            return filename;
        }
    }
    closedir(dir);
    return "";
}

Request::~Request()
{ }

Request::Request(char *raw, const ServerConfig &servr, int socket, ssize_t bytes_received)
    : _server(servr), _socket(socket), _bytes_rec(bytes_received)
{
    this->r_header.append(raw, this->_bytes_rec);
    std::istringstream iss(raw);
    std::string buffer;
    iss >> this->r_method >> this->r_location >> this->r_version;

	std::string::size_type pos = this->r_header.find("\r\n\r\n",0);
	if (pos != std::string::npos)
	{
		this->_bytes_rec -= pos + 4;
		std::string body;
		body.append(raw+pos+4, this->_bytes_rec);
		this->r_body = body;
	}

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

		bodi += "<p>" + this->_loc.index + " only handles: </p>";

		for(unsigned long i = 0; i < this->_loc.allowed_methods.size(); i++)
			bodi += "<p> - " + this->_loc.allowed_methods[i] + " Method</p>";

		HttpForms exec405(this->_socket,405,contentType,bodi,this->_ReqContent);
	}else
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
		std::istringstream s(this->r_body, std::ios::binary);
		std::string buf;
		while(getline(s,buf))
		{

			if (buf==this->r_boundary + "--\r"){
				break;}
			else if (buf==this->r_boundary+'\r')
			{
				parsestate = !parsestate;
			}
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
				std::ofstream outFile(full_path.c_str(), std::ios::trunc | std::ios::binary);
				if (!outFile)
					throw std::ofstream::failure("aFailed to open file");
			}
			else
			{

				std::string& filename = this->file.name;
				const std::string& content = buf+'\n';
				std::ofstream outFile(filename.c_str(),std::ios::app | std::ios::binary);  // Creates the file if it doesn't exist
				if (!outFile)
					throw std::ofstream::failure("bFailed to open file");
				outFile<<content;
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
		if (*this->r_boundary.rbegin()==' ')
			this->r_boundary.resize(this->r_boundary.size()-1);

    }
	else
		this->r_boundary = "void";

	//calculate data length
	ssize_t  content_length = 0;
	if (this->http_params.find("Content-Length") != this->http_params.end())
	{
		content_length = atol(this->http_params["Content-Length"].c_str());
		if (content_length > 10 * 1024 * 1024) { // 10 MB limit
			HttpForms toolarge(this->_socket,413);
			toolarge._sendclose();
			return;
		}
	}


	//EXTRACT DATA INTO THIS->R_FULL_REQUEST
	char buffer[2048];
	while (this->_bytes_rec < content_length)
	{
		ssize_t ret = recv(this->_socket, buffer, sizeof(buffer), 0);
		if (ret == 0)
			break;
		if (ret < 0)
		{
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				continue; // retry the recv
			else {
				std::cerr << "\033[31m [x] post fatal recv err socket: "
						  << this->_socket << " (" << strerror(errno) << ")\033[0m\n";
				HttpForms ServError(this->_socket, 500,"text/plain","Failed to receive POST data.\r\n");
				ServError._sendclose();
				return;
			}
		}
		this->ret = ret;
		this->r_body.append(buffer, ret);
		this->_bytes_rec += ret;
	}
	try
	{
		this->writeData();
		HttpForms ok(this->_socket, 200);
		ok._send();
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

// std::map<string, string> -> char*[] envp
static char** buildEnvp(std::map<std::string, std::string>& env)
{
    char** envp = new char*[env.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it)
	{
        std::string entry = it->first + "=" + it->second;
        envp[i] = new char[entry.size() + 1];
        std::strcpy(envp[i], entry.c_str());
        i++;
    }
    envp[i] = NULL;
    return envp;
}

// - exec CGI script in a fork()
// - returns CGI stdout in a std::str
static std::string executeCGI(const std::string& scriptPath, const std::string& method, const std::string& body, std::map<std::string, std::string> env)
{
    int pipe_out[2];
    int pipe_in[2];

    if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1) {
        perror("pipe");
        return "status: 500\r\n\r\nInternal Server Error";
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return "status: 500\r\n\r\nfork Failed";
    }
	std::cout << "running " << scriptPath << "..." << std::endl;
	// c
    if (pid == 0)
	{
        // Child
        dup2(pipe_in[0], STDIN_FILENO);	dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[1]);	close(pipe_out[0]);

        char* argv[] = { (char*)scriptPath.c_str(), NULL };
        char** envp = buildEnvp(env);

        execve(scriptPath.c_str(), argv, envp);
        perror("execve");
        exit(1);
    }
	// p
	else
	{
        close(pipe_in[0]);
        close(pipe_out[1]);
        if (method == "POST" && !body.empty())
            write(pipe_in[1], body.c_str(), body.size());
        close(pipe_in[1]);

        char buffer[4096];
        std::string output;
        ssize_t r;
        while ((r = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
            output.append(buffer, r);

        close(pipe_out[0]);
        int status;
        waitpid(pid, &status, 0);
        return output;
    }
}



// ______________________GET METHOD____________________________
void Request::Get()
{
	// std::cout << "|GET EXECUTED !!| \033[0m" << std::endl;
    std::string full_path = this->_loc.root; //+ this->r_location;
    std::string file_path;

    struct stat st;

    if (stat(full_path.c_str(), &st) == 0) // 🛠️ REQUIRED!
    {
        if (S_ISDIR(st.st_mode) && (!this->_loc.index.empty() ||
			((&(this->_loc.cgi_extension) != NULL && !this->_loc.cgi_extension.empty())) ))
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
		HttpForms notfound(this->_socket, 200,"text/html", body,this->_ReqContent);
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
		HttpForms notfound(this->_socket, 404,contentType, body,this->_ReqContent);
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
				// std::cout << "Found 403 page for " << it->first << ": " << it->second << std::endl;
				path_403 = it->second;
				break;
			}
		}
		const std::string&
				body = readFile(path_403),
				contentType = "text/html";
		HttpForms forbid(this->_socket, 403, contentType, body,this->_ReqContent);
	}
	else if (file_path == "[REDIRECTION]")
	{
		// todo
		// handle redirection
	}
	else
	{
		// cgi or default
		if(&(this->_loc.cgi_extension) == NULL || this->_loc.cgi_extension.empty())
		{
			const std::string&
				body = readFile(file_path),
				contentType = "text/html";
			HttpForms ok(this->_socket, 200, contentType, body,this->_ReqContent);
		}else
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


			std::cout << script_path << std::endl;
			std::map<std::string, std::string> env;
			env["REQUEST_METHOD"] = this->r_method;
			std::stringstream ss; ss << this->r_body.size();
			env["CONTENT_LENGTH"] = ss.str();
			env["SCRIPT_FILENAME"] = script_path;
			// env["QUERY_STRING"] = this->r_query;
			env["CONTENT_TYPE"] = this->http_params["Content-Type"];
			env["GATEWAY_INTERFACE"] = "CGI/1.1";
			env["SERVER_PROTOCOL"] = "HTTP/1.1";
			env["REDIRECT_STATUS"] = "200";


			std::string cgi_output = executeCGI(script_path, this->r_method, this->r_body, env);
			std::string contentType = "text/plain";
			std::string body;

			// extract content-type from CGI output
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

			HttpForms ok(this->_socket, 200, contentType, body,this->_ReqContent);
			std::cout << this->_ReqContent << std::endl;
		}
	}
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
			std::cerr << "\033[31m[not found]: " << full_path << "\033[0m"<< std::endl;
			HttpForms notfound(this->_socket, 404);
			notfound._send();
			return;
		}else{
			std::cerr << "\033[32m[successfully found]: " << full_path << "\033[0m"<< std::endl;
			f_path = (full_path);
		}
	}else
	{
		HttpForms Badreq(this->_socket, 400);
		Badreq._sendclose();
		return;
	}

    // Try to delete the file
    if (std::remove(f_path.c_str()) == 0)
    {
		HttpForms ok(this->_socket,200, "text/plain","success");
		ok._send();
		return;
    }
    else
    {
        // file deletion failed, send 404 or 403
		HttpForms notfound(this->_socket,404);
		notfound._send();
    }
    close(this->_socket);
}


std::string Request::_get_ReqContent()
{
	return this->_ReqContent;
}
// ______________________________________________

