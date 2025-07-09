#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include "request.hpp"
#include "webserv.hpp"

inline std::string readFile(const std::string& file_path)
{
    std::ifstream file(file_path.c_str());
    if (!file.is_open())
        return ""; // cant open file

    std::stringstream buffer;
    buffer << file.rdbuf();  // Read entire file contents into buffer
    return buffer.str();     // Return as a std::string
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
inline bool handle_client(int client_socket,  ServerConfig &serv)
{
    char buffer[2048];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    // std::cerr<<"BUFFER:\n"<<buffer<<"\n|ENDOFBUFFER"<<std::endl;

    if (bytes_received < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            std::cerr<<"HANDLE clientsocket pollin: "<<client_socket<<"\n";
            return false;
        }
        std::cerr << "\033[31m[x] recv() error on client " << client_socket << ": " << strerror(errno) << "\033[0m\n";
        return true; // done with this socket (error, cleanup)
    }
    else if (bytes_received == 0)
    {
        std::cerr << "\033[33m[~] Client disconnected: " << client_socket << "\033[0m\n";
        return true;
    }

    // log  received HTTP request
    Request R(buffer, serv, client_socket, bytes_received);

    std::string response = R._get_ReqContent();
    ssize_t sent = send(client_socket, response.c_str(), response.size(), 0);
    if (sent < 0)
    {
        std::cerr << "\033[31m[x] send() failed: " << strerror(errno) << "\033[0m\n";
    }
    if (!R.keepalive)
    {
        return true;
    }
    return false;
}

// template <typename K, typename V>
// bool contientValeur(const std::map<K, V>& maMap, const V& valeurRecherchee) {
//     typename std::map<K, V>::const_iterator it;
//     for (it = maMap.begin(); it != maMap.end(); ++it) {
//         if (it->second == valeurRecherchee) {
//             return true;
//         }
//     }
//     return false;
// }
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


#endif