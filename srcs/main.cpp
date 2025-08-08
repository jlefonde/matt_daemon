#include "Daemon.hpp"

int main(void)
{
    Daemon &daemon = Daemon::getInstance();
    daemon.initialize();
    daemon.start(4242);
}
