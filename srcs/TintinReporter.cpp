#include "TintinReporter.hpp"

static const char* log_level_str[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"
};

static std::string getTimestamp(const char *format)
{
    time_t now = time(0);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), format, &timeinfo);

    return std::string(timestamp);
}

TintinReporter::TintinReporter(const std::string& article_name, const LogLevel log_level)
    : article_name_(article_name), log_level_(log_level)
{
    std::string log_folder_path = "/var/log/" + article_name_;
    if (mkdir(log_folder_path.c_str(), 0755) == -1)
    {
        if (errno != EEXIST)
            throw std::runtime_error(std::string(strerror(errno)));
    }

    std::string log_file_path = log_folder_path + "/" + article_name_ + ".log";
    log_file_.open(log_file_path, std::ofstream::out | std::ofstream::app);
    if (!log_file_.is_open())
        throw std::runtime_error(std::string(strerror(errno)));
}

TintinReporter::~TintinReporter()
{
    if (log_file_.is_open())
        log_file_.close();
}

void TintinReporter::log(LogLevel log_level, const char *article)
{
    if (log_level <= log_level_)
    {
        log_file_ << "[" << getTimestamp("%d/%m/%Y-%H:%M:%S") << "] [ " << log_level_str[log_level] << " ] - " << article_name_ << ": " << article << std::endl; 
    }
}
