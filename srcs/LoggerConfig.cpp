#include "LoggerConfig.hpp"

const char *log_level_str_[] = {
    [DEBUG] = "DEBUG",
    [INFO] = "INFO",
    [WARNING] = "WARNING",
    [ERROR] = "ERROR"
};

LoggerConfig::LoggerConfig() : 
    log_file_("/var/log/matt_daemon/matt_daemon.log"),
    log_level_(ERROR),
    auto_rotate_(true),
    rotate_interval_(24),
    rotate_size_(1024),
    rotate_count_(5) {}

LoggerConfig::LoggerConfig(const LoggerConfig &logger_config)
{
    log_file_ = logger_config.log_file_;
    log_level_ = logger_config.log_level_;
    auto_rotate_ = logger_config.auto_rotate_;
    rotate_interval_ = logger_config.rotate_interval_;
    rotate_size_ = logger_config.rotate_size_;
    rotate_count_ = logger_config.rotate_count_;
}

LoggerConfig::~LoggerConfig() {}

LoggerConfig &LoggerConfig::operator=(const LoggerConfig &logger_config)
{
    if (this != &logger_config)
    {
        log_file_ = logger_config.log_file_;
        log_level_ = logger_config.log_level_;
        auto_rotate_ = logger_config.auto_rotate_;
        rotate_interval_ = logger_config.rotate_interval_;
        rotate_size_ = logger_config.rotate_size_;
        rotate_count_ = logger_config.rotate_count_;
    }

    return *this;
}

std::string &LoggerConfig::getLogFile()
{
    return log_file_;
}

LogLevel LoggerConfig::getLogLevel()
{
    return log_level_;
}

bool LoggerConfig::isAutoRotateEnabled()
{
    return auto_rotate_;
}

size_t LoggerConfig::getRotateInterval()
{
    return rotate_interval_;
}

size_t LoggerConfig::getRotateSize()
{
    return rotate_size_;
}

size_t LoggerConfig::getRotateCount()
{
    return rotate_count_;
}

void LoggerConfig::setLogFile(const std::string &log_file)
{    
    if (!isAbsolutePath(log_file))
        throw std::invalid_argument("'" + log_file + "' must be an absolute path.");

    log_file_ = log_file;
}

void LoggerConfig::setLogLevel(LogLevel log_level)
{
    log_level_ = log_level;
}

void LoggerConfig::setAutoRotate(bool auto_rotate)
{
    auto_rotate_ = auto_rotate;
}

void LoggerConfig::setRotateInterval(size_t rotate_interval)
{
    if (rotate_interval == 0)
        throw std::invalid_argument("Rotate interval must be greater than 0.");

    rotate_interval_ = rotate_interval;
}

void LoggerConfig::setRotateSize(size_t rotate_size)
{
    if (rotate_size == 0)
        throw std::invalid_argument("Rotate size must be greater than 0.");

    rotate_size_ = rotate_size;
}

void LoggerConfig::setRotateCount(size_t rotate_count)
{
    if (rotate_count == 0)
        throw std::invalid_argument("Rotate count must be greater than 0.");

    rotate_count_ = rotate_count;
}

void LoggerConfig::setLogLevelFromStr(const std::string &log_level_str)
{
    for (size_t i = 0; i < sizeof(log_level_str_)/sizeof(log_level_str_[0]); i++)
    {
        if (log_level_str_[i] && log_level_str == log_level_str_[i])
        {
            log_level_ = static_cast<LogLevel>(i);
            return;
        }
    }

    throw std::invalid_argument("Invalid log level provided: " + log_level_str);
}

const char *LoggerConfig::getLogLevelStr(LogLevel log_level)
{
    return log_level_str_[log_level];
}
