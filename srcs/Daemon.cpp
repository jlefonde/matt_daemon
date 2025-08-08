#include "Daemon.hpp"

Daemon& Daemon::getInstance()
{
    static Daemon instance;
    return instance;
}

static void signal_handler(int sig)
{
    std::cout << "Signal handler." << std::endl;

    if (sig == SIGTERM)
        Daemon::getInstance().shutdown();
}

static void write_pid_to_fd(int fd, pid_t pid)
{
    std::string pid_str = std::to_string(pid) + "\n";
    write(fd, pid_str.c_str(), pid_str.size());
}

void Daemon::initialize(int port)
{
    port_ = port;
    logger_ = std::make_unique<TintinReporter>("matt_daemon", ERROR);

    log(INFO, "Started.");

    if (signal(SIGTERM, &signal_handler) == SIG_ERR)
    {
        log(ERROR, strerror(errno));
        exit(EXIT_FAILURE);
    }

    lock_fd_ = open(LOCK_FILE_PATH, O_CREAT | O_RDWR, 0644);
    if (lock_fd_ == -1)
    {
        log(ERROR, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (flock(lock_fd_, LOCK_EX | LOCK_NB) == -1)
    {
        log(ERROR, "Error file locked.");
        std::cerr << "Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Daemon::start()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        log(ERROR, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (setsid() == -1)
        {
            log(ERROR, strerror(errno));
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid < 0)
        {
            log(ERROR, strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            write_pid_to_fd(lock_fd_, getpid());

            if (chdir("/") == -1)
            {
                log(ERROR, strerror(errno));
                exit(EXIT_FAILURE);
            }

            umask(0);

            close(STDIN_FILENO);
            close(STDERR_FILENO);
            close(STDOUT_FILENO);

            if (open("/dev/null", O_RDWR) == -1)
            {
                log(ERROR, strerror(errno));
                exit(EXIT_FAILURE);
            }

            dup(0);
            dup(0);

            while (true)
            {
                sleep(1000);
            }
        }
        else
            exit(EXIT_SUCCESS);
    }
    else
        exit(EXIT_SUCCESS);
}

void Daemon::shutdown()
{
    log(INFO, "Quitting.");

    if (lock_fd_ != -1)
        close(lock_fd_);

    if (remove(LOCK_FILE_PATH))
    {
        log(ERROR, strerror(errno));
        exit(EXIT_FAILURE);          
    }

    exit(EXIT_SUCCESS);
}

void Daemon::log(LogLevel log_level, const char *msg)
{
    logger_->log(log_level, msg);
}