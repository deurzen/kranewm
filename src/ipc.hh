#ifndef __KRANEWM__IPC__GUARD__
#define __KRANEWM__IPC__GUARD__

#include "x-data/window.hh"
#include "x-data/property.hh"
#include "x-data/event.hh"
#include "x-data/property.hh"

#include <map>
#include <tuple>

enum class ipcop_t
{
    noop,
    goto_next_ws, goto_prev_ws,
    goto_ws_index,
    goto_next_cx, goto_prev_cx,
    goto_cx_index,
    focus_win,
};

typedef ::std::tuple<x_data::x_type*, ipcop_t> ipccommand_t;

const ::std::string IPC_PREFIX = "_KRANEWM_";
const bool IPC_ENABLED = true;


class ipc_t
{
public:
    ipc_t()
      : supported_commands({
          {"NEXT_WS",   {{},                       ipcop_t::goto_next_ws}},
          {"PREV_WS",   {{},                       ipcop_t::goto_prev_ws}},
          {"GOTO_WS",   {new x_data::cardinal_t{}, ipcop_t::goto_ws_index}},

          {"NEXT_CX",   {{},                       ipcop_t::goto_next_cx}},
          {"PREV_CX",   {{},                       ipcop_t::goto_next_cx}},
          {"GOTO_CX",   {new x_data::cardinal_t{}, ipcop_t::goto_cx_index}},

          {"FOCUS_WIN", {new x_data::window_t{},   ipcop_t::focus_win}},
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
