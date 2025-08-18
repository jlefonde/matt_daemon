#ifndef MAIN_HPP
#define MAIN_HPP

#include <getopt.h>
#include <iostream>
#include <string>

#include "Daemon.hpp"
#include "Config.hpp"
#include "TintinReporter.hpp"

#define ROOT_UID 0

typedef struct s_context {
    char    *config_path;
    std::string config_path_abs;
}   t_context;

#endif