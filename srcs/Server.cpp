#include "Server.hpp"
#include <unistd.h>

Server::Server(const ServerConfig &config, TintinReporter &logger) : 
    config_(config),
    logger_(logger),
    socket_fd_(-1), 
    epoll_fd_(-1), 
    client_count_(0), 
    is_running_(true)
{
    events_.resize(config_.getMaxConnections());
}

Server::~Server()
{
    for (auto & fd : client_fds_)
        close(fd);

    if (socket_fd_ != -1)
        close(socket_fd_);

    if (epoll_fd_ != -1)
        close(epoll_fd_);
}

ServerConfig Server::getConfig()
{
    return config_;
}

void Server::setConfig(ServerConfig &config)
{
    config_ = config;
}

bool Server::addToEpoll(int fd, uint32_t events)
{
    event_.events = events;
    event_.data.fd = fd;

    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK))
        return false;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event_) == -1)
        return false;

    return true;
}

void Server::createListeningSocket()
{
    struct sockaddr_in serv_addr;

    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == -1)
        throw std::runtime_error(std::string("epoll_create failed: ") + strerror(errno));

    int opt = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error(std::string("setsockopt failed: ") + strerror(errno));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(config_.getPort());
    if (bind(socket_fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        throw std::runtime_error(std::string("bind failed: ") + strerror(errno));

    if (listen(socket_fd_, SOMAXCONN) == -1)
        throw std::runtime_error(std::string("listen failed: ") + strerror(errno));

    if (!addToEpoll(socket_fd_, EPOLLIN))
        throw std::runtime_error(std::string("Failed to add listening socket to epoll: ") + strerror(errno));
}

void Server::createServer()
{
    logger_.log(INFO, "Creating server.");

    epoll_fd_ = epoll_create(1);
    if (epoll_fd_ == -1)
        throw std::runtime_error(std::string("epoll_create failed: ") + strerror(errno));

    createListeningSocket();

    logger_.log(INFO, "Server created.");

    std::string port_info = "Listening on port " + std::to_string(config_.getPort()) + ".";
    logger_.log(INFO, port_info.c_str());
}

bool Server::handleClientConnect()
{
    sockaddr *addr = NULL;
    socklen_t *addr_len = NULL;

    int client_fd = accept(socket_fd_, addr, addr_len);
    if (client_fd == -1)
    {
        std::string error_info = std::string("accept failed: ") + strerror(errno);
        logger_.log(WARNING, error_info.c_str());
        return false;
    }

    if (client_count_ == config_.getMaxConnections())
    {
        std::string error_info = "Maximum number of clients reached, rejecting new connection.";
        logger_.log(WARNING, error_info.c_str());
        close(client_fd);
        return false;
    }

    if (!addToEpoll(client_fd, EPOLLIN))
    {
        std::string error_info = std::string("Failed to add client socket to epoll: ") + strerror(errno);
        logger_.log(WARNING, error_info.c_str());
        close(client_fd);
        return false;
    }

    client_count_++;

    client_fds_.push_back(client_fd);
    std::string client_info = "Client " + std::to_string(client_fd) + " connected.";
    logger_.log(INFO, client_info.c_str());

    return true;
}

void Server::handleClientDisconnect(int fd, ssize_t bytes_read)
{
    if (bytes_read == 0)
    {
        std::string client_info = "Client " + std::to_string(fd) + " disconnected.";
        logger_.log(INFO, client_info.c_str());
    }
    else
    {
        std::string client_info = "Couldn't read client " + std::to_string(fd) + ", closing connection.";
        logger_.log(WARNING, client_info.c_str());
    }
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event_) == -1)
    {
        std::string error_info = std::string("Failed to remove client socket to epoll: ") + strerror(errno);
        logger_.log(WARNING, error_info.c_str());
    }

    close(fd);
    client_count_--;
}

void Server::handleClientData(int fd)
{
    char buffer[BUFFER_SIZE];

    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read <= 0)
        handleClientDisconnect(fd, bytes_read);
    else
    {
        if (buffer[bytes_read - 1] == '\n')
        {
            buffer[bytes_read - 1] = '\0';
            if (bytes_read > 1 && buffer[bytes_read - 2] == '\r')
                buffer[bytes_read - 2] = '\0';
        }
        else
            buffer[bytes_read] = '\0';

        if (!strcmp(buffer, "quit"))
            stop();
        else
        {
            std::string user_input = "Received from client " + std::to_string(fd) + ": \"" + std::string(buffer) + "\"";
            logger_.log(DEBUG, user_input.c_str());
        }
    }
}

void Server::run()
{
    createServer();

    logger_.log(INFO, "Entering daemon mode.");
    std::string pid_info = "Started. PID: " + std::to_string(getpid()) + "."; 
    logger_.log(INFO, pid_info.c_str());

    while (is_running_)
    {
        int nfds = epoll_wait(epoll_fd_, events_.data(), events_.size(), 1000);
        if (nfds == -1)
        {
            if (errno != EINTR)
            {
                std::string error_info = std::string("epoll_wait failed: ") + strerror(errno);
                logger_.log(WARNING, error_info.c_str());
            }
            continue;
        }

        for (int i = 0; i < nfds; i++)
        {
            int fd = events_[i].data.fd;
            if (fd == socket_fd_)
            {
                if (!handleClientConnect())
                    continue;
            }
            else if (events_[i].events & EPOLLIN)
                handleClientData(fd);
        }
    }
}

void Server::stop()
{
    is_running_ = false;
}
