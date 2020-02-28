#ifndef __KRANEWM__IPC__GUARD__
#define __KRANEWM__IPC__GUARD__

#include "commands.hh"

#include "x-data/window.hh"
#include "x-data/property.hh"
#include "x-data/event.hh"
#include "x-data/property.hh"

#include <map>
#include <tuple>

typedef ::std::tuple<x_data::x_type*, commandop_t> ipccommand_t;

const ::std::string IPC_PREFIX = "_KRANEWM_";
const bool IPC_ENABLED = false;


class ipc_t
{
public:
    ipc_t()
      : supported_commands({
          {"NEXT_WS",   {{},                       commandop_t::nextworkspace}},
          {"PREV_WS",   {{},                       commandop_t::previousworkspace}},
          /* {"GOTO_WS",   {new x_data::cardinal_t{}, commandop_t::goto_ws_index}}, */

          {"NEXT_CX",   {{},                       commandop_t::nextcontext}},
          {"PREV_CX",   {{},                       commandop_t::previouscontext}},
          /* {"GOTO_CX",   {new x_data::cardinal_t{}, commandop_t::goto_cx_index}}, */

          /* {"FOCUS_WIN", {new x_data::window_t{},   commandop_t::focus_win}}, */
        })
    {
        if (!IPC_ENABLED)
            return;

        x_data::replace_property<x_data::string_t>(x_data::g_root,
            {"_IPC_PREFIX", IPC_PREFIX});

        x_data::replace_property<x_data::cardinal_t>(x_data::g_root,
            {IPC_PREFIX + "IPC_ENABLED", IPC_ENABLED});

        for (auto&& [name,atom] : supported_commands) {
            x_data::replace_property<x_data::cardinal_t>(x_data::g_root,
                {IPC_PREFIX + name});
            x_data::remove_property(x_data::g_root, IPC_PREFIX + name);
        }

    }

    ~ipc_t()
    {
        for (auto [name,data] : supported_commands) {
            auto [atom,_] = data;
            delete atom;
        }
    }

    bool assert_target(x_data::event_t) const;
    ipccommand_t resolve_operation(x_data::event_t) const;

private:
    ::std::map<::std::string, ipccommand_t> supported_commands;

};


#endif//__KRANEWM__IPC__GUARD__
