#ifndef __KRANEWM__X_WRAPPER__ERROR__GUARD__
#define __KRANEWM__X_WRAPPER__ERROR__GUARD__

#include "common.hh"

extern "C" {
#include <X11/Xlib.h>
}


namespace x_wrapper {
    extern int g_xerror(Display*, XErrorEvent*);
    extern int g_xerroroff(Display*, XErrorEvent*);
}

#endif//__KRANEWM__X_WRAPPER__ERROR__GUARD__
