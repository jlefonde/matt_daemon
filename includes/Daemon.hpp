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

#include "TintinReporter.hpp"
#include "Server.hpp"

class Daemon
{
public:
    Daemon();
    ~Daemon();
    static void handleSignal(int sig);
    void initialize(const std::string& config_path);
    void cleanup();
    void shutdown();
    void log(LogLevel log_level, const char *msg);
    void showError(const char *msg);
    
private:
    Daemon(const Daemon &daemon) = delete;
    Daemon &operator=(const Daemon &daemon) = delete;
    void addSignal(int sig);
    bool parseConfig(const std::string& config_path);
    void start(int port);
    void addSignals();
    static Daemon *instance_;
    std::unique_ptr<TintinReporter> logger_;
    std::unique_ptr<Server> server_;
    std::string config_path_;
    std::string lock_file_;
    std::string pid_file_;
    int lock_fd_;
};

#endif