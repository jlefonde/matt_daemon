#include "Daemon.hpp"

int main(void)
{
    try
    {
        Daemon &daemon = Daemon::getInstance();
        daemon.initialize();
        daemon.start(4242);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
