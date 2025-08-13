#include "Daemon.hpp"

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

static bool parseArgs(int argc, char **argv, std::string& config_path)
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
                exit(EXIT_SUCCESS);
            }
            else
            {
                std::cerr << "Error: Unknown option '" << argv[i] << "'. Use -h for help." << std::endl;
                return false;
            }
        }
        else if (config_mode)
        {
            config_path = argv[i];
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

    if (!config_path.empty())
    {
        struct stat stats;
        if (stat(config_path.c_str(), &stats) == -1)
        {
            std::cerr << "Error: '" << config_path << "': " << strerror(errno) << std::endl;
            return false;
        }

        if ((stats.st_mode & S_IFMT) != S_IFREG)
        {
            std::cerr << "Error: '" << config_path << "' must be a regular file." << std::endl;
            return false;
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    std::string config_path;
    if (!parseArgs(argc, argv, config_path))
        return 1;

    Daemon daemon;
    try {
        daemon.initialize(config_path);
    }
    catch (const std::exception& e) {
        daemon.showError(e.what());
        return 1;
    }
    catch (...) {
        daemon.showError("Unknown exception.");
        return 1;
    }
    return 0;
}
