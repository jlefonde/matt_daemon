#include "Server.hpp"
#include <unistd.h>

Server::Server(int port, pid_t pid, TintinReporter& logger) : port_(port), pid_(pid), socket_fd_(-1), logger_(logger) {}

Server::~Server(){}

void Server::log(LogLevel log_level, const char *msg)
{
    logger_.log(log_level, msg);
}

void Server::run()
{
    logger_.log(INFO, "Creating server.");

    logger_.log(INFO, "Server created.");

    logger_.log(INFO, "Entering daemon mode.");
    std::string pid_info = "Started. PID: " + std::to_string(pid_); 
    logger_.log(INFO, pid_info.c_str());

    while (true)
    {
        logger_.log(INFO, "...");
        sleep(10);
    }
}

void Server::stop()
{
    // TODO: cleanup server
}
