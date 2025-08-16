#ifndef REQUEST_HPP

# define REQUEST_HPP

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
#include "webserv.hpp"
#include <dirent.h>
#include <string>
#include <iostream>

#define WAITING 0
#define READING 1
#define READINGDATA 2
#define WRITING 3
#define CLOSING 4
#define DONE 5
#define READINGPOSTDATA 6
#define READINGHEADER 7
#define EXECUTING 8
#define DELETE 9
#define SENDERROR 10

struct file_id
{
    std::string name, fname, type;
};

class Request
{
    public:
      Request(const ServerConfig &serv, int socket, int status);
      Request(std::string buffer, const ServerConfig &serv, int socket, ssize_t bytes_rec);
      ~Request();

      void readRequest();
      void sendResponse();
      void requestParser();
      int checkPostDataOk();
      int checkHeaderCompletion();
      void check_allowed_methods(const ServerConfig &serv);
      void execute(std::string s);
      std::string _get_ReqContent();
      std::map<std::string,std::string> http_params;
      int _socket;
      bool authorized,keepalive;
      std::string& getMethod(){return(this->r_method);};
      std::string& getHeader(){return(this->r_header);};
      std::string& getExecCode(){return(this->exec_code);};
      std::string& getDataRec(){return(this->_datarec);};
      void Post_data_write();
      int _request_status;
      size_t _bytes_rec, _contlen, ret, _totalrec, _totalsent;
    private:
        const ServerConfig &_server;
        LocationConfig _loc;
		    std::string r_method, r_location,
                    r_version, r_boundary,
                    r_body, r_header,
                    location_filename,
                    connec, exec_code, _datarec;
        file_id file;
        

        void Post();
        void Post_data_receive();
        
        void Get();
        void Delete();
        void writeData();
        int checkbound(std::istringstream& s);
        std::string _ReqContent;

};

#endif