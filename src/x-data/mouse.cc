#include "mouse.hh"
#include "mask.hh"
#include "display.hh"

using namespace x_data;

window_t l_confined;

void
x_data::grab_button(unsigned button, unsigned mask)
{
    for (auto& to_ignore : g_ignored_masks)
        XGrabButton(g_dpy, button, mask | to_ignore,
            g_root, True, ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None);
}

void
x_data::confine_pointer(window_t win)
{
    if (l_confined)
        return;

    XGrabPointer(g_dpy, win, False,
        PointerMotionMask | ButtonReleaseMask,
        GrabModeAsync, GrabModeAsync,
        None, None, CurrentTime);

    l_confined = win;
}

void
x_data::release_pointer()
{
    if (!l_confined)
        return;

    XUngrabButton(g_dpy, AnyButton, AnyModifier, l_confined);
    l_confined = None;
}

pos_t
x_data::pointer_position()
{
    Window _w;
    int _i;
    unsigned _u;

    pos_t pos;
    XQueryPointer(g_dpy, g_root, &_w, &_w, &pos.x, &pos.y, &_i, &_i, &_u);

    return pos;
}
