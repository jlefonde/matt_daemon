#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <csignal>
#include <memory>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>
#include <linux/limits.h>

#include "Config.hpp"
#include "DaemonConfig.hpp"
#include "TintinReporter.hpp"
#include "Server.hpp"

class Daemon
{
public:
    Daemon(const std::string &config_path, const DaemonConfig &config, TintinReporter &logger, Server &server);
    ~Daemon();
    static void handleSignal(int sig);
    void initialize();
    void updateConfig();
    void cleanup();
    void shutdown();
    void log(LogLevel log_level, const char *msg);

private:
    Daemon();
    Daemon(const Daemon &daemon) = delete;
    Daemon &operator=(const Daemon &daemon) = delete;
    void acquireLock();
    void addSignal(int sig);
    void start();
    static Daemon *instance_;
    DaemonConfig config_;
    TintinReporter &logger_;
    Server &server_;
    std::string config_path_;
    int lock_fd_;
};

#endif