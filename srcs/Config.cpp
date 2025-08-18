#include "Config.hpp"
#include <iostream>

static const std::string parseSectionHeader(const std::string &line, size_t line_nbr)
{
    size_t section_end = line.find_first_of(']');
    if (section_end == std::string::npos)
        throw std::invalid_argument("Section has no end character ']' at line " + std::to_string(line_nbr) + ".");

    std::string section_name = line.substr(1, section_end - 1);
    if (section_name.empty())
        throw std::invalid_argument("Section name can not be empty at line " + std::to_string(line_nbr) + ".");

    std::string trailing_content = (section_end + 1 < line.length()) ? line.substr(section_end + 1) : "";
    
    if (!trailing_content.empty())
        ltrim(trailing_content);

    if (!trailing_content.empty() && trailing_content.front() != ';' && trailing_content.front() != '#')
        throw std::invalid_argument("No content expected after the section end ']' at line " + std::to_string(line_nbr) + ".");

    return section_name;
}

void Config::initializeSchema() {
    config_schema_ = {
        {"daemon", {
            {"lock_file", std::make_pair([this](const std::string &value) { daemon_config_.setLockFile(value); }, true)},
            {"pid_file", std::make_pair([this](const std::string &value) { daemon_config_.setPidFile(value); }, true)}
        }},
        {"server", {
            {"port", std::make_pair([this](const std::string &value) { server_config_.setPort(std::stoul(value)); }, true)},
            {"max_connections", std::make_pair([this](const std::string &value) { server_config_.setMaxConnections(std::stoul(value)); }, true)}
        }},
        {"logger", {
            {"log_file", std::make_pair([this](const std::string &value) { logger_config_.setLogFile(value); }, false)},
            {"log_level", std::make_pair([this](const std::string &value) { logger_config_.setLogLevelFromStr(value); }, false)},
            {"auto_rotate", std::make_pair([this](const std::string &value) { logger_config_.setAutoRotateFromStr(value); }, false)},
            {"rotate_interval", std::make_pair([this](const std::string &value) { logger_config_.setRotateInterval(std::stoul(value)); }, false)},
            {"rotate_size", std::make_pair([this](const std::string &value) { logger_config_.setRotateSize(std::stoul(value)); }, false)},
            {"rotate_count", std::make_pair([this](const std::string &value) { logger_config_.setRotateCount(std::stoul(value)); }, false)}
        }}
    };
}

Config::Config()
{
    initializeSchema();
}

Config::Config(const std::string &config_path) : config_path_(config_path)
{
    initializeSchema();

    bool updating_config = false;
    parse(updating_config);
}

Config::Config(const std::string &config_path, DaemonConfig &daemon_config, ServerConfig &server_config,
    LoggerConfig &logger_config) : 
    config_path_(config_path),
    daemon_config_(daemon_config),
    server_config_(server_config),
    logger_config_(logger_config)
{
    initializeSchema();

    bool updating_config = true;
    parse(updating_config);
}

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

void Config::parse(bool updating_config)
{
    std::ifstream config_ifs;

    config_ifs.open(config_path_, std::ifstream::in);
    if (!config_ifs.is_open())
        throw std::runtime_error(std::string("open config failed: ") + strerror(errno));

    char line[CONFIG_BUFFER_SIZE];
    size_t line_nbr = 0;
    std::string section_name;
    auto current_section = config_schema_.end();

    while (config_ifs.getline(line, CONFIG_BUFFER_SIZE))
    {
        line_nbr++;

        std::string line_str = std::string(line);

        if (line_str.empty())
            continue;

        ltrim(line_str);
        switch (line_str.front())
        {
            case ';':
            case '#':
                continue;
            case '[':
            {
                section_name = parseSectionHeader(line_str, line_nbr);
                current_section = config_schema_.find(section_name);
                if (current_section == config_schema_.end())
                    throw std::invalid_argument("Invalid section name at line " + std::to_string(line_nbr) + ".");

                break;
            }
            default:
            {
                if (section_name.empty())
                    throw std::invalid_argument("Keys without section are not allowed at line " + std::to_string(line_nbr) + ".");

                std::stringstream ss(line_str);
                std::string key;
                std::string value;

                getline(ss, key, '=');
                getline(ss, value);

                trim(key);
                trim(value);

                auto section_key = current_section->second.find(key);
                if (section_key == current_section->second.end())
                    throw std::invalid_argument("Invalid section key at line " + std::to_string(line_nbr) + ".");

                if (!updating_config || (updating_config && !section_key->second.second))
                    section_key->second.first(value);
            }
        }
    }
}
