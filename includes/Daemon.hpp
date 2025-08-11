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

class Daemon {
    public:
        Daemon();
        ~Daemon();
        static void signal_handler(int sig);
        void initialize();
        void start(int port);
        void cleanup();
        void shutdown();
        void log(LogLevel log_level, const char *msg);
        void showError(const char *msg);
        
        private:
        Daemon(const Daemon &deamon) = delete;
        Daemon &operator=(const Daemon &deamon) = delete;
        void addSignal(int sig);
        void addSignals();
        static Daemon *instance_;
        std::unique_ptr<TintinReporter> logger_;
        std::unique_ptr<Server> server_;
        std::string lock_file_path_;
        int lock_fd_;
        bool has_lock_;
    };

#endif