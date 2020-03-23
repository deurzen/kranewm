#ifndef __KRANEWM__CONFIG__GUARD__
#define __KRANEWM__CONFIG__GUARD__

#include "common.hh"

#include "commands.hh"

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <unordered_map>


const char CONFIG_COMMENT_DELIMITER = '#';

enum class configkeyword_t
{
    map,
    set
};


class confighandler_t
{
public:
    confighandler_t()
        : m_keywords({
              { "map", configkeyword_t::map },
              { "set", configkeyword_t::set },
          }),
          m_supported_commands({
              {},
          }),
          m_erroneous(false),
          m_configdir(((::std::getenv("XDG_CONFIG_HOME"))
              ? ::std::string(::std::getenv("XDG_CONFIG_HOME"))
              : "~/.config") + "/" + WMNAME + "/"),
          m_configfile("config"),
          m_autostartfile_blocking("blocking_autostart"),
          m_autostartfile_nonblocking("nonblocking_autostart")
    {
        if (::std::filesystem::exists(m_configdir + m_configfile) && parse_config())
            ;

#ifndef DEBUG
        if (::std::filesystem::exists(m_configdir + m_autostartfile_blocking))
            ::std::system((m_configdir + m_autostartfile_blocking).c_str());

        if (::std::filesystem::exists(m_configdir + m_autostartfile_nonblocking))
            ::std::system((m_configdir + m_autostartfile_nonblocking).c_str());
#endif
    }

private:
    bool parse_config();
    bool parse_set(::std::istringstream&, unsigned);
    bool parse_map(::std::istringstream&, unsigned);

    void report_syntax_error(unsigned);
    void report_command_error(unsigned);

    ::std::unordered_map<::std::string, configkeyword_t> m_keywords;
    ::std::unordered_map<::std::string, commandop_t> m_supported_commands;

    bool m_erroneous;

    const ::std::string m_configdir;
    const ::std::string m_configfile;
    const ::std::string m_autostartfile_blocking;
    const ::std::string m_autostartfile_nonblocking;

    ::std::unordered_map<::std::string, ::std::string> m_symbols;
    ::std::unordered_map<::std::string, ::std::string> m_mousebinds;
    ::std::unordered_map<::std::string, ::std::string> m_keybinds;

};


#endif//__KRANEWM__CONFIG__GUARD__
