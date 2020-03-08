#include "config.hh"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>


bool
confighandler_t::parse_config()
{
    ::std::ifstream configfile(m_configdir + m_configfile);
    ::std::string line;

    while (::std::getline(configfile, line)) {
        ::std::stringstream linestream(line);
        ::std::string type, activator, target, command;

        { // remove comments and empty lines
            if (::std::getline(linestream, line, CONFIG_COMMENT_DELIMITER))
                linestream.str(line);

            if (line.empty() || ::std::all_of(line.begin(), line.end(), ::isspace))
                continue;
        }

        ::std::cout << "line: " << line << ::std::endl;

        if ((linestream >> type) && m_keywords.count(type)) { // first word cmp {map,set}
            switch (m_keywords.at(type)) {
                case configkeyword_t::set: if (parse_set(linestream)) break; else continue;
                case configkeyword_t::map: if (parse_map(linestream)) break; else continue;
                default: break;
            }

            ::std::cout << "continuing" << ::std::endl;

            continue;
        }

        ::std::cout << type << ::std::endl;
        ::std::cout << "returning false" << ::std::endl;

        return false;
    }

    ::std::cout << "returning true" << ::std::endl;

    return true;
}

bool
confighandler_t::parse_set(::std::stringstream& linestream)
{
    ::std::cout << "handling set" << ::std::endl;
    return true;
}

bool
confighandler_t::parse_map(::std::stringstream& linestream)
{
    ::std::cout << "handling map" << ::std::endl;
    return true;
}
