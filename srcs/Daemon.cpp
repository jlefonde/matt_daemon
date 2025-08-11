#include "Daemon.hpp"

Daemon* Daemon::instance_ = nullptr;

Daemon::Daemon() : logger_(nullptr),
    server_(nullptr),
    lock_file_path_("/var/lock/matt_daemon.lock"),
    pid_file_path_("/var/run/matt_daemon.pid"),
    lock_fd_(-1),
    has_lock_(false) {}

Daemon::~Daemon()
{
    if (logger_)
        logger_->log(INFO, "Quitting.");
}

void Daemon::signal_handler(int sig)
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
    if (signal(sig, &signal_handler) == SIG_ERR)
    {
        std::string signal_error = "signal " + std::string(strsignal(sig)) + " (" + std::to_string(sig) + ") failed: " + strerror(errno);
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

void Daemon::initialize()
{
    if (geteuid() != 0)
        throw std::runtime_error("The program must be run with root privileges.");

    instance_ = this;

    logger_ = std::make_unique<TintinReporter>("matt_daemon", ERROR);
    logger_->log(INFO, "Started.");

    addSignals();

    lock_fd_ = open(lock_file_path_.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (lock_fd_ == -1)
        throw std::runtime_error(std::string("open lock failed: ") + strerror(errno));

    if (flock(lock_fd_, LOCK_EX | LOCK_NB) == -1)
    {
        if (errno == EWOULDBLOCK)
            std::cerr << "Error: Another instance is already running." << std::endl;

        throw std::runtime_error(std::string("flock failed: ") + strerror(errno));
    }

    has_lock_ = true;
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
            int pid_fd = open(pid_file_path_.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
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
    if (lock_fd_ != -1 && has_lock_)
    {
        flock(lock_fd_, LOCK_UN);
        close(lock_fd_);
        remove(lock_file_path_.c_str());
        remove(pid_file_path_.c_str());
    }
}

void Daemon::log(LogLevel log_level, const char *msg)
{
    logger_->log(log_level, msg);
}

void Daemon::showError(const char *msg)
{
    if (logger_)
        logger_->log(ERROR, msg);
    else
        std::cerr << "Error: " << msg << std::endl; 
}
