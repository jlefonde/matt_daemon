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

#include "Config.hpp"
#include "DaemonConfig.hpp"
#include "TintinReporter.hpp"
#include "Server.hpp"

class Daemon
{
public:
    Daemon();
    ~Daemon();
    static void handleSignal(int sig);
    bool initialize(int argc, char **argv);
    void updateConfig();
    void cleanup();
    void shutdown();
    void log(LogLevel log_level, const char *msg);
    void showError(const char *msg);

private:
    Daemon(const Daemon &daemon) = delete;
    Daemon &operator=(const Daemon &daemon) = delete;
    bool parseArgs(int argc, char **argv);
    void addSignal(int sig);
    void addSignals();
    void start();
    static Daemon *instance_;
    DaemonConfig *config_;
    std::string config_path_;
    std::unique_ptr<TintinReporter> logger_;
    std::unique_ptr<Server> server_;
    int lock_fd_;
};

#endif