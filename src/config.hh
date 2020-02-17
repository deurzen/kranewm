#ifndef __KRANEWM__CONFIG__GUARD__
#define __KRANEWM__CONFIG__GUARD__

#include "common.hh"

#include <csignal>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include <filesystem>


class confighandler_t
{
public:
    confighandler_t()
        : m_configdir(((::std::getenv("XDG_CONFIG_HOME"))
              ? ::std::string(::std::getenv("XDG_CONFIG_HOME"))
              : "~/.config") + "/" + WMNAME + "/"),
          m_configfile("config"),
          m_autostartfile_blocking("blocking_autostart"),
          m_autostartfile_nonblocking("nonblocking_autostart")
    {
        if (!::std::filesystem::exists(m_configdir + m_configfile))
            ;

        if (!::std::filesystem::exists(m_configdir + m_autostartfile_blocking))
            ;

        if (!::std::filesystem::exists(m_configdir + m_autostartfile_nonblocking))
            ;
    }

    bool parse_config();

private:
    const ::std::string m_configdir;
    const ::std::string m_configfile;
    const ::std::string m_autostartfile_blocking;
    const ::std::string m_autostartfile_nonblocking;

    ::std::unordered_map<::std::string, ::std::string> m_symbols;

};


#endif//__KRANEWM__CONFIG__GUARD__
