#include "util.hh"

#include <unistd.h>

void
Util::die(const std::string&& msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}

void
Util::warn(const std::string&& msg)
{
    std::cerr << msg << std::endl;
}
