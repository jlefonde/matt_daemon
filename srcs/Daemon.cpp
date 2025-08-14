#include "Daemon.hpp"

Daemon* Daemon::instance_ = nullptr;

Daemon::Daemon() :
    logger_(nullptr),
    server_(nullptr),
    lock_fd_(-1) {}

Daemon::~Daemon()
{
    if (lock_fd_ != -1)
        close(lock_fd_);
}

static void showHelper()
{
    std::cout << "Usage: MattDaemon [OPTIONS]\n"
              << "\n"
              << "A daemon with a server that listen on a specific port and register incoming data in a log file.\n"
              << "\n"
              << "OPTIONS:\n"
              << "  -c <config_file>    Specify configuration file path\n"
              << "  -h                  Show this help message and exit\n"
              << std::endl;
}

void Daemon::handleSignal(int sig)
{
    if (sig == SIGHUP)
        instance_->updateConfig();
    else
    {
        std::string signal_info = "Signal handler - " + std::string(strsignal(sig)) + " (" + std::to_string(sig) + ").";
        instance_->log(INFO, signal_info.c_str());
        instance_->server_->stop();
        instance_->cleanup();
    }
}

void Daemon::updateConfig()
{

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

bool Daemon::parseArgs(int argc, char **argv)
{
    if (geteuid() != 0)
    {
        std::cerr << "Error: Root privileges are required to run. Please run with sudo or as root user." << std::endl;
        return false;
    }

    bool config_mode = false;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-c") == 0)
                config_mode = true;
            else if (strcmp(argv[i], "-h") == 0)
            {
                showHelper();
                return false;
            }
            else
            {
                std::cerr << "Error: Unknown option '" << argv[i] << "'. Use -h for help." << std::endl;
                return false;
            }
        }
        else if (config_mode)
        {
            config_path_ = argv[i];
            config_mode = false;
        }
        else
        {
            std::cerr << "Error: Unexpected argument '" << argv[i] << "'. Use -h for help." << std::endl;
            return false;
        }
    }

    if (config_mode)
    {
        std::cerr << "Error: Option -c requires a configuration file path. Use -h for help." << std::endl;
        return false;
    }

    if (!config_path_.empty())
    {
        struct stat stats;
        if (stat(config_path_.c_str(), &stats) == -1)
        {
            std::cerr << "Error: '" << config_path_ << "': " << strerror(errno) << std::endl;
            return false;
        }

        if ((stats.st_mode & S_IFMT) != S_IFREG)
        {
            std::cerr << "Error: '" << config_path_ << "' must be a regular file." << std::endl;
            return false;
        }
    }

    return true;
}

void Daemon::initialize(int argc, char **argv)
{
    if (!parseArgs(argc, argv))
        throw std::invalid_argument("");

    std::unique_ptr<Config> config = config_path_.empty() 
        ? std::make_unique<Config>()
        : std::make_unique<Config>(config_path_);

    config_ = &config->getDaemonConfig();
    logger_ = std::make_unique<TintinReporter>("matt_daemon", config->getLoggerConfig());
    server_ = std::make_unique<Server>(config->getServerConfig(), *logger_); 
    instance_ = this;

    logger_->log(INFO, "Started.");

    lock_fd_ = open(config_->getLockFile().c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (lock_fd_ == -1)
        throw std::runtime_error(std::string("open lock failed: ") + strerror(errno));

    if (flock(lock_fd_, LOCK_EX | LOCK_NB) == -1)
    {
        if (errno == EWOULDBLOCK)
            std::cerr << "Error: Another instance is already running." << std::endl;

        throw std::runtime_error(std::string("flock failed: ") + strerror(errno));
    }

    addSignals();

    start();
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
            int pid_fd = open(config_->getPidFile().c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
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

    remove(config_->getLockFile().c_str());
    remove(config_->getPidFile().c_str());
}

void Daemon::log(LogLevel log_level, const char *msg)
{
    logger_->log(log_level, msg);
}

void Daemon::showError(const char *msg)
{
    if (logger_)
    {
        if (strlen(msg) > 0)
            logger_->log(ERROR, msg);
        logger_->log(INFO, "Quitting.");
    }
    else
    {
        if (strlen(msg) > 0)
            std::cerr << "Error: " << msg << std::endl; 
    }
}
