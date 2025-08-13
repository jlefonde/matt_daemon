#ifndef DAEMON_CONFIG_HPP
#define DAEMON_CONFIG_HPP

#include <string>

class DaemonConfig
{
public:
    DaemonConfig();
    DaemonConfig(const DaemonConfig &daemon_config);
    ~DaemonConfig();
    DaemonConfig &operator=(const DaemonConfig &daemon_config);
    
private:
    std::string lock_file_;
    std::string pid_file_;
};

#endif