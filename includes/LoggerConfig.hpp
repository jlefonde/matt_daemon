#ifndef LOGGER_CONFIG_HPP
#define LOGGER_CONFIG_HPP

#include <string>

enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class LoggerConfig
{
public:
    LoggerConfig();
    LoggerConfig(const LoggerConfig &logger_config);
    ~LoggerConfig();
    LoggerConfig &operator=(const LoggerConfig &logger_config);

private:
    std::string log_file_;
    LogLevel log_level_;
    bool auto_rotate_;
    size_t rotate_interval_;
    size_t rotate_size_;
    size_t rotate_count_;
};

#endif