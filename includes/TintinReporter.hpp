#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/stat.h>

#include "LoggerConfig.hpp"

class TintinReporter
{
public:
    TintinReporter(const LoggerConfig &config, const std::string& name);
    ~TintinReporter();
    LoggerConfig getConfig();
    void setConfig(LoggerConfig &config);
    void resetAutoRotate();
    void log(LogLevel log_level, const char *msg);
    
private:
    TintinReporter();
    TintinReporter(const TintinReporter& reporter) = delete;
    TintinReporter& operator=(const TintinReporter& reporter) = delete;
    void openLogFile();
    void rotateLogs(size_t msg_size);
    LoggerConfig config_;
    std::string name_;
    std::ofstream log_file_ofs_;
    size_t rotate_count_;
    struct stat stats_;
};


#endif