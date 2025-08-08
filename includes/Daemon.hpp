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

#define LOCK_FILE_PATH "/var/lock/matt_daemon.lock"

class Daemon {
    public:
        static Daemon &getInstance();
        void initialize(int port);
        void start();
        void shutdown();
        void log(LogLevel log_level, const char *msg);

    private:
        Daemon() = default;
        ~Daemon() = default;
        Daemon(const Daemon &) = delete;
        Daemon &operator=(const Daemon &) = delete;
        std::unique_ptr<TintinReporter> logger_;
        int port_;
        int lock_fd_;
};

#endif