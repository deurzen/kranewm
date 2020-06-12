#include "util.hh"

#include "common.hh"

#include <algorithm>
#include <iostream>


void
die(const ::std::string&& msg)
{
    ::std::cerr << CLIENTNAME << ": " << msg << ::std::endl;
    exit(1);
}

void
warn(const ::std::string&& msg)
{
    ::std::cerr << msg << ::std::endl;
}

::std::string
uppercase(const ::std::string word)
{
    ::std::string uppercase_word = word;

    for (char& c : uppercase_word)
        c = ::toupper(c);

    return uppercase_word;
}
