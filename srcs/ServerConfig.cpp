#include "ServerConfig.hpp"

ServerConfig::ServerConfig() : port_(4242), max_connections_(4096) {}

ServerConfig::ServerConfig(const ServerConfig &server_config)
{
    port_ = server_config.port_;
    max_connections_ = server_config.max_connections_;
}

ServerConfig::~ServerConfig() {}

ServerConfig &ServerConfig::operator=(const ServerConfig &server_config)
{
    if (this != &server_config)
    {
        port_ = server_config.port_;
        max_connections_ = server_config.max_connections_;
    }

    return *this;
}

size_t ServerConfig::getPort()
{
    return port_;
}

size_t ServerConfig::getMaxConnections()
{
    return max_connections_;
}

void ServerConfig::setPort(size_t port)
{
    if (port < 1024 || port > 65535)
        throw std::invalid_argument("Port number must be between 1024 and 65535.");

    port_ = port;
}

void ServerConfig::setMaxConnections(size_t max_connections)
{
    if (max_connections < 1 || max_connections > 65535)
        throw std::invalid_argument("Maximum connections must be between 1 and 65535.");

    max_connections_ = max_connections;
}
