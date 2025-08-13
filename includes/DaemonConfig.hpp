#ifndef DAEMON_CONFIG_HPP
#define DAEMON_CONFIG_HPP

#include <string>
#include <stdexcept>

#include "utils.hpp"

class DaemonConfig
{
public:
    DaemonConfig();
    DaemonConfig(const DaemonConfig &daemon_config);
    ~DaemonConfig();
    DaemonConfig &operator=(const DaemonConfig &daemon_config);
    std::string &getLockFile();
    std::string &getPidFile();
    void setLockFile(const std::string &lock_file);
    void setPidFile(const std::string &pid_file);

private:
    std::string lock_file_;
    std::string pid_file_;
};

#endif