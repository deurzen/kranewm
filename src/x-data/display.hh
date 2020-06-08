#ifndef __KRANEWM__X_DATA__DISPLAY__GUARD__
#define __KRANEWM__X_DATA__DISPLAY__GUARD__

#include "cursor.hh"

extern "C" {
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
}


namespace x_data
{
    class window_t;
    class display_t
    {
    public:
        explicit display_t(const char* _dpy)
          : dpy(XOpenDisplay(_dpy))
        {
            XSetWindowAttributes wa;
            wa.cursor = XCreateFontCursor(dpy, XC_left_ptr);
            XChangeWindowAttributes(dpy, (XDefaultRootWindow(dpy)), CWCursor, &wa);
        }

        inline
        operator Display*() const
        {
            return dpy;
        }

        inline void
        close() const
        {
            XCloseDisplay(dpy);
        }

    private:
        Display* dpy;

    };

    extern display_t g_dpy;
    extern window_t g_root;
    extern int g_connection;

}

#endif//__KRANEWM__X_DATA__DISPLAY__GUARD__
