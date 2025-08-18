#include "main.hpp"

static struct option long_options[] = {
    {"help",   no_argument,       0,  'h' },
    {"config", required_argument, 0,  'c' },
    {0,        0,                 0,   0  }
};

static void showHelper()
{
    std::cout << "Usage: MattDaemon [OPTIONS]\n"
              << "\n"
              << "DESCRIPTION:\n"
              << "  A daemon with a server that listen on a specific port and register incoming data in a log file.\n"
              << "\n"
              << "OPTIONS:\n"
              << "  -h                  Show this help message and exit\n"
              << "  -c <config_file>    Specify configuration file path\n"
              << std::endl;
}

static Config createConfig(t_context *ctx)
{
    if (!ctx->config_path)
    {
        Config config;
        return config;
    }

    char resolved_path[PATH_MAX];
    if (realpath(ctx->config_path, resolved_path) == NULL)
    {
        std::cerr << "Error: Cannot resolve path '" << ctx->config_path << "': " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    ctx->config_path_abs = std::string(resolved_path);

    struct stat stats;
    if (stat(ctx->config_path_abs.c_str(), &stats) == -1)
    {
        std::cerr << "Error: '" << ctx->config_path << "': " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if ((stats.st_mode & S_IFMT) != S_IFREG)
    {
        std::cerr << "Error: '" << ctx->config_path << "' must be a regular file." << std::endl;
        exit(EXIT_FAILURE);
    }

    Config config(ctx->config_path_abs);
    return config;
}

int main(int argc, char **argv)
{
    if (geteuid() != ROOT_UID)
    {
        std::cerr << "Error: Root privileges are required to run. Please run with sudo or as root user." << std::endl;
        return 1;
    }

    t_context ctx;
    ctx.config_path = NULL;

    int ch;
    while ((ch = getopt_long(argc, argv, "hc:", long_options, NULL)) != -1)
    {
        switch (ch)
        {
        case 'h':
            showHelper();
            return 0;
        case 'c':
            ctx.config_path = optarg;
            break;
        default:
            std::cerr << "Use -h for help." << std::endl;
            return 1;
        }
    }

    try {
        Config config = createConfig(&ctx);

        TintinReporter logger(config.getLoggerConfig(), "matt_daemon");
        Server server(config.getServerConfig(), logger);
        Daemon daemon(ctx.config_path_abs, config.getDaemonConfig(), logger, server);

        daemon.initialize();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception." << std::endl;
        return 1;
    }

    return 0;
}
