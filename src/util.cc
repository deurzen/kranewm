#include "util.hh"
#include "common.hh"

#include <iostream>


void
die(const ::std::string& msg)
{
    ::std::cerr << WMNAME << ": " << msg << ::std::endl;
    exit(1);
}
