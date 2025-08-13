#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

#include "DaemonConfig.hpp"
#include "ServerConfig.hpp"
#include "LoggerConfig.hpp"

class Config
{
public:
    Config();
    Config(const Config &config);
    ~Config();
    Config &operator=(const Config &config);
    DaemonConfig &getDaemonConfig();
    ServerConfig &getServerConfig();
    LoggerConfig &getLoggerConfig();
    void setDaemonConfig(DaemonConfig &daemon_config);
    void setServerConfig(ServerConfig &server_config);
    void setLoggerConfig(LoggerConfig &logger_config);
    bool parse(const std::string& config_path);
    
private:
    DaemonConfig daemon_config_;
    ServerConfig server_config_;
    LoggerConfig logger_config_;
};

#endif