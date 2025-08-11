#ifndef SERVER_HPP
#define SERVER_HPP

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <stdexcept>

#include "TintinReporter.hpp"

#define MAX_CLIENTS 3
#define BUFFER_SIZE 3072

class Server 
{
public:
    Server(int port, TintinReporter& logger);
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
    bool handleClientConnect();
    void handleClientDisconnect(int fd, ssize_t bytes_read);
    void handleClientData(int fd);
    int port_;
    int socket_fd_;
    int epoll_fd_;
    size_t client_count_;
    bool is_running_;
    struct epoll_event event_;
    std::vector<struct epoll_event> events_;
    TintinReporter& logger_;
};

#endif