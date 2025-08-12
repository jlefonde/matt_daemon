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
    if (log_file_.empty() || log_file_.back() == '/')
        return;

    createParentDirectories(log_file_);

    log_file_ofs_.open(log_file_, std::ofstream::out | std::ofstream::app);
    if (!log_file_ofs_.is_open())
        return;

    if (stat(log_file_.c_str(), &stats_))
        return;
}

TintinReporter::TintinReporter(LogLevel log_level, const std::string& name, const std::string& log_file, 
    bool auto_rotate, size_t rotate_interval, size_t rotate_size, size_t rotate_count) :
    log_level_(log_level),
    name_(name),
    log_file_(log_file),
    auto_rotate_(auto_rotate),
    rotate_interval_(rotate_interval),
    rotate_size_(rotate_size),
    rotate_count_(rotate_count),
    cur_rotate_count_(0)
{
    openLogFile();
}

TintinReporter::~TintinReporter()
{
    if (log_file_ofs_.is_open())
        log_file_ofs_.close();
}

void TintinReporter::rotateLogs(size_t log_msg_size)
{
    time_t next_rotate = stats_.st_ctime + (rotate_interval_ * 3600);

    if (((stats_.st_size + log_msg_size) > (rotate_size_ * 1024 * 1024)) || (time(NULL) >= next_rotate))
    {
        log_file_ofs_.close();

        cur_rotate_count_++;

        if (cur_rotate_count_ == rotate_count_)
        {
            std::string last_file = log_file_ + "." + std::to_string(cur_rotate_count_);
            remove(last_file.c_str());

            for (int i = rotate_count_ - 1; i > 0; i--)
            {
                std::string old_file = log_file_ + "." + std::to_string(i);
                std::string new_filename = log_file_ + "." + std::to_string(i + 1);
                rename(old_file.c_str(), new_filename.c_str());
            }
            std::string new_filename = log_file_ + ".1";
            rename(log_file_.c_str(), new_filename.c_str());

            cur_rotate_count_ = 0;
        }
        else
        {
            std::string new_filename = log_file_ + "." + std::to_string(cur_rotate_count_);
            rename(log_file_.c_str(), new_filename.c_str());
        }

        openLogFile();
        stat(log_file_.c_str(), &stats_);
    }
}

void TintinReporter::log(LogLevel log_level, const char *msg)
{
    if (log_level > log_level_)
        return;

    std::string log_msg = "[" + getTimestamp("%d/%m/%Y-%H:%M:%S") + "] [ " + log_level_str[log_level] + " ] - " 
            + name_ + ": " + msg;

    if (auto_rotate_)
        rotateLogs(log_msg.size() + 1);

    if (log_file_ofs_.is_open())
        log_file_ofs_ << log_msg << std::endl;

    stat(log_file_.c_str(), &stats_);
}
