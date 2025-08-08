#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <csignal>
#include <memory>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "TintinReporter.hpp"
#include "Server.hpp"

#define LOCK_FILE_PATH "/var/lock/matt_daemon.lock"

class Daemon {
    public:
        static Daemon &getInstance();
        void initialize();
        void start(int port);
        void shutdown();
        void log(LogLevel log_level, const char *msg);

    private:
        Daemon() = default;
        ~Daemon() = default;
        Daemon(const Daemon &) = delete;
        Daemon &operator=(const Daemon &) = delete;
        std::unique_ptr<TintinReporter> logger_;
        std::unique_ptr<Server> server_;
        int lock_fd_;
        pid_t pid_;
};

#endif