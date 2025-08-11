#include "Server.hpp"
#include <unistd.h>

Server::Server(int port, pid_t pid, TintinReporter& logger) : port_(port), pid_(pid), socket_fd_(-1), epoll_fd_(-1), client_count_(0), is_running_(true), events_(3), logger_(logger) {}

Server::~Server(){}

bool Server::addToEpoll(int fd, uint32_t events)
{
	event_.events = events;
	event_.data.fd = fd;

	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK))
	{
		logger_.log(ERROR, strerror(errno));
		return false;
	}

	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event_) == -1)
	{
		logger_.log(ERROR, strerror(errno));
		return false;
	}

    return true;
}

void Server::createListeningSocket()
{
    struct sockaddr_in serv_addr;

	socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd_ == -1)
	{
        logger_.log(ERROR, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
        logger_.log(ERROR, strerror(errno));
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_);
	if (bind(socket_fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
        logger_.log(ERROR, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (listen(socket_fd_, SOMAXCONN) == -1)
	{
        logger_.log(ERROR, strerror(errno));
        exit(EXIT_FAILURE);
	}

    if (!addToEpoll(socket_fd_, EPOLLIN))
    {
        logger_.log(ERROR, "Failed to add listening socket to epoll.");
        exit(EXIT_FAILURE);
    }
}

void Server::createServer()
{
    logger_.log(INFO, "Creating server.");

	epoll_fd_ = epoll_create(1);
	if (epoll_fd_ == -1)
    {
		logger_.log(ERROR, strerror(errno));
        exit(EXIT_FAILURE);
    }

    createListeningSocket();

    logger_.log(INFO, "Server created.");

    std::string port_info = "Listening on port " + std::to_string(port_) + ".";
    logger_.log(INFO, port_info.c_str());
}

void Server::run()
{
    createServer();

    logger_.log(INFO, "Entering daemon mode.");
    std::string pid_info = "Started. PID: " + std::to_string(pid_) + "."; 
    logger_.log(INFO, pid_info.c_str());

    sockaddr *addr = NULL;
    socklen_t *addr_len = NULL;
    char buffer[3072];

    while (is_running_)
    {
        int nfds = epoll_wait(epoll_fd_, events_.data(), events_.size(), -1);
		for (int i = 0; i < nfds; i++)
		{
			int fd = events_[i].data.fd;
            if (fd == socket_fd_)
            {
                int client_fd = accept(socket_fd_, addr, addr_len);
                if (client_fd == -1)
                {
                    logger_.log(ERROR, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                if (client_count_ == MAX_CLIENTS)
                {
                    logger_.log(WARNING, "Too much clients");
                    close(client_fd);
                    continue;
                }
                
                if (!addToEpoll(client_fd, EPOLLIN))
                {
                    logger_.log(ERROR, "Failed to add client socket to epoll.");
                    exit(EXIT_FAILURE);
                }
                
                client_count_++;
                std::string client_info = "Client " + std::to_string(client_fd) + " connected.";
                logger_.log(INFO, client_info.c_str());
            }
            else if (events_[i].events & EPOLLIN)
            {
                size_t bytes_read = read(fd, buffer, 3072);
                if (bytes_read <= 0)
                {
                    if (bytes_read == 0)
                    {
                        std::string client_info = "Client " + std::to_string(fd) + " disconnected.";
                        logger_.log(INFO, client_info.c_str());
                    }
                    else
                    {
                        std::string client_info = "Couldn't read client " + std::to_string(fd);
                        logger_.log(WARNING, client_info.c_str());
                    }
                    
                    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event_) == -1)
                    {
                        logger_.log(ERROR, strerror(errno));
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                    close(fd);
                    client_count_--;
                }
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
                        logger_.log(INFO, user_input.c_str());
                    }
                }
            }
        }
    }
}

void Server::stop()
{
    is_running_ = false;
    // TODO: cleanup server
}
