#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>  // for std::invalid_argument
#include <vector>
#include <map>
#include <stack>
#include <sstream>
#include <arpa/inet.h>
#include <filesystem>
#include <sys/stat.h>
#include <algorithm>

// std::string request_handler(char *request_buffer, const ServerConfig& server)
// {
//     std::string request(request_buffer), response, line_buffer, key_buffer, method;
//     std::istringstream request_stream(request);
//     int line =0, word=0;

//     while (std::getline(request_stream, line_buffer))
//     {
//         // std::cout<<"GETLINE:"<< line_buffer<<std::endl;
//         std::istringstream key_stream(line_buffer);
//         while(std::getline(key_stream, key_buffer,' '))
//         {
//             if (line==0 && word ==0)
//             {
//                 method = check_allowed_methods(key_buffer, server);
//             }
//             if (line==0 && word ==1)
//             {

//             }
//             word++;
//             // std::cout<<"GETword:"<< key_buffer<<std::endl;
//         }
//         line =1;
//         // std::cout<<std::endl;
//     }
//     // std::cout<<"endofGETLINE\n";
//     return request;

// }

// std::string check_allowed_methods(std::string key, const ServerConfig& server)
// {
//     std::vector<LocationConfig>::const_iterator it_loc = server.locations.begin();
//     for(;it_loc != server.locations.end();it_loc++)
//     {
//         std::cout<<"servlocs |"<<it_loc->path<< "|"<<std::endl;
//         std::vector<std::string>::const_iterator it_meth = it_loc->allowed_methods.begin();
//         for(;it_meth != it_loc->allowed_methods.end();it_meth++)
//         {
//             // std::cout<<"methloop: |"<<*it_meth<<"|"<<std::endl;
//             if(key == *it_meth)
//             {
//                 std::cout<<"method |"<<key<< "|:AUTHORIZED"<<std::endl;
//                 // return key;
//             }
//         }
//     }
//     std::cout<<"method "<<key<< ": NOT AUTHORIZED"<<std::endl;
//     key = "DENY";
//     return key;
// }