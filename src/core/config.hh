#ifndef __CONFIG_H_GUARD__
#define __CONFIG_H_GUARD__

#include "search.hh"
#include "rules.hh"

#include <string>
#include <vector>

struct Config
{
#ifdef DEBUG
    static constexpr bool debugging = true;
#else
    static constexpr bool debugging = false;
#endif

#ifdef ENABLE_IPC
    static constexpr bool ipc_enabled = true;
#else
    static constexpr bool ipc_enabled = false;
#endif

    Config();
    ~Config();

    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(const Config&) = delete;
    Config& operator=(Config&&) = delete;

    std::string directory = "$HOME/.config";
    std::string blocking_autostart = "blocking_autostart";
    std::string nonblocking_autostart = "nonblocking_autostart";

    std::vector<SearchSelector_ptr> ignored_producers;
    std::vector<SearchSelector_ptr> ignored_consumers;

    std::vector<std::tuple<SearchSelector_ptr, Rules>> default_rules;

};

#endif//__CONFIG_H_GUARD__
