#include "utils.hpp"

bool isAbsolutePath(const std::string &path)
{
    return (!path.empty() && path[0] == '/');
}
