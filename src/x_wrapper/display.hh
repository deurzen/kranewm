#ifndef __KRANEWM__X_WRAPPER__DISPLAY__GUARD__
#define __KRANEWM__X_WRAPPER__DISPLAY__GUARD__

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
}

#endif//__KRANEWM__X_WRAPPER__DISPLAY__GUARD__
