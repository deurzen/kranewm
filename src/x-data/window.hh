#ifndef __KRANEWM__X_DATA__WINDOW__GUARD__
#define __KRANEWM__X_DATA__WINDOW__GUARD__

#include "common.hh"
#include "type.hh"
#include "display.hh"

extern "C" {
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
}

#include <vector>


namespace x_data
{
    extern void enable_substructure_events();
    extern void disable_substructure_events();

    class window_t : public x_type
    {
    public:
        window_t()
            : val(None) {}

        window_t(Window win)
            : val(win) {}

        explicit window_t(void* raw_data, unsigned long _)
            : val(*(Window*)raw_data) {}

        operator Window() const { return val; }
        operator bool()   const { return val != None; }

        inline bool operator==(const window_t& win) const
        {
            return win.val == val;
        }

        inline bool operator==(const Window& win) const
        {
            return win == val;
        }

        inline int  length() const { return 1; }
        inline Atom type()   const { return XA_WINDOW; }
        inline int  size()   const { return 32; }

        inline Window get() const { return val; }
        inline Window* get_ptr() { return &val; }
        inline const Window* get_const_ptr() const { return &val; }

        inline void set(Window new_val) { val = new_val; }

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

        window_t& move(pos_t pos)
        {
            disable_substructure_events();
            XMoveWindow(g_dpy, val, pos.x, pos.y);
            enable_substructure_events();
            return *this;
        }

        window_t& resize(dim_t dim)
        {
            disable_substructure_events();
            XResizeWindow(g_dpy, val, dim.w, dim.h);
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

        window_t& lower()
        {
            disable_substructure_events();
            XLowerWindow(g_dpy, val);
            enable_substructure_events();
            return *this;
        }

        window_t& reparent(pos_t pos, window_t parent = g_root)
        {
            disable_substructure_events();
            XReparentWindow(g_dpy, val, parent.get(), pos.x, pos.y);
            enable_substructure_events();
            return *this;
        }


        window_t& set_border_color(unsigned long color)
        {
            XSetWindowBorder(g_dpy, val, color);
            return *this;
        }

        window_t& set_border_width(int width)
        {
            enable_substructure_events();
            XSetWindowBorderWidth(g_dpy, val, width);
            disable_substructure_events();
            return *this;
        }

        window_t& set_background_color(unsigned long color)
        {
            XSetWindowBackground(g_dpy, val, color);
            XClearWindow(g_dpy, val);
            return *this;
        }

        ::std::string get_class()
        {
            ::std::string cls;
            XClassHint* hint = XAllocClassHint();
            XGetClassHint(g_dpy, val, hint);

            if (hint->res_class) {
                cls.assign(hint->res_class);
                XFree(hint);
            }

            return cls;
        }

        ::std::string get_instance()
        {
            ::std::string inst;
            XClassHint* hint = XAllocClassHint();
            XGetClassHint(g_dpy, val, hint);

            if (hint->res_name) {
                inst.assign(hint->res_name);
                XFree(hint);
            }

            return inst;
        }

        void destroy()
        {
            XDestroyWindow(g_dpy, val);
        }

        void grab()
        {
            XGrabButton(g_dpy, AnyButton, AnyModifier, val, True,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None);
        }

        void ungrab()
        {
            XUngrabButton(g_dpy, AnyButton, AnyModifier, val);
        }

        ::std::string get_name();
        void close();
        void force_close();
        void set_state(long);
        bool is_of_type(::std::string&&);
        bool is_of_state(::std::string&&);

    private:
        Window val;

    };

    extern window_t g_root;
    extern display_t g_dpy;
    extern unsigned g_numlockmask;

    extern window_t create_window(bool do_not_manage = false);
    extern ::std::vector<window_t> get_top_level_windows();
    extern window_t get_transient_for(window_t&);
    extern bool should_manage(window_t&);
    extern window_t get_input_focus();
    extern bool set_input_focus(window_t = None);
    extern void select_input(window_t, long);
    extern void restack_windows(Window*, int);
    extern void update_numlockmask();

}

namespace std
{
    template <>
    struct hash<x_data::window_t>
    {
        ::std::size_t operator()(const x_data::window_t& win) const
        {
            return win.get();
        }
    };
}

#endif//__KRANEWM__X_DATA__WINDOW__GUARD__
