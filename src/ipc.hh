#ifndef __KRANEWM__IPC__GUARD__
#define __KRANEWM__IPC__GUARD__

#include "x-data/window.hh"
#include "x-data/property.hh"
#include "x-data/event.hh"
#include "x-data/property.hh"

#include <map>


const ::std::string IPC_PREFIX = "_KRANEWM_";
const bool IPC_ENABLED = true;

class ipc_t
{
public:
    ipc_t()
      : supported_commands({
          {"NEXT_WS", {}},
          {"PREV_WS", {}},
          {"GOTO_WS", new x_data::cardinal_t{}},

          {"NEXT_CX", {}},
          {"PREV_CX", {}},
          {"GOTO_CX", new x_data::cardinal_t{}},

          {"FOCUS_WIN", new x_data::window_t{}},
        })
    {
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
        for (auto [name,atom] : supported_commands)
            delete atom;
    }

    bool assert_target(x_data::event_t) const;

private:
    ::std::map<::std::string, x_data::x_type*> supported_commands;

};


#endif//__KRANEWM__IPC__GUARD__
