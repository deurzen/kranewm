#include "util.hh"

#include <utility>

extern "C" {
#include <unistd.h>
}

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

void
Util::assert(bool condition, const std::string&& msg)
{
    if (!condition)
        Util::die(std::forward<const std::string&&>(msg));
}
