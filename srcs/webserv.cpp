#include "webserv.hpp"
#include "request.hpp"
#include "HttpForms.hpp"
#include "utils.hpp"
#include "client.hpp"
#include "cgi_handler.hpp"

pollfd* findPollfd(std::vector<pollfd>& poll_fds, int target_fd)
{
    for (std::vector<pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
    {
        if (it->fd == target_fd)
        {
            return &(*it); // retourne un pointeur vers la struct pollfd
        }
    }
    return NULL; // si non trouvé
}

Webserv::Webserv(void)
{
    // std:: cout << "WEBSERVER CAME !"<< std::endl;
}

Webserv::~Webserv()
{
    for(uint32_t i = 0; i < this->servers.size(); i ++)
    {
        ServerConfig serv_ = this->servers[i];
        if (serv_.client_socket > 0) {
            close(serv_.client_socket);
            std::cout << "Client socket closed." << std::endl;
        }

        if (serv_.server_socket > 0) {
            close(serv_.server_socket);
            std::cout << "Server socket closed." << std::endl;
        }
    }
}

void Webserv::init(void)
{
    for(uint32_t i = 0; i < this->servers.size(); i ++)
    {
        ServerConfig &serv_ = this->servers[i];
        if(serv_.host.empty() || serv_.port <= 0)
        {
            std::cerr << "Wrong .conf initialization [ports | domain | methods | root]!" << std::endl;
            continue;
        }

        serv_.client_addr_len = sizeof(serv_.client_addr);
        // create server socket
        serv_.server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (serv_.port < 1024)
            std::cerr << "Warning: Using a port below 1024 may require elevated privileges." << std::endl;
        if (serv_.server_socket < 0)
        {
            perror("Error creating socket");
            continue;
        }else {
            // std::cout << "Socket created successfully, server_socket = " << serv_.server_socket  << std::endl;
            // Allow address reuse
            int opt = 1;
            if (setsockopt(serv_.server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                std::cerr << "Error setting socket options: " << strerror(errno) << std::endl;
                close(serv_.server_socket);
                continue;;
            }
        }

        memset(&serv_.server_addr, 0, sizeof(serv_.server_addr));
        serv_.server_addr.sin_family = AF_INET;
        serv_.server_addr.sin_port = htons(serv_.port);
        // host string to network address (binary form)
        in_addr_t address = inet_addr(serv_.host.c_str());
        serv_.server_addr.sin_addr.s_addr = address; // like "127.0.0.1"
        // check if ip address is valid
        if (address == INADDR_NONE)
        {
            std::cerr << "Invalid IP address format: " << serv_.host << std::endl;
            close(serv_.server_socket);
            exit(EXIT_FAILURE);
        }


        // bind socket to address and port
        // (turns [address, port] -> [fd] )
        if (bind(serv_.server_socket, (struct sockaddr*)&serv_.server_addr, sizeof(serv_.server_addr)) < 0)
        {
            if (errno == EADDRINUSE){
                std::cerr << "\033[36mServer["<<i << "] port "<<  serv_.host << ":" << serv_.port << " is already in use! \033[0m" << std::endl;
                close(serv_.server_socket);
                this->servers.erase(this->servers.begin() + i);
                i--;
                continue;
            }
            std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
            close(serv_.server_socket);
            continue;
        }

        // Listen for incoming connections
        if (listen(serv_.server_socket, SOMAXCONN) < 0)
        {
            std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
            close(serv_.server_socket);
            return;
        }
        std::cout << "\033[32mServer[" << i << "] is ready on  " << serv_.host
                << ":" << serv_.port << " ["<< serv_.server_name << "]" << "\033[0m "<< std::endl;
    }
}

// SIGINT 
volatile sig_atomic_t g_terminate = 0;

inline void handle_sigint(int signal)
{
    if (signal == SIGINT)
        g_terminate = 1;
}

void Webserv::start(void)
{
    std::vector<pollfd> poll_fds;
    std::list<pollfd> poll_fds_list;
    std::map<int, ServerConfig*> client_fd_to_server; // client_socket -> svconfig
    std::map<int, ServerConfig*> fd_to_server;        // server_socket -> svconfig
    std::vector<int> fds_to_remove;
    std::set<int> server_fds;
    std::map<int, client*> clientlist;

    // Register all server sockets
    for (size_t i = 0; i < this->servers.size(); ++i)
    {
        int server_fd = this->servers[i].server_socket;

        pollfd pfd;
        pfd.fd = server_fd;
        pfd.events = POLLIN;// | POLLOUT; 
        poll_fds.push_back(pfd);
        pfd.revents = 0;
        fd_to_server[server_fd] = &this->servers[i];
        server_fds.insert(server_fd); // track listening sockets
    }

    std::cout << "\033[92m ===== STARTED " << poll_fds.size() << " SERVERZ ===== \033[0m" << std::endl;
    std::signal(SIGINT, handle_sigint);  
       
    while (!g_terminate)
    {
        ///////////////////////////
        //ADD CGI HANDLER TO POLL//
        ///////////////////////////
        for (std::map<int, client*>::iterator it = clientlist.begin(); it != clientlist.end(); ++it)
        {
            client* client_ptr = it->second;
            if (client_ptr->cgi_handler && client_ptr->cgi_fd > -1 && client_ptr->cgi_handler->registered == 0)
            {
                client_ptr->cgi_handler->registered = 1;
                // std::cerr<<" register pollcgi_fd n: "<<client_ptr->cgi_fd ;
                struct pollfd cgi_pfd;
                cgi_pfd.fd = client_ptr->cgi_fd;
                cgi_pfd.events = POLLIN;
                cgi_pfd.revents = 0;
                poll_fds.push_back(cgi_pfd);
            }
        }


        //////////////////////
        //polling for events//
        //////////////////////
        int ret = poll(poll_fds.data(), poll_fds.size(), 500); // 100ms timeoutt
        if (ret < 0)
        {
            if (errno == EINTR) continue; // poll was interrupted by signal
            perror("poll");
            break;
        }
         if (ret == 0)
            continue;
        fds_to_remove.clear();


        /////////////////////////////////////////////////////////////
        //LOOPING THROUGH ALL REGISTERD PFD TO FIND THE PINGED ONES//
        /////////////////////////////////////////////////////////////
        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            struct pollfd& pfd = poll_fds[i];
            // if (clientlist.count(pfd.fd))
            // {
            //     std::cerr << "  AAAClient " << pfd.fd << " STATUS: " << clientlist[pfd.fd]->status << std::endl;
            // }

            //print pfd revents if pfd reven
            if (pfd.revents)
            {
                // std::cerr << "\n  PINGED FD: " << pfd.fd << " REVENTS: " << pfd.revents;
                if (clientlist.count(pfd.fd))
                {
                    // std::cerr << " STATUS: " << clientlist[pfd.fd]->status ;
                    // if (clientlist[pfd.fd]->_request)
                        // std::cerr<< " class request " << clientlist[pfd.fd]->_request << " reqstat " << clientlist[pfd.fd]->_request->_request_status;
                }
                // std::cerr << std::endl;
            }
            
            ////////////////////////////////////////////////////////////////////
            //WRITING RESPONSE ON THE SOCKET IF READY TO RECEIVE DATA(POLLOUT)//
            ////////////////////////////////////////////////////////////////////
            if ((pfd.revents & POLLOUT) && pfd.events == POLLOUT && clientlist.count(pfd.fd))
            {
                if (clientlist[pfd.fd]->getStatus() == WRITING)
                {
                    if(!clientlist[pfd.fd]->answerClient(pfd))
                        continue ;
                    if (clientlist[pfd.fd]->keepalive == false)
                    {
                        close(pfd.fd);
                        fds_to_remove.push_back(pfd.fd);
                        delete clientlist[pfd.fd];
                        clientlist.erase(pfd.fd);
                    }
                }
                continue ;
            }

           
            /////////////////////////////////////////
            //READING THE CGI RESULT ON HIS PIPE FD//
            /////////////////////////////////////////
            for (std::map<int, client*>::iterator it = clientlist.begin(); it != clientlist.end(); ++it)
            {
                client* client_ptr = it->second;
                if (client_ptr->cgi_handler && client_ptr->cgi_fd == pfd.fd )
                {
                    if (pfd.revents & (POLLIN | POLLHUP))
                    {
                        bool finished = client_ptr->cgi_handler->readOutput();
                        if (finished) {

                            // std::cerr << "  B4 cgi FD |" << pfd.fd << "| STATUS: " << client_ptr->status<< "CGI FD: " << client_ptr->cgi_fd;
                            // std::cerr<< " Parser et envoyer la réponse HTTP ";
                            // client_ptr->_request->_ReqContent = client_ptr->cgi_handler->getBuffer();
                            // ...parse headers/body comme avant...
                            // ...envoie la réponse via HttpForms...
                            fds_to_remove.push_back(pfd.fd);
                            client_ptr->status = WRITING;
                            delete client_ptr->cgi_handler;
                            client_ptr->cgi_handler = NULL;
                            // std::cerr << " AFTER CLIENT |" << pfd.fd << "| STATUS: " << client_ptr->status<< "CGI FD: " << client_ptr->cgi_fd
                                // <<" CLIENT FD. " << client_ptr->getFd();


                            struct pollfd* changeevent = findPollfd(poll_fds, client_ptr->getFd());
                            if (changeevent)
                            {
                                changeevent->events = POLLOUT; // Set to POLLOUT for writing response
                                // std::cerr << "  CHANGEEVENT POLLOUT " << changeevent->fd << std::endl;
                            }
                        }
                    }
                }
            }

            ////////////////////////////////////////////////////////

            
            if (!(pfd.revents & (POLLIN | POLLOUT | POLLHUP)))
                continue;
            
            if (clientlist.count(pfd.fd) && clientlist[pfd.fd]->status ==   WRITING)
            {
                // std::cerr << "  BBBBClient " << pfd.fd << " WRITING ";
            }
            //////////////////////////////////////////////////////////////////////////////
            //IF THE POLLED FD IS A SERVER FD, MEANS NEW CLIENT. CREATE A SOCKET FOR HIM//
            //////////////////////////////////////////////////////////////////////////////
            if (server_fds.count(pfd.fd))
            {
                ServerConfig* serv = fd_to_server[pfd.fd];
                if (!serv)
                {
                    // std::cerr << "Error: no ServerConfig for fd " << pfd.fd << std::endl;
                    continue;
                }
                int client_fd = accept(pfd.fd, (struct sockaddr*)&(serv->client_addr), &serv->client_addr_len);                
                if (client_fd < 0)
                    continue;

                // socket timeout (optional, useful)
                struct timeval timeout = {10, 0}; // 10 seconds
                setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                
                // make non-blocking
                int flags = fcntl(client_fd, F_GETFL, 0);
                if (flags != -1)
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);


                pollfd client_pfd;
                client_pfd.fd = client_fd;
                client_pfd.events = POLLIN;// | POLLOUT;
                client_pfd.revents = 0;
                poll_fds.push_back(client_pfd);
                client_fd_to_server[client_fd] = serv;
                clientlist[client_fd] = new client(client_pfd, serv);
            }
            //////////////////////////////////////////////////////////////
            //POLL FD ISNT A SERV, SO A CLIENT ALREADY CREATED, TREAT IT//
            //////////////////////////////////////////////////////////////
            else 
            {
                ServerConfig* serv = client_fd_to_server[pfd.fd];
                if (!serv)
                    continue;
                // std::cerr<<" PASSED "<<pfd.revents << " on pfd" << pfd.fd;
                clientlist[pfd.fd]->handle_jesus(pfd);
            }
        }

        ///////////////////////////////
        //CLEANING CLOSED CONNECTIONS//
        ///////////////////////////////
        for (size_t j = 0; j < fds_to_remove.size(); ++j)
        {
            int fd = fds_to_remove[j];

            if (server_fds.count(fd)) continue; // do not remove server sockets
            // std::cerr << "erasing: "<< fd;
            client_fd_to_server.erase(fd);

            for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
            {
                if (it->fd == fd)
                {
                    poll_fds.erase(it);
                    break;
                }
            }
        }
    }
    // Close all client sockets
    for (std::map<int, ServerConfig*>::iterator it = client_fd_to_server.begin(); it != client_fd_to_server.end(); ++it)
        close(it->first);
    // Close all server sockets
    for (std::set<int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it)
        close(*it);
    std::cout << "Server shutdown gracefully.\n";
}




