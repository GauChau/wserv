#include "webserv.hpp"

int main(int argc, char **argv)
{
    if(argc != 2 || !argv[1]){
        std::cout << "Your executable should be executed as follows:\n \033[36m ./webserv [configuration file(.conf)]\033[0m " 
        << std::endl;
        return (EXIT_FAILURE);
    }
    (void)(argv);
    Webserv w;
    if(!w.parseConfigFile(argv[1]))
    {
        std::cout << "Error parsing:\n \033[36m "<< argv[1]<< "[.conf]\033[0m " 
        << std::endl;
        return (EXIT_FAILURE);
    }
    // init all servers (TCPs, sockets ...)
    try{
        w.init();
    } catch (const std::invalid_argument& e)
    {
        std::cerr << "\033[31mError initializing servesConfigs: \033[0m" << e.what() << std::endl;
        return (EXIT_FAILURE);
    }
    // // start the server
    try{
        w.start();  // Invalid input
    } catch (const std::invalid_argument& e)
    {
        std::cerr << "\033[33m Error starting the server: \033[0m" << e.what() << std::endl;
        return (EXIT_FAILURE);
    }
    return 0;
}