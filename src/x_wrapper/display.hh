#ifndef __KRANEWM__X_WRAPPER__DISPLAY__GUARD__
#define __KRANEWM__X_WRAPPER__DISPLAY__GUARD__

#include "window.hh"

extern "C" {
#include <X11/Xlib.h>
}


namespace x_wrapper
{
    class display_t
    {
    public:
        display_t(const char* dpy)
            : dpy(XOpenDisplay(dpy))
        {}

        operator Display*() const { return dpy; }

    private:
        Display* dpy;

    };

    extern display_t g_dpy;
    extern window_t g_root;
}

#endif//__KRANEWM__X_WRAPPER__DISPLAY__GUARD__
