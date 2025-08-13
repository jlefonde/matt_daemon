#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>

class ServerConfig
{
public:
    ServerConfig();
    ServerConfig(const ServerConfig &server_config);
    ~ServerConfig();
    ServerConfig &operator=(const ServerConfig &server_config);

private:
    size_t port_;
    size_t max_connections_;
};

#endif