#ifndef SERVER_HPP
#define SERVER_HPP

#include "TintinReporter.hpp"

class Server 
{
    public:
        Server(int port, pid_t pid, TintinReporter& logger);
        ~Server();
        void log(LogLevel log_level, const char *msg);
        void run();
        void stop();
        
    private:
        Server() = delete;
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        int port_;
        pid_t pid_;
        int socket_fd_;
        TintinReporter& logger_;
};

#endif