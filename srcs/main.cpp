#include "Daemon.hpp"
#include "Config.hpp"

int main(int argc, char **argv)
{
    Daemon daemon;
    try {
        daemon.initialize(argc, argv);
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
