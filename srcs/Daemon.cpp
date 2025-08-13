#include "Daemon.hpp"

Daemon* Daemon::instance_ = nullptr;

Daemon::Daemon() :
    logger_(std::make_unique<TintinReporter>(ERROR, "matt_daemon", "/var/log/matt_daemon/matt_daemon.log")),
    server_(nullptr),
    lock_file_("/var/lock/matt_daemon.lock"),
    pid_file_("/var/run/matt_daemon.pid"),
    lock_fd_(-1) {}

Daemon::~Daemon()
{
    if (lock_fd_ != -1)
        close(lock_fd_);
}

void Daemon::handleSignal(int sig)
{
    if (sig != SIGHUP)
    {
        std::string signal_info = "Signal handler - " + std::string(strsignal(sig)) + " (" + std::to_string(sig) + ").";
        instance_->log(INFO, signal_info.c_str());
        instance_->server_->stop();
        instance_->cleanup();
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

void Daemon::addSignals()
{
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
}

void Daemon::initialize(Config &config)
{
    config_ = &config.getDaemonConfig();
    instance_ = this;

    logger_->log(INFO, "Started.");

    lock_fd_ = open(lock_file_.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (lock_fd_ == -1)
        throw std::runtime_error(std::string("open lock failed: ") + strerror(errno));

    if (flock(lock_fd_, LOCK_EX | LOCK_NB) == -1)
    {
        if (errno == EWOULDBLOCK)
            std::cerr << "Error: Another instance is already running." << std::endl;

        throw std::runtime_error(std::string("flock failed: ") + strerror(errno));
    }

    addSignals();

    start(4242);
}

void Daemon::start(int port)
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
            int pid_fd = open(pid_file_.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
            if (pid_fd == -1)
                throw std::runtime_error(std::string("open pid failed: ") + strerror(errno));

            std::string pid_str = std::to_string(getpid()) + "\n";
            write(lock_fd_, pid_str.c_str(), pid_str.size());
            write(pid_fd, pid_str.c_str(), pid_str.size());

            if (chdir("/") == -1)
                throw std::runtime_error(std::string("chdir failed: ") + strerror(errno));

            umask(0);

            close(STDIN_FILENO);
            close(STDERR_FILENO);
            close(STDOUT_FILENO);

            if (open("/dev/null", O_RDWR) == -1)
                throw std::runtime_error(std::string("open /dev/null failed: ") + strerror(errno));

            if (dup(0) == -1)
                throw std::runtime_error(std::string("dup failed: ") + strerror(errno));

            if (dup(0) == -1)
                throw std::runtime_error(std::string("dup failed: ") + strerror(errno));

            server_ = std::make_unique<Server>(port, *logger_);
            server_->run();

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
    logger_->log(INFO, "Quitting.");

    remove(lock_file_.c_str());
    remove(pid_file_.c_str());
}

void Daemon::log(LogLevel log_level, const char *msg)
{
    logger_->log(log_level, msg);
}

void Daemon::showError(const char *msg)
{
    if (logger_)
    {
        logger_->log(ERROR, msg);
        logger_->log(INFO, "Quitting.");
    }
    else
        std::cerr << "Error: " << msg << std::endl; 
}
