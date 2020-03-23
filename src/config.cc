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
    unsigned line_nr = 0;

    while (::std::getline(configfile, line) && ++line_nr) {
        ::std::istringstream linestream(line);
        ::std::string type, activator, target, command;

        { // remove comments and empty lines
            if (::std::getline(linestream, line, CONFIG_COMMENT_DELIMITER))
                linestream = ::std::istringstream(line);

            if (line.empty() || ::std::all_of(line.begin(), line.end(), ::isspace))
                continue;
        }

        linestream >> type;
        if (m_keywords.count(type)) // first word cmp {map,set}
            switch (m_keywords.at(type)) {
                case configkeyword_t::set: parse_set(linestream, line_nr); break;
                case configkeyword_t::map: parse_map(linestream, line_nr); break;
                default: break;
            }
        else
            report_syntax_error(line_nr);
    }

    configfile.close();
    return m_erroneous;
}

bool
confighandler_t::parse_set(::std::istringstream& linestream, unsigned number)
{
    ::std::string option;
    ::std::getline(linestream, option);

    if (option.empty() || ::std::all_of(option.begin(), option.end(), ::isspace)) {
        report_syntax_error(number);
        return false;
    }

    // TODO: check option recognized

    return true;
}

bool
confighandler_t::parse_map(::std::istringstream& linestream, unsigned number)
{
    ::std::string mapping;
    ::std::getline(linestream, mapping);

    if (mapping.empty() || ::std::all_of(mapping.begin(), mapping.end(), ::isspace)) {
        report_syntax_error(number);
        return false;
    }

    // TODO: check sensical mapping

    return true;
}

void
confighandler_t::report_syntax_error(unsigned number)
{
    ::std::cerr << "invalid syntax in config at line number " << number << ::std::endl;
    m_erroneous = true;
}

void
confighandler_t::report_command_error(unsigned number)
{
    ::std::cerr << "unsupported command in config at line number " << number << ::std::endl;
    m_erroneous = true;
}
