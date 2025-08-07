#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <fstream>

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
        TintinReporter(const std::string& article_name, const std::string& publication_file);
        TintinReporter(const TintinReporter& reporter);
        ~TintinReporter();
        TintinReporter& operator=(const TintinReporter& reporter);
        void log(LogLevel log_level, const char *msg);

    private:
        TintinReporter();
        std::string article_name_;
        std::string publication_file_;
};


#endif