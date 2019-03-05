#include "window.hh"


x_wrapper::window_t x_wrapper::create_window(bool do_not_manage)
{
    window_t win(XCreateSimpleWindow(x_wrapper::g_dpy, x_wrapper::g_root, -2, -2,
        1, 1, 1, 0x000000, 0x404040));

    if (do_not_manage) {
        XSetWindowAttributes wa;
        wa.override_redirect = true;
        XChangeWindowAttributes(x_wrapper::g_dpy, win, CWOverrideRedirect, &wa);
    }

    return win;
}
