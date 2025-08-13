#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>
#include <stdexcept>

class ServerConfig
{
public:
    ServerConfig();
    ServerConfig(const ServerConfig &server_config);
    ~ServerConfig();
    ServerConfig &operator=(const ServerConfig &server_config);
    size_t getPort();
    size_t getMaxConnections();
    void setPort(size_t port);
    void setMaxConnections(size_t max_connections);

private:
    size_t port_;
    size_t max_connections_;
};

#endif