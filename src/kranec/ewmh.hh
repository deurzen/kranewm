#ifndef __KRANEWM__KRANEC__EWMH__GUARD__
#define __KRANEWM__KRANEC__EWMH__GUARD__

#include "../x-data/window.hh"
#include "../x-data/cardinal.hh"
#include "../x-data/string.hh"


class ewmh_t
{
public:
    ewmh_t() = default;
    ~ewmh_t() = default;

    x_data::window_t get_supporting_wm_check_property();
    x_data::cardinal_t get_ipc_fd();

};


#endif//__KRANEWM__KRANEC__EWMH__GUARD__
