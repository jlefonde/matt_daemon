#include "Daemon.hpp"

int main(void)
{
    try
    {
        Daemon &daemon = Daemon::getInstance();
        daemon.initialize(4242);
        daemon.start();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
