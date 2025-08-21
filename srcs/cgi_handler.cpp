#include "cgi_handler.hpp"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include "webserv.hpp"
#include "client.hpp"
#include "HttpForms.hpp"

CGIHandler::CGIHandler(client* client)
    : _client(client), _fd(-1), _pid(-1), _finished(false) {registered = 0; }

CGIHandler::~CGIHandler()
{
    std::cerr<<" deletecgiclass ["<<_fd<<"] ";
    
    
    this->_client->cgi_handler = NULL;
    this->_client->cgi_fd=-1;
    this->_client->cgiresgitered =false;
    if (_fd != -1) close(_fd);
}

void CGIHandler::setEnv(const std::map<std::string, std::string>& env) { _env = env; }
void CGIHandler::setScriptPath(const std::string& path) { _scriptPath = path; }
void CGIHandler::setRequestBody(const std::string& body) { _requestBody = body; }

int CGIHandler::launch() 
{
    int pipe_out[2];
    int pipe_in[2];

    std::cerr<<" launch cgi handler for client: "<<_client->getFd()<<" script: "<<_scriptPath;

    if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(pipe_out[0]);
        close(pipe_out[1]);
        close(pipe_in[0]);
        close(pipe_in[1]);
        return -1;
    }
    std::cout << " " << this->_scriptPath << "..." << std::endl;
    if (pid == 0)
    {
        std::cerr<<" cgi child launched for client: "<<_client->getFd()<<" script: "<<_scriptPath;
        // Child
        if (this->_client->_request->getRmethod() == "POST")
        {
            int fd = open(this->_client->_request->getfileName().c_str(), O_RDONLY); // ou chemin dynamique
            if (fd < 0)
            {
                perror("open");
                exit(1);
            }

            dup2(fd, STDIN_FILENO);
            close(fd); // après dup2
            dup2(pipe_out[1], STDOUT_FILENO);
            close(pipe_in[0]);
            close(pipe_out[1]);
        }
        else 
        {
            dup2(pipe_in[0], STDIN_FILENO);
            dup2(pipe_out[1], STDOUT_FILENO);
            close(pipe_in[1]);close(pipe_out[0]);
        }

        char* argv[] = { (char*)this->_scriptPath.c_str(), NULL };
        char **envp = buildEnvArray();
        execve(_scriptPath.c_str(), argv, envp);
        perror("execve");
        exit(0);
    }
    else
    {

        std::cerr<<" cgi parent launched for client: "<<_client->getFd()<<" script: "<<_scriptPath;
        close(pipe_in[0]);
        close(pipe_out[1]);
        // if (this->r_method == "POST" && !this->r_body.empty())
        //     write(pipe_in[1], this->r_body.c_str(),this->r_body.size());
        close(pipe_in[1]);

        fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);
        std::cerr<<" pipeout0:"<<pipe_out[0];
        // exit(0);
        this->_fd = pipe_out[0];
        return pipe_out[0]; // fd à surveiller dans poll
    }
}

bool CGIHandler::readOutput()
{
    if (_fd == -1)
        return false;

    char buffer[4096];
    ssize_t bytes_read = read(_fd, buffer, sizeof(buffer));
    if (bytes_read > 0) 
    {
        // std::cerr<<" cgi read output for client: "<<_client->getFd()<<" bytes read: "<<bytes_read;
        _buffer.append(buffer, bytes_read);
        return false; // Pas fini
    }

    if (bytes_read == 0) // EOF : CGI terminé
    {
        // std::cerr<<" cgi read output for client: "<<_client->getFd()<<" EOF reached";
        std::cerr<<" joe ";
        close(_fd);
        _fd = -1;
        _finished = true;

        // Parser headers/body
        std::string::size_type header_end = _buffer.find("\r\n\r\n");
        if (header_end == std::string::npos)
            header_end = _buffer.find("\n\n");
        std::string contentType = "text/plain";
        std::string body;
        if (header_end != std::string::npos)
        {
            std::cerr<<" EOFB ";
            std::string headers = _buffer.substr(0, header_end);
            body = _buffer.substr(header_end + 4);
            std::istringstream headerStream(headers);
            std::string line;
            while (std::getline(headerStream, line))
            {
                if (line.find("Content-Type:") != std::string::npos)
                {
                    contentType = line.substr(line.find(":") + 1);
                    while (!contentType.empty() && contentType[0] == ' ')
                        contentType.erase(0, 1);
                }
            }
        }
        else
            body = _buffer;

        // Envoyer la réponse HTTP
        if(body.empty())
        {
            body = "No content returned from CGI script.";
            HttpForms ok(0, 500, this->_client->keepalive, contentType, body, this->_client->_request->_ReqContent);
        }
        else 
             HttpForms ok(0, 200, this->_client->keepalive, contentType, body, this->_client->_request->_ReqContent);
        this->_client->status = WRITING;
        this->_client->cgi_buffer.clear();
        return true ;
    }
    return false;
}

const std::string& CGIHandler::getBuffer() const { return _buffer; }
bool CGIHandler::isFinished() const { return _finished; }


 char** CGIHandler::buildEnvArray()const
{
    std::map<std::string, std::string> env = this->_env;
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
