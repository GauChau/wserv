#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include "request.hpp"
#include "webserv.hpp"
#include <csignal>


inline std::string nonblocking_readcgi(const std::string& file_path, int input_fd = -1, int pid =-1)
{
    int fd;
    if(input_fd == - 1){
     fd = open(file_path.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd < 0)
            return "";  // Can't open file
    }else
    {
        fd = input_fd;
    }

    std::string content;
    char buffer[4096];
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    ssize_t bytes_read;
    do
    {
        int ret = poll(&pfd, 1, 4000);  // 1 second timeout
        if (ret < 0) {
            close(fd);
            return "";
        } else if (ret == 0) 
            {
                close(fd);
                kill(pid,SIGTERM);
            break ;
        }
        

        if (pfd.revents & POLLIN)
        {
            std::cerr<<"retardboucle\n";

            bytes_read = read(fd, buffer, sizeof(buffer));
            if (bytes_read > 0)
                content.append(buffer, bytes_read);
            else if (bytes_read == 0) 
                {close(fd);break;}
            else {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    continue; // try again
                close(fd);
                return "";
            }
        }
    }while(bytes_read != 0 && (pfd.revents & POLLIN));

    close(fd);
    return content;
}


inline std::string nonblocking_read(const std::string& file_path, int input_fd = -1)
{
    int fd;
    if(input_fd == - 1){
     fd = open(file_path.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd < 0)
            return "";  // Can't open file
    }else
    {
        fd = input_fd;
    }

    std::string content;
    char buffer[4096];
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    ssize_t bytes_read;
    do
    {
        int ret = poll(&pfd, 1, 1000);  // 1 second timeout
        if (ret < 0) {
            close(fd);
            return "";
        } else if (ret == 0) 
            break ;
        

        if (pfd.revents & POLLIN)
        {
            bytes_read = read(fd, buffer, sizeof(buffer));
            if (bytes_read > 0)
                content.append(buffer, bytes_read);
            else if (bytes_read == 0) 
                break;
            else {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    continue; // try again
                close(fd);
                return "";
            }
        }

    }while(bytes_read != 0);

    close(fd);
    return content;
}

inline ssize_t nonblocking_recv(int socket_fd, char *buffer, unsigned int n)
{
    (void)(n);
    struct pollfd pfd;
    pfd.fd = (socket_fd);
    pfd.events = POLLIN;


    int ret = poll(&pfd, 1, 200);  // 0.2 second timeout
    if (ret < 0)
    {
        return -1;
    }
    if (pfd.revents & POLLIN)
    {
        return recv(socket_fd, buffer, sizeof(buffer), 0);
    }

    return -1;
}
inline ssize_t nonblocking_send(int socket_fd, const void *s, unsigned int size)
{
    struct pollfd pfd;
    pfd.fd = (socket_fd);
    pfd.events = POLLOUT;


    int ret = poll(&pfd, 1, 200);  // 0.2 second timeout
    if (ret < 0)
    {
        return -1;
    }
    if (pfd.revents & POLLOUT)
    {
        return send(socket_fd, s, size, 0);
    }

    return (-1);
}

inline ssize_t nonblocking_write(std::string file_path, const void *s, unsigned int size)
{
    int socket_fd = open(file_path.c_str(), O_WRONLY | O_NONBLOCK | O_CREAT | O_APPEND,0644);
    if (socket_fd < 0)
        return -1;  // Can't open file
    struct pollfd pfd;
    pfd.fd = (socket_fd);
    pfd.events = POLLOUT;

    int ret = poll(&pfd, 1, 200);  // 0.2 second timeout
    if (ret < 0)
    {
        return -1;
    }
    if (pfd.revents & POLLOUT)
    {
        ssize_t q = write(socket_fd, s, size);
        close(socket_fd);
        return(q);
    }
    return (-1);
}


inline std::string extract_field_path(const std::string& buf, const std::string& field)
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
inline std::string sanitize_filename(const std::string& filename)
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


inline std::string findfrstWExtension(const std::string& dirPath, const std::string& ext)
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


inline std::string trim(const std::string& str)
{
    const std::string whitespace = " \t\n\r";
    const size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    const size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

// std::map<string, string> -> char*[] envp
inline char** buildEnvp(std::map<std::string, std::string>& env)
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
#define MAX_HEADER_SIZE 8192  // 8 KB
inline bool ULTIMATEFUCKERY(int client_socket,  ServerConfig &serv)
{
    char buffer[2048];
    std::string chunky="", response;
    ssize_t 
        totalrec=0,
        bytes_received = 0;
    bool joe = false;
    do
    {
        if(!joe)
            bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        else
        {
            joe=true;    
            bytes_received = nonblocking_recv(client_socket, buffer, sizeof(buffer));
        }

        totalrec += bytes_received;
        if (bytes_received < 0)
        {
            std::cerr << "\033[31m[x] recv() error on client " << client_socket << ": " << strerror(errno) << "\033[0m\n";
            return true; // done with this socket (error, cleanup)
        }
        else if (bytes_received == 0)
        {
            std::cerr << "\033[33m[~] Client disconnected: " << client_socket << "\033[0m\n";
            return true;
        }
        chunky.append(buffer,bytes_received);
    } while (chunky.find("\r\n\r\n") == std::string::npos);
    

    // log  received HTTP request
    Request R(buffer, serv, client_socket, totalrec);

    response = R._get_ReqContent();
    size_t sent=0, total=0;
    while (total <response.size() && sent >=0)
    {
        sent = nonblocking_send(client_socket, response.data() + total, response.size() - total);
        total += sent;
    }
    if (sent < 0)
    {
        std::cerr << "\033[31m[x] send() failed: " << strerror(errno) << "\033[0m\n";
        return true;
    }
    if (!R.keepalive)
    {
        return true;
    }
    return false;
}

// - exec CGI script in a fork()
// - returns CGI stdout in a std::str
inline std::string executeCGI(const std::string& scriptPath, const std::string& method, const std::string& body, std::map<std::string, std::string> env)
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
 
        fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);  
        std::string ah= nonblocking_readcgi("", pipe_out[0],pid);
        // kill(pid, SIGINT);
        // waitpid(pid, &status, 0);
        // close(pipe_out[1]);
        return ah;
    }
}


inline bool is_directory(const char* path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return false; // path doesn't exist or error
    return S_ISDIR(statbuf.st_mode);
}

inline bool is_proper_prefix(const std::string& uri, const std::string& loc)
{
    if (uri.compare(0, loc.size(), loc) != 0)
        return false;

    if (uri.size() == loc.size())
        return true;

    if (uri[loc.size()] == '/')
        return true;

    return false;
}

inline bool match_location(const std::string& uri,const std::vector<LocationConfig> &locations, LocationConfig &_loc_target)
{
	bool found=false;
    std::string best_match = "";
	std::vector<LocationConfig>::const_iterator it_loc = locations.begin();
    for(;it_loc != locations.end();it_loc++)
	{
        if (is_proper_prefix(uri, it_loc->path)) {
            if (it_loc->path.length() > best_match.length()) {
                best_match = it_loc->path;
				_loc_target = *it_loc;
				found=true;
            }
        }
    }
    return found;
}

#endif