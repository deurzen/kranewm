#ifndef __KRANEWM__X_WRAPPER__WINDOW__GUARD__
#define __KRANEWM__X_WRAPPER__WINDOW__GUARD__

#include "common.hh"
#include "type.hh"
#include "display.hh"

extern "C" {
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
}

#include <vector>


namespace x_wrapper
{
    extern void enable_substructure_events();
    extern void disable_substructure_events();

    class window_t : public x_type
    {
    public:
        window_t(Window win)
            : val(win)
        {}

        explicit window_t(void* raw_data)
            : val(*(Window*)raw_data)
        {}

        operator Window() const { return val; }
        operator bool()   const { return val != 0; }

        inline int  length() const { return 1; }
        inline Atom type()   const { return XA_WINDOW; }
        inline int  size()   const { return 32; }

        inline Window get() const { return val; }
        inline Window* get_ptr() { return &val; }
        inline const Window* get_const_ptr() const { return &val; }

        window_t& map()
        {
            XMapWindow(g_dpy, val);
            return *this;
        }

        window_t& unmap()
        {
            XUnmapWindow(g_dpy, val);
            return *this;
        }

        window_t& move(Pos pos)
        {
            disable_substructure_events();
            XMoveWindow(g_dpy, val, pos.x, pos.y);
            enable_substructure_events();
            return *this;
        }

        window_t& resize(Size size)
        {
            disable_substructure_events();
            XResizeWindow(g_dpy, val, size.w, size.h);
            enable_substructure_events();
            return *this;
        }

        window_t& raise()
        {
            disable_substructure_events();
            XRaiseWindow(g_dpy, val);
            enable_substructure_events();
            return *this;
        }

        window_t& reparent(Pos pos, window_t parent = g_root)
        {
            disable_substructure_events();
            XReparentWindow(g_dpy, val, parent.get(), pos.x, pos.y);
            enable_substructure_events();
            return *this;
        }

        void close();
        void force_close();

        void destroy()
        {
            XDestroyWindow(g_dpy, val);
        }


    private:
        Window val;

    };

    extern window_t g_root;
    extern display_t g_dpy;

    extern window_t create_window(bool do_not_manage);
    extern void get_top_level_windows(::std::vector<window_t>&);

}

#endif//__KRANEWM__X_WRAPPER__WINDOW__GUARD__
