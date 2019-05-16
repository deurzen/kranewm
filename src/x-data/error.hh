#ifndef __KRANEWM__X_DATA__ERROR__GUARD__
#define __KRANEWM__X_DATA__ERROR__GUARD__

#include "common.hh"

extern "C" {
#include <X11/Xlib.h>
}


namespace x_data
{
    extern int g_xerror(Display*, XErrorEvent*);
    extern int g_xerroroff(Display*, XErrorEvent*);

    extern XErrorHandler set_error_handler(int (*)(Display*, XErrorEvent*));
}

#endif//__KRANEWM__X_DATA__ERROR__GUARD__
