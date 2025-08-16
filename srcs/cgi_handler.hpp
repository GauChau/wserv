#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include "utils.hpp"

class client; // Forward declaration

class CGIHandler
{
public:
    CGIHandler(client* client);
    ~CGIHandler();

    void setEnv(const std::map<std::string, std::string>& env);
    void setScriptPath(const std::string& path);
    void setRequestBody(const std::string& body);

    // Lance le CGI et retourne le fd à surveiller
    int launch();

    // Lecture non bloquante du rendu CGI
    bool readOutput();

    // Accès au buffer CGI
    const std::string& getBuffer() const;
    std::string getrequestBody() const { return _requestBody; }

    // Indique si le CGI est terminé
    bool isFinished() const;
    int registered = 0;

private:
    client* _client;
    std::map<std::string, std::string> _env;
    std::string _scriptPath;
    std::string _requestBody;

    int _fd;           // fd du pipe de sortie CGI
    pid_t _pid;        // pid du processus CGI
    std::string _buffer;
    bool _finished;

    char** buildEnvArray() const;
};