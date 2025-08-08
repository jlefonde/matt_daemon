#ifndef SERVER_HPP
#define SERVER_HPP

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

#include "TintinReporter.hpp"

#define MAX_CLIENTS 3

class Server 
{
    public:
        Server(int port, pid_t pid, TintinReporter& logger);
        ~Server();
        void run();
        void stop();
        
    private:
        Server() = delete;
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        void createServer();
        bool addToEpoll(int fd, uint32_t events);
        void createListeningSocket();
        int port_;
        pid_t pid_;
        int socket_fd_;
        int epoll_fd_;
        bool is_running_;
        struct epoll_event event_;
        std::vector<struct epoll_event> events_;
        TintinReporter& logger_;
};

#endif