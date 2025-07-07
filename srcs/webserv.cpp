#include "webserv.hpp"
#include "request.hpp"

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










// handle a client request (send a basic HTTP response)
// static bool handle_client(int client_socket, const ServerConfig &serv)
// {
//     char buffer[2048];
//     // std::cerr << " \033[31m Error " << client_socket<<buffer<<sizeof(buffer-1) << "\033[0m" << std::endl;
//     ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
//     if (bytes_received < 0)
//     {
//         std::cerr << " \033[31m Error receiving data from client!  : " << client_socket <<" errno: "<<errno<< "\033[0m" << std::endl;
//         close(client_socket);
//         return (false);
//     }
//     buffer[bytes_received] = '\0'; // null-terminate data


//     Request R = Request(buffer, serv, client_socket);
//     std::string response_ = R._get_ReqContent();
//     send(client_socket, response_.c_str(), (response_.size()), 0);
//     close(client_socket); // Close client socket after sending the response

//     return(true);
// }


static bool handle_client(int client_socket, const ServerConfig &serv)
{
    char buffer[2048];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // no data now, try again (non-blocking socket)
            return false; // don't close socket, let poll try again
        }
        std::cerr << "\033[31m[x] recv() error on client " << client_socket
                  << ": " << strerror(errno) << "\033[0m\n";
        close(client_socket);
        return true; // done with this socket (error, cleanup)
    }
    else if (bytes_received == 0)
    {
        // Client closed connection
        std::cerr << "\033[33m[~] Client disconnected: " << client_socket << "\033[0m\n";
        close(client_socket);
        return true;
    }
    // buffer[bytes_received] = '\0';

    // log  received HTTP request
    Request R(buffer, serv, client_socket, bytes_received);
    std::string response = R._get_ReqContent();
    ssize_t sent = send(client_socket, response.c_str(), response.size(), 0);
    if (sent < 0)
    {
        std::cerr << "\033[31m[x] send() failed: " << strerror(errno) << "\033[0m\n";
    }
    close(client_socket);
    return true;
}





void Webserv::start(void)
{
    std::vector<pollfd> poll_fds;
    std::map<int, ServerConfig*> client_fd_to_server; // client_socket -> svconfig
    std::map<int, ServerConfig*> fd_to_server;        // server_socket -> svconfig
    std::vector<int> fds_to_remove;
    std::set<int> server_fds;

    // Register all server sockets
    for (size_t i = 0; i < this->servers.size(); ++i)
    {
        int server_fd = this->servers[i].server_socket;

        pollfd pfd;
        pfd.fd = server_fd;
        pfd.events = POLLIN;
        poll_fds.push_back(pfd);

        fd_to_server[server_fd] = &this->servers[i];
        server_fds.insert(server_fd); // track listening sockets
    }

    std::cout << "\033[92m ===== STARTED " << poll_fds.size() << " SERVERZ ===== \033[0m" << std::endl;

    // poll() loop on all servers
    while (true)
    {
        int ret = poll(poll_fds.data(), poll_fds.size(), -1);
        if (ret == -1)
        {
            perror("poll");
            break;
        }

        fds_to_remove.clear();

        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            struct pollfd& pfd = poll_fds[i];

            if (!(pfd.revents & POLLIN))
                continue;

            // accept new connections on serv socket
            if (server_fds.count(pfd.fd))
            {
                ServerConfig* serv = fd_to_server[pfd.fd];
                if (!serv)
                {
                    std::cerr << "Error: no ServerConfig for fd " << pfd.fd << std::endl;
                    continue;
                }

                int client_fd = accept(pfd.fd,
                                       (struct sockaddr*)&(serv->client_addr),
                                       &serv->client_addr_len);

                if (client_fd < 0)
                    continue;

                // socket timeout (optional, useful)
                struct timeval timeout = {15, 0}; // 15 seconds
                setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

                // make non-blocking
                int flags = fcntl(client_fd, F_GETFL, 0);
                if (flags != -1)
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                // track new client socket
                pollfd client_pfd;
                client_pfd.fd = client_fd;
                client_pfd.events = POLLIN;
                poll_fds.push_back(client_pfd);

                client_fd_to_server[client_fd] = serv;
            }
            else
            {
                // handle client req
                ServerConfig* serv = client_fd_to_server[pfd.fd];
                if (!serv)
                    continue;

                bool done = handle_client(pfd.fd, *serv);
                if (done)
                    fds_to_remove.push_back(pfd.fd);
            }
        }

        // clean-up client sockets
        for (size_t j = 0; j < fds_to_remove.size(); ++j)
        {
            int fd = fds_to_remove[j];

            if (server_fds.count(fd))
                continue; // do not remove server sockets

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
}