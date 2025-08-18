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

static void createParentDirectories(const std::string& path)
{
    size_t last_slash = path.find_last_of('/');
    if (last_slash == std::string::npos)
        return;

    std::string dir_path = path.substr(0, last_slash);
    for (size_t i = 1; i < dir_path.length(); i++)
    {
        if (dir_path[i] == '/')
        {
            std::string sub_path = dir_path.substr(0, i);
            if (mkdir(sub_path.c_str(), 0755) == -1 && errno != EEXIST)
                return;
        }
    }

    if (mkdir(dir_path.c_str(), 0755) == -1 && errno != EEXIST)
        return;
}

void TintinReporter::openLogFile()
{
    std::string log_file = config_.getLogFile();
    if (log_file.empty() || log_file.back() == '/')
        return;

    createParentDirectories(log_file);

    log_file_ofs_.open(log_file, std::ofstream::out | std::ofstream::app);

    stat(log_file.c_str(), &stats_);
}

TintinReporter::TintinReporter(const LoggerConfig &config, const std::string& name) :
    config_(config),
    name_(name),
    rotate_count_(0)
{
    openLogFile();
    log(INFO, "Started.");
}

TintinReporter::~TintinReporter()
{
    if (log_file_ofs_.is_open())
        log_file_ofs_.close();
}

LoggerConfig TintinReporter::getConfig()
{
    return config_;
}

void TintinReporter::setConfig(LoggerConfig &config)
{
    config_ = config;
}

void TintinReporter::rotateLogs(size_t log_msg_size)
{
    time_t next_rotate = stats_.st_ctime + (config_.getRotateInterval() * 3600);

    if (((stats_.st_size + log_msg_size) > (config_.getRotateSize() * 1024)) || (time(NULL) >= next_rotate))
    {
        log_file_ofs_.close();
        
        std::string log_file = config_.getLogFile();
        
        if (rotate_count_ == config_.getRotateCount())
        {
            std::string oldest_file = log_file + "." + std::to_string(config_.getRotateCount());
            remove(oldest_file.c_str());
        }

        size_t max_rotate = std::min(rotate_count_, config_.getRotateCount() - 1);
        for (size_t i = max_rotate; i > 0; i--)
        {
            std::string old_file = log_file + "." + std::to_string(i);
            std::string new_file = log_file + "." + std::to_string(i + 1);
            rename(old_file.c_str(), new_file.c_str());
        }

        rename(log_file.c_str(), (log_file + ".1").c_str());

        if (rotate_count_ < config_.getRotateCount())
            rotate_count_++;

        openLogFile();
        stat(log_file.c_str(), &stats_);
    }
}

void TintinReporter::resetAutoRotate()
{
    rotate_count_ = 0;
}

void TintinReporter::log(LogLevel log_level, const char *msg)
{
    std::string log_file = config_.getLogFile();

    if (log_level > config_.getLogLevel())
        return;

    std::string log_msg = "[" + getTimestamp("%d/%m/%Y-%H:%M:%S") + "] [ " + log_level_str[log_level] + " ] - " 
            + name_ + ": " + msg;

    struct stat stats;
    if (stat(log_file.c_str(), &stats) != 0 || stats.st_ino != stats_.st_ino)
    {
        if (log_file_ofs_.is_open())
            log_file_ofs_.close();

        openLogFile();
        stat(log_file.c_str(), &stats_);
    }

    if (config_.isAutoRotateEnabled())
        rotateLogs(log_msg.size() + 1);

    if (log_file_ofs_.is_open())
        log_file_ofs_ << log_msg << std::endl;

    stat(log_file.c_str(), &stats_);
}
