#include "ewmh.hh"

#include "ipc.hh"

#include "../x-data/property.hh"


x_data::window_t
ewmh_t::get_supporting_wm_check_property()
{
    return x_data::get_property<x_data::window_t>(x_data::g_root, "_NET_SUPPORTING_WM_CHECK");
}

x_data::cardinal_t
ewmh_t::get_ipc_fd()
{
    return x_data::get_property<x_data::cardinal_t>(x_data::g_root, IPC_PREFIX + "SOCKET_FD");
}
