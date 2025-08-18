#include "Daemon.hpp"

Daemon* Daemon::instance_ = nullptr;

Daemon::Daemon(const std::string &config_path, const DaemonConfig &config, TintinReporter &logger, Server &server) :
    config_(config),
    logger_(logger),
    server_(server),
    config_path_(config_path),
    lock_fd_(-1) {}

Daemon::~Daemon()
{
    if (lock_fd_ != -1)
        close(lock_fd_);
}

void Daemon::handleSignal(int sig)
{
    std::string signal_info = "Signal handler - " + std::string(strsignal(sig)) + " (" + std::to_string(sig) + ").";
    instance_->log(INFO, signal_info.c_str());

    if (sig == SIGHUP)
        instance_->updateConfig();
    else
        instance_->server_.stop();
}

void Daemon::updateConfig()
{
    try {
        logger_.log(INFO, "Updating configuration.");

        if (config_path_.empty()) {
            logger_.log(WARNING, "No configuration file specified, cannot update configuration.");
            return;
        }

        DaemonConfig daemon_config = config_;
        ServerConfig server_config = server_.getConfig();
        LoggerConfig logger_config = logger_.getConfig();
        
        // Validate configuration file syntax and structure
        Config config(config_path_);

        // Parse config with current values, applying only non-restart settings
        Config newConfig(config_path_, daemon_config, server_config, logger_config);

        config_ = newConfig.getDaemonConfig();
        server_.setConfig(newConfig.getServerConfig());

        if (logger_.getConfig().getLogFile() == newConfig.getLoggerConfig().getLogFile())
            logger_.resetAutoRotate();

        logger_.setConfig(newConfig.getLoggerConfig());

        logger_.log(INFO, "Configuration updated.");
    }
    catch (const std::exception &e)
    {
        std::string error_msg = "Failed to update configuration: " + std::string(e.what());
        logger_.log(ERROR, error_msg.c_str());
    }
}

void Daemon::addSignal(int sig)
{
    if (signal(sig, &handleSignal) == SIG_ERR)
    {
        std::string signal_error = "signal " + std::string(strsignal(sig)) + " (" + std::to_string(sig) + ") failed: " 
            + strerror(errno);
        throw std::runtime_error(signal_error);
    }
}

void Daemon::acquireLock()
{
    lock_fd_ = open(config_.getLockFile().c_str(), O_CREAT | O_RDWR, 0644);
    if (lock_fd_ == -1)
        throw std::runtime_error(std::string("open lock failed: ") + strerror(errno));

    if (flock(lock_fd_, LOCK_EX | LOCK_NB) == -1)
    {
        if (errno == EWOULDBLOCK)
            std::cerr << "Error: Another instance is already running." << std::endl;

        throw std::runtime_error(std::string("flock failed: ") + strerror(errno));
    }
}

void Daemon::initialize()
{
    try {
        instance_ = this;

        acquireLock();

        addSignal(SIGHUP);
        addSignal(SIGINT);
        addSignal(SIGQUIT);
        addSignal(SIGILL);
        addSignal(SIGTRAP);
        addSignal(SIGABRT);
        addSignal(SIGUSR1);
        addSignal(SIGSEGV);
        addSignal(SIGUSR2);
        addSignal(SIGPIPE);
        addSignal(SIGALRM);
        addSignal(SIGTERM);

        start();
    }
    catch (const std::exception &e) {
        logger_.log(ERROR, e.what());
        logger_.log(INFO, "Quitting.");
    }
}

void Daemon::start()
{
    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error(std::string("fork 1 failed: ") + strerror(errno));
    else if (pid == 0)
    {
        if (setsid() == -1)
            throw std::runtime_error(std::string("setsid failed: ") + strerror(errno));

        pid = fork();
        if (pid < 0)
            throw std::runtime_error(std::string("fork 2 failed: ") + strerror(errno));
        else if (pid == 0)
        {
            int pid_fd = open(config_.getPidFile().c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
            if (pid_fd == -1)
                throw std::runtime_error(std::string("open pid failed: ") + strerror(errno));

            std::string pid_str = std::to_string(getpid()) + "\n";
            write(lock_fd_, pid_str.c_str(), pid_str.size());
            write(pid_fd, pid_str.c_str(), pid_str.size());

            if (chdir("/") == -1)
                throw std::runtime_error(std::string("chdir failed: ") + strerror(errno));

            umask(0);

            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);

            if (open("/dev/null", O_RDWR) == -1)
                throw std::runtime_error(std::string("open /dev/null failed: ") + strerror(errno));

            if (dup(0) == -1)
                throw std::runtime_error(std::string("dup failed: ") + strerror(errno));

            if (dup(0) == -1)
                throw std::runtime_error(std::string("dup failed: ") + strerror(errno));

            server_.run();

            cleanup();
        }
        else
            return;
    }
    else
        return;
}

void Daemon::cleanup()
{
    if (remove(config_.getLockFile().c_str()) == -1)
    {
        std::string warning_msg = std::string("remove lock failed: ") + strerror(errno);
        logger_.log(ERROR, warning_msg.c_str());
    }
    
    if (remove(config_.getPidFile().c_str()) == -1)
    {
        std::string warning_msg = std::string("remove pid failed: ") + strerror(errno);
        logger_.log(ERROR, warning_msg.c_str());
    }
    
    logger_.log(INFO, "Quitting.");
}

void Daemon::log(LogLevel log_level, const char *msg)
{
    logger_.log(log_level, msg);
}
