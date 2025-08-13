#include "DaemonConfig.hpp"

DaemonConfig::DaemonConfig() : lock_file_("/var/lock/matt_daemon.lock"), pid_file_("/var/run/matt_daemon.pid") {}

DaemonConfig::DaemonConfig(const DaemonConfig &daemon_config)
{
    lock_file_ = daemon_config.lock_file_;
    pid_file_ = daemon_config.pid_file_;
}

DaemonConfig::~DaemonConfig() {}

DaemonConfig &DaemonConfig::operator=(const DaemonConfig &daemon_config)
{
    if (this != &daemon_config)
    {
        lock_file_ = daemon_config.lock_file_;
        pid_file_ = daemon_config.pid_file_;
    }

    return *this;
}

std::string &DaemonConfig::getLockFile()
{
    return lock_file_;
}

std::string &DaemonConfig::getPidFile()
{
    return pid_file_;
}

void DaemonConfig::setLockFile(const std::string &lock_file)
{
    if (!isAbsolutePath(lock_file))
        throw std::invalid_argument("'" + lock_file + "' must be an absolute path.");

    lock_file_ = lock_file;
}

void DaemonConfig::setPidFile(const std::string &pid_file)
{
    if (!isAbsolutePath(pid_file))
        throw std::invalid_argument("'" + pid_file + "' must be an absolute path.");

    pid_file_ = pid_file;
}
