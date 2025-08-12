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
        TintinReporter(LogLevel log_level, const std::string& name, const std::string& log_file, bool auto_rotate = true,
            size_t rotate_interval = 24, size_t rotate_size = 10, size_t rotate_count = 5);
        ~TintinReporter();
        void log(LogLevel log_level, const char *msg);
        
    private:
        TintinReporter();
        TintinReporter(const TintinReporter& reporter) = delete;
        TintinReporter& operator=(const TintinReporter& reporter) = delete;
        void openLogFile();
        void rotateLogs(size_t msg_size);
        LogLevel log_level_;
        std::string name_;
        std::string log_file_;
        std::ofstream log_file_ofs_;
        bool auto_rotate_;
        size_t rotate_interval_;
        size_t rotate_size_;
        size_t rotate_count_;
        size_t cur_rotate_count_;
        struct stat stats_;
};


#endif