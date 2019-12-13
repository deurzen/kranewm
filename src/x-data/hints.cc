#include "hints.hh"
#include "display.hh"
#include "window.hh"

#include <cstring>

using namespace x_data;


wmhints_t
x_data::get_wmhints(window_t& win)
{
    bool status = false;
    XWMHints hints;
    XWMHints* x_hints = XGetWMHints(g_dpy, win.get());
    if (x_hints) {
        ::std::memcpy(&hints, x_hints, sizeof(XWMHints));
        XFree(x_hints);
        status = true;
    }

    return {hints, status};
}

void
x_data::set_wmhints(window_t& win, wmhints_t& hints)
{
    XSetWMHints(g_dpy, win.get(), hints.get_ptr());
}

sizehints_t
x_data::get_sizehints(window_t& win)
{
    long _l;
    XSizeHints hints;
    bool status;

    if ((status = !XGetWMNormalHints(g_dpy, win.get(), &hints, &_l)))
        hints.flags = PSize;

    return {hints, status};
}
