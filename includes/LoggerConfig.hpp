#ifndef LOGGER_CONFIG_HPP
#define LOGGER_CONFIG_HPP

#include <string>
#include <stdexcept>

#include "utils.hpp"

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
    std::string &getLogFile();
    LogLevel getLogLevel();
    bool isAutoRotateEnabled();
    size_t getRotateInterval();
    size_t getRotateSize();
    size_t getRotateCount();
    void setLogFile(const std::string &log_file);
    void setLogLevel(LogLevel log_level);
    void setAutoRotate(bool auto_rotate);
    void setRotateInterval(size_t rotate_interval);
    void setRotateSize(size_t rotate_size);
    void setRotateCount(size_t rotate_count);
    void setLogLevelFromStr(const std::string &log_level_str);
    const char *getLogLevelStr(LogLevel log_level);
    
private:
    static const char *log_level_str_[];
    std::string log_file_;
    LogLevel log_level_;
    bool auto_rotate_;
    size_t rotate_interval_;
    size_t rotate_size_;
    size_t rotate_count_;
};

#endif