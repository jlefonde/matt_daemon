#include <csignal>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>

#define LOCK_FILE_PATH "/var/lock/matt_daemon.lock"
#define LOG_FOLDER_PATH "/var/log/matt_daemon/"
#define LOG_FILE_PATH LOG_FOLDER_PATH "matt_daemon.log"

int g_lock_fd = -1;

void signal_handler(int sig)
{
    std::cout << "Signal handler." << std::endl;

    if (sig == SIGTERM)
    {
        std::cout << "Quitting." << std::endl;

        if (g_lock_fd != -1)
        {
            close(g_lock_fd);
            g_lock_fd = -1;
        }

        if (remove(LOCK_FILE_PATH))
        {
            std::cerr << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);          
        }

        exit(EXIT_SUCCESS);
    }
}

void write_pid_to_fd(int fd, pid_t pid)
{
    std::string pid_str = std::to_string(pid) + "\n";
    write(fd, pid_str.c_str(), pid_str.size());
}

int main(void)
{
    if (mkdir(LOG_FOLDER_PATH, 0755) == -1)
    {
        if (errno != EEXIST)
        {
            std::cerr << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    std::ofstream log_file;
    log_file.open(LOG_FILE_PATH, std::ofstream::ate | std::ofstream::app);
    if (log_file.fail() || log_file.eof())
    {
        std::cerr << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    log_file << "Started." << std::endl;

    if (signal(SIGTERM, &signal_handler) == SIG_ERR)
    {
        log_file << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    g_lock_fd = open(LOCK_FILE_PATH, O_CREAT | O_RDWR, 0644);
    if (g_lock_fd == -1)
    {
        log_file << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (flock(g_lock_fd, LOCK_EX | LOCK_NB) == -1)
    {
        log_file << "Error file locked." << std::endl;
        std::cerr << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        log_file << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (setsid() == -1)
        {
            log_file << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid < 0)
        {
            log_file << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            write_pid_to_fd(g_lock_fd, getpid());

            if (chdir("/") == -1)
            {
                log_file << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }

            umask(0);

            close(STDIN_FILENO);
            close(STDERR_FILENO);
            close(STDOUT_FILENO);

            if (open("/dev/null", O_RDWR) == -1)
            {
                log_file << strerror(errno) << std::endl;
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
        {
            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        exit(EXIT_SUCCESS);
    }
}
