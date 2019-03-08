#ifndef __KRANEWM__X_WRAPPER__MOUSE__GUARD__
#define __KRANEWM__X_WRAPPER__MOUSE__GUARD__

#include "window.hh"

extern "C" {
#include <X11/Xutil.h>
}


namespace x_wrapper
{
    extern void grab_button(unsigned, unsigned);
    extern void confine_pointer(window_t);
    extern void release_pointer();
    extern Pos pointer_position();
}

#endif//__KRANEWM__X_WRAPPER__MOUSE__GUARD__
