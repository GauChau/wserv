#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include "request.hpp"
#include "webserv.hpp"
#include <csignal>


inline std::string nonblocking_read(const std::string& file_path)
{
    std::ifstream outjoe(file_path.c_str(), std::ios::binary);
        std::ostringstream ss;
        ss << outjoe.rdbuf();
        std::string content = ss.str();
    return content;
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


// - exec CGI script in a fork()
// - returns CGI stdout in a std::str


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