bool Webserv::handleCGIResponse(client* client_ptr)
{
    if (!client_ptr) return false;
    // std::cerr<<" AhandleCGIResponse ";

    char buffer[4096];
    ssize_t bytes_read = read(client_ptr->cgi_fd, buffer, sizeof(buffer));
    if (bytes_read > 0)
    {
        client_ptr->cgi_buffer.append(buffer, bytes_read);
        // On attend la fin du CGI (EOF) pour parser et répondre
        // std::cerr<<" APPENDCGI RESP ";
        return true;
    }
    if (bytes_read == 0) // EOF : CGI terminé
    {
        // std::cerr<<" EOF REACHED ";
        close(client_ptr->cgi_fd);
        // client_ptr->cgi_fd = -1;

        // Parser headers/body
        std::string::size_type header_end = client_ptr->cgi_buffer.find("\r\n\r\n");
        if (header_end == std::string::npos)
            header_end = client_ptr->cgi_buffer.find("\n\n");
        std::string contentType = "text/plain";
        std::string body;
        if (header_end != std::string::npos)
        {
            // std::cerr<<" EOFB ";
            std::string headers = client_ptr->cgi_buffer.substr(0, header_end);
            body = client_ptr->cgi_buffer.substr(header_end + 4);
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
            body = client_ptr->cgi_buffer;
        // std::cerr<<" EOFC ";

        // Envoyer la réponse HTTP
        HttpForms ok(client_ptr->getFd(), 200, client_ptr->keepalive, contentType, body, client_ptr->_request->_ReqContent);
        // std::cerr<<"client_ptr->_request->_ReqContent: "<<client_ptr->_request->_ReqContent<<std::endl;


        client_ptr->status = WRITING;
        client_ptr->cgi_buffer.clear();
        return true ;
    }
    return false;

    // Si bytes_read < 0 et errno == EAGAIN/EWOULDBLOCK, on attend le prochain poll
}


