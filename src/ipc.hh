#ifndef __KRANEWM__IPC__GUARD__
#define __KRANEWM__IPC__GUARD__

#include "x-data/window.hh"
#include "x-data/property.hh"
#include "x-data/event.hh"


const ::std::string IPC_PREFIX = "_KRANEWM_";
const bool IPC_ENABLED = true;

class ipc_t
{
public:
    ipc_t()
    {
        x_data::replace_property<x_data::string_t>(x_data::g_root,
            {"_IPC_PREFIX", IPC_PREFIX});

        x_data::replace_property<x_data::cardinal_t>(x_data::g_root,
            {"_KRANEWM_IPC_ENABLED", IPC_ENABLED});
    }

    bool assert_target(x_data::event_t) const;

private:

};


#endif//__KRANEWM__IPC__GUARD__
