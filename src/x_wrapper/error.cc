#include "error.hh"

extern "C" {
#include <X11/Xproto.h>
}

#include <unordered_map>

int
x_wrapper::g_xerror(Display* dpy, XErrorEvent* error)
{
    static const ::std::unordered_map<int, int> permissible_errors({
        { X_GrabButton,        BadAccess   },
        { X_GrabKey,           BadAccess   },
        { X_CopyArea,          BadDrawable },
        { X_PolyFillRectangle, BadDrawable },
        { X_PolySegment,       BadDrawable },
        { X_PolyText8,         BadDrawable },
        { X_ConfigureWindow,   BadMatch    },
        { X_SetInputFocus,     BadMatch    },
    });

    if (error->error_code == BadWindow)
        return 0;

    for (auto reqerr_pair : permissible_errors)
        if (error->request_code == reqerr_pair.first && error->error_code == reqerr_pair.second)
            return 0;

    ::std::cerr << "kranewm: fatal error: request code=" << error->request_code
              << ", error code=" << error->error_code << ::std::endl;

    return -1;
}

int
x_wrapper::g_xerroroff(Display* dpy, XErrorEvent* error)
{
    return 0;
}

XErrorHandler
x_wrapper::set_error_handler(int (*handler)(Display*, XErrorEvent*))
{
    return XSetErrorHandler(handler);
}
