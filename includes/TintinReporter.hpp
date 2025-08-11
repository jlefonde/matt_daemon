#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/stat.h>

enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class TintinReporter
{
    public:
        TintinReporter(const std::string& article_name, const LogLevel log_level);
        ~TintinReporter();
        void log(LogLevel log_level, const char *article);
        
    private:
        TintinReporter();
        TintinReporter(const TintinReporter& reporter) = delete;
        TintinReporter& operator=(const TintinReporter& reporter) = delete;
        std::string article_name_;
        std::ofstream log_file_;
        LogLevel log_level_;
};


#endif