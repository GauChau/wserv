#include "webserv.hpp"

static std::string trim(const std::string& str)
{
    size_t
        start = str.find_first_not_of(" \t\r\n"),
        end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ?
        "" :
        str.substr(start, end - start + 1);
}

// turn [10M, 1G, 254k] => straight size_t
static size_t parseSize(const std::string& input)
{
    size_t multiplier = 1;
    std::string numPart = input;
    if (input[input.size() - 1] == 'K' || input[input.size() - 1] == 'k') {
        multiplier = 1024;
        numPart.erase(numPart.size() - 1);
    } else if (input[input.size() - 1] == 'M' || input[input.size() - 1] == 'm') {
        multiplier = 1024 * 1024;
        numPart.erase(numPart.size() - 1);
    } else if (input[input.size() - 1] == 'G' || input[input.size() - 1] == 'g') {
        multiplier = 1024 * 1024 * 1024;
        numPart.erase(numPart.size() - 1);
    }
    std::istringstream iss(numPart);
    unsigned long long value = 0;
    iss >> value;

    return value * multiplier;
}


static std::vector<std::string> split_string(const std::string& input)
{
    std::vector<std::string> result;
    std::istringstream stream(input);
    std::string w;
        while (stream >> w)
        result.push_back(w);
    return result;
}

bool Webserv::parseConfigFile(const std::string& filename)
{
     if (filename.empty())
    {
        std::cerr << "empty .conf file!" << std::endl;
        return false;
    }
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Could not open config file!" << std::endl;
        return false;
    }

    std::string line;
    std::stack<std::string> context;
    ServerConfig cur_serv;
    LocationConfig serv_loc;

    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line == "server {")
        {
            context.push("server");
            cur_serv = ServerConfig(); // reset

        } else if (line == "location / {" || line == "route / {"  || (     // append new location
            line.find("location") != std::string::npos && line.find("/") != std::string::npos &&
            line.find("{") != std::string::npos)
            || (  // append new route
            line.find("route") != std::string::npos && line.find("/") != std::string::npos &&
            line.find("{") != std::string::npos)
        )
        {
            context.push("location");
            serv_loc = LocationConfig(); // reset
            size_t
                startPos = line.find('/'),
                endPos = line.find('{');
            //std::cout << "MON GROS PAFF " << line.substr(startPos, endPos - startPos - 1) << std::endl;
            serv_loc.path = line.substr(startPos, endPos - startPos - 1);
        } else if (line == "}")
        {
            if (!context.empty())
            {
                if (context.top() == "location")
                    cur_serv.locations.push_back(serv_loc);
                else if (context.top() == "server")
                    this->servers.push_back(cur_serv);
                context.pop();
            }
        } else
        {
            std::istringstream
                iss(line);
            std::string
                key,
                value;
            iss >> key >> value;
            if (!value.empty() && value[value.size()-1] == ';')
                value = value.substr(0, value.size() - 1);

            if (!context.empty())
            {
                // server part
                if (context.top() == "server")
                {
                    if (key == "host") cur_serv.host = value;
                    else if (key == "port") cur_serv.port = atoi(value.c_str());
                    else if (key == "server_name") cur_serv.server_name = value;
                    else if (key == "client_max_body_size") cur_serv.client_max_body_size = (size_t)(parseSize(value));
                    else if (key == "error_page") {
                        std::vector<std::string> s = split_string(line);
                        cur_serv.error_pages.push_back(std::make_pair(
                            static_cast<unsigned int>(atoi( s[1].c_str())),
                            s[2]
                        ));

                    }
                }
                // eaach route part
                else if (context.top() == "location")
                {
                    if (key == "root")
                        serv_loc.root = value;
                    else if (key == "autoindex")
                        serv_loc.autoindex = (value == "on");
                    else if (key == "methods"){
                        // serv_loc.allowed_methods.push_back(line.substr(0, line.size() - 1));
                        line = line.substr(0, line.size() - 1);  // removing last char (e.g., '\n')
                        std::istringstream iss(line);
                        std::string method;

                        while (iss >> method) {
                            serv_loc.allowed_methods.push_back(method);
                        }

                    }
                    else if (key == "index")
                        serv_loc.index = (value);
                    else if (key == "cgi_extension")
                        serv_loc.cgi_extension = (value);
                    else if (key == "cgi_path")
                        serv_loc.cgi_path = (value);
                }
            }
        }
    }

    file.close();
    std::cout << "\033[92m ==== SUCCESSFULLY PARSED " << servers.size() << " SERVERS ! ==== \033[0m" << std::endl;
    if(true == false)
    {
        for (size_t i = 0; i < servers.size(); ++i)
        {
            // if client_max_body_size not precised
            if(!servers[i].client_max_body_size)
                servers[i].client_max_body_size = parseSize("10M");

            std::cout << "\033[1;96m__________ [Server " << i << "] ________ \n - host: " << servers[i].host << "]\n - port: " << servers[i].port <<"]\033[0m" << std::endl;
            std::cout << "\033[1;96m - server_name: '" << servers[i].server_name << "' \n";
            std::cout << " - client_max_body_size: " << servers[i].client_max_body_size << " (bytes) \n";
            std::cout << " - error_pages: \n";
            for (size_t a = 0; a < servers[i].error_pages.size(); ++a)
                    std::cout << "\t [ " << servers[i].error_pages[a].first << ", " << servers[i].error_pages[a].second << "]" << std::endl;
            std::cout << "\033[0m" << std::endl;
            for (size_t j = 0; j < servers[i].locations.size(); ++j)
            {
                std::cout << " \033[94m     (Location/Route): '" << servers[i].locations[j].path << "'\n"
                        << "\t -root: " << servers[i].locations[j].root << "\n"
                        << "\t -index: " << servers[i].locations[j].index << "\n"
                        << "\t -autoindex: " << (servers[i].locations[j].autoindex ? "on" : "off") << "\n"
                        << "\t -methods: ";
                        if(!servers[i].locations[j].allowed_methods.size())
                            std::cout << "\n";
                        for (size_t k = 0; k < servers[i].locations[j].allowed_methods.size(); ++k)
                            std::cout << servers[i].locations[j].allowed_methods[k] << std::endl;
                        std::cout << "\t -cgi_path: " << servers[i].locations[j].cgi_path << "\n"
                        << "\t -cgi_extension: " << servers[i].locations[j].cgi_extension << "\n"
                        << "\033[0m"
                        << std::endl;

            }
        }
    }
    return true;
}