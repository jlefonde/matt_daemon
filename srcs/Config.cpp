#include "Config.hpp"

Config::Config() {}

Config::Config(const std::string &config_path) : config_path_(config_path) {}

Config::Config(const Config &config)
{
    config_path_ = config.config_path_;
    daemon_config_ = config.daemon_config_;
    server_config_ = config.server_config_;
    logger_config_ = config.logger_config_;
}

Config::~Config() {}

Config &Config::operator=(const Config &config)
{
    if (this != &config)
    {
        config_path_ = config.config_path_;
        daemon_config_ = config.daemon_config_;
        server_config_ = config.server_config_;
        logger_config_ = config.logger_config_;
    }

    return *this;
}

std::string &Config::getConfigPath()
{
    return config_path_;
}

DaemonConfig &Config::getDaemonConfig()
{
    return daemon_config_;
}

ServerConfig &Config::getServerConfig()
{
    return server_config_;
}

LoggerConfig &Config::getLoggerConfig()
{
    return logger_config_;
}

void Config::setConfigPath(const std::string &config_path)
{
    config_path_ = config_path;
}

void Config::setDaemonConfig(DaemonConfig &daemon_config)
{
    daemon_config_ = daemon_config;
}

void Config::setServerConfig(ServerConfig &server_config)
{
    server_config_ = server_config;
}

void Config::setLoggerConfig(LoggerConfig &logger_config)
{
    logger_config_ = logger_config;
}

bool Config::parse()
{
    return true;
}
