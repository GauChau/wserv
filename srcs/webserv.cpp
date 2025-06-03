#include "webserv.hpp"
#include "request.hpp"

/*
    BRIEF :
    ✅ Common Socket Syscalls (for a TCP server)

    1️⃣ Create socket	socket()	Creates a new socket file descriptor (FD).
    1. -int fd = socket(AF_INET, SOCK_STREAM, 0);
       -creates a socket using IPv4 (AF_INET) and TCP (SOCK_STREAM).

    2️⃣ Bind address	bind()	Assigns an address (IP + port) to socket.
    2. -bind(fd, (struct sockaddr*)&addr, sizeof(addr));
       -binds socket to an IP and port so  kernel knows where to listen.

    3️⃣ Listen for clients	listen()	Marks socket as passive (ready to accept connections).
    3.  -listen(fd, backlog);
        -enables socket to accept incoming connections (backlog = queue size).

    4️⃣ Accept connection	accept()	Accepts an incoming connection, returns a new socket FD for that client.
    4.  -int client_fd = accept(server_fd, NULL, NULL);.
        -accepts client connection. returns new FD for communicating with that client.


    5️⃣ recv() / read()	read data/bytes from client socket.
    5. -recv(client_fd, buffer, sizeof(buffer), 0);
        // or
       -read(client_fd, buffer, sizeof(buffer));

    6️⃣ send() / write()sends data back to client socket.
    6. - send(client_fd, response, length, 0);
        // or
        write(client_fd, response, length);

    7️⃣ close socket	close()	closes socket file descriptor.
    7. - close(fd);
       - closes the socket (server or client).
*/


Webserv::Webserv(void)
{
    std:: cout << "WEBSERVER CAME !"<< std::endl;
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
        std::cout << "\033[32mServer[" << i << "] is ready on  " << serv_.host << ":" << serv_.port  << "\033[0m "<< std::endl;
    }
}


// handle a client request (send a basic HTTP response)
void Webserv::handle_client(int client_socket, const ServerConfig &serv)
{
    char buffer[1024];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
    {
        // std::cerr << " \033[31m Error receiving data from client!  : " << client_socket << "\033[0m" << std::endl;
        close(client_socket);
        return;
    }

    // log  received HTTP request
    std::cout << "\033[36m>>>>>>>>> Received a request! <----\n" << buffer << "\nEND REQUEST\033[0m"<< std::endl;
    Request R = Request(buffer, serv, client_socket);
    buffer[bytes_received] = '\0'; // null-terminate data
    std::string response_ = R._get_ReqContent();
    send(client_socket, response_.c_str(), (response_.size()), 0);
    close(client_socket); // Close client socket after sending the response
}

void Webserv::start(void)
{
    std::vector<pollfd> poll_fds;
    std::map<int, ServerConfig*> client_fd_to_server; // client_socket -> svconfig
    std::map<int, ServerConfig*> fd_to_server; // server_socket -> svconfig
    std::vector<int> fds_to_remove;


    // Register all server sockets
    for (size_t i = 0; i < this->servers.size(); ++i)
    {
        pollfd pfd;
        pfd.fd = this->servers[i].server_socket;
        pfd.events = POLLIN;
        poll_fds.push_back(pfd);
        fd_to_server[this->servers[i].server_socket] = &this->servers[i];
    }

    std::cout << "\033[92m ===== STARTED " << poll_fds.size() << " SERVERZ ===== \033[0m" << std::endl;

    // poll() loop on all servers
    while (true)
    {
        int ret = poll(poll_fds.data(), poll_fds.size(), -1);
        if (ret == -1) {
            perror("poll"); // cant poll ?
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            struct pollfd& pfd = poll_fds[i];

            // nothing....
            if (!(pfd.revents & POLLIN))
                continue;

            // accept new [connection]
            if (fd_to_server.count(pfd.fd))
            {
                ServerConfig* serv = NULL;
                std::map<int, ServerConfig*>::iterator  it = fd_to_server.find(pfd.fd);
                if (it != fd_to_server.end() && it->second != NULL)
                    serv = it->second;
                else
                    std::cerr << "Error: no valid ServerConfig found for [fd]: " << pfd.fd << std::endl;


                int client_fd = accept(pfd.fd,
                    (struct sockaddr*)&(serv->client_addr), &serv->client_addr_len
                );

                if (client_fd < 0) {
                    std::cerr << "Error accepting connection on port "<< serv->port << ": " << strerror(errno) << std::endl;
                    continue;
                }

                // add new client socket to poll
                pollfd client_pfd;

                client_pfd.fd = client_fd;
                client_pfd.events = POLLIN;
                poll_fds.push_back(client_pfd);
                client_fd_to_server[client_fd] = serv;

            }
            // handle [connection]
            else
            {
                // client socket: Handle client
                std::cerr << "///handle client// " <<std::endl;
                ServerConfig* serv = client_fd_to_server[pfd.fd];
                this->handle_client(serv->client_socket, *serv);

                if (true) {
                    int client_fd = pfd.fd;  // Add this at the start of the else branch
                    close(client_fd);

                    // For now: always close the client after handling one request
                    fds_to_remove.push_back(client_fd);
                    // std::cout << "Successfully closed connection (fd: " << client_fd << ")" << std::endl;
                }

            }
        }
    }
    // cleanups
    for (size_t j = 0; j < fds_to_remove.size(); ++j)
    {
        int fd_to_remove = fds_to_remove[j];

        client_fd_to_server.erase(fd_to_remove);

        for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
        {
            if (it->fd == fd_to_remove) {
                poll_fds.erase(it);
                break;
            }
        }

        std::cout << "[-] Closed connection and removed fd: " << fd_to_remove << std::endl;
    }
}