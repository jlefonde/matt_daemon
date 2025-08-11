#include "Daemon.hpp"

int main(void)
{
    Daemon daemon;
    try {
        daemon.initialize();
        daemon.start(4241);
    }
    catch (const std::exception& e) {
        daemon.showError(e.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        daemon.showError("Unknown exception");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
