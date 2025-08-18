#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <string.h>

#include "utils.hpp"
#include "DaemonConfig.hpp"
#include "ServerConfig.hpp"
#include "LoggerConfig.hpp"

#define CONFIG_BUFFER_SIZE 3072

class Config
{
public:
    Config();
    Config(const std::string &config_path);
    Config(const Config &config);
    ~Config();
    Config &operator=(const Config &config);
    std::string &getConfigPath();
    DaemonConfig &getDaemonConfig();
    ServerConfig &getServerConfig();
    LoggerConfig &getLoggerConfig();
    void setConfigPath(const std::string &config_path);
    void setDaemonConfig(DaemonConfig &daemon_config);
    void setServerConfig(ServerConfig &server_config);
    void setLoggerConfig(LoggerConfig &logger_config);
    
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::function<void(const std::string &value)>>> config_schema_;
    void initializeSchema();
    void parse();
    std::string config_path_;
    DaemonConfig daemon_config_;
    ServerConfig server_config_;
    LoggerConfig logger_config_;
};

#endif