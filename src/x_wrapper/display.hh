#ifndef __KRANEWM__X_WRAPPER__DISPLAY__GUARD__
#define __KRANEWM__X_WRAPPER__DISPLAY__GUARD__

extern "C" {
#include <X11/Xlib.h>
}


namespace x_wrapper
{
    class window_t;
    class display_t
    {
    public:
        explicit display_t(const char* dpy)
            : dpy(XOpenDisplay(dpy))
        {}

        operator Display*() const;

    private:
        Display* dpy;

    };

    inline display_t::operator Display*() const { return dpy; }

    extern display_t g_dpy;
    extern window_t g_root;
}

#endif//__KRANEWM__X_WRAPPER__DISPLAY__GUARD__
