#ifndef __KRANEWM__X_WRAPPER__WINDOW__GUARD__
#define __KRANEWM__X_WRAPPER__WINDOW__GUARD__

#include "type.hh"
#include "display.hh"

extern "C" {
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
}


namespace x_wrapper
{

    class window_t : protected x_type
    {
    public:
        explicit window_t(Window win)
            : val(win)
        {}

        operator Window() const { return val; }
        operator bool() const { return val != 0; }

        inline int  length() const { return 1; }
        inline Atom type()   const { return XA_WINDOW; }
        inline int  size()   const { return 32; }

        inline Window get() const { return val; }

    private:
        Window val;

    };

    extern window_t g_root;
    extern display_t g_dpy;
    extern window_t create_window(bool do_not_manage);
}

#endif//__KRANEWM__X_WRAPPER__WINDOW__GUARD__
