#include "event.hh"

using namespace x_wrapper;


int l_substructure_level = 0;
long l_prev_root_mask = 0;

void
x_wrapper::enable_substructure_events()
{
    if (--l_substructure_level != 0)
        return;

    if (!(l_prev_root_mask & SubstructureNotifyMask))
        return;

    XSelectInput(g_dpy, g_root, l_prev_root_mask | SubstructureNotifyMask);
    XFlush(g_dpy);
}

void
x_wrapper::disable_substructure_events()
{
    if (++l_substructure_level != 1)
        return;

    if (!(l_prev_root_mask & SubstructureNotifyMask))
        return;

    XSelectInput(g_dpy, g_root, l_prev_root_mask & ~SubstructureNotifyMask);
    XFlush(g_dpy);
}

void
x_wrapper::next_event(event_t& event)
{
    XNextEvent(g_dpy, event.get_ptr());
}

void
x_wrapper::last_typed_event(event_t& event, int type)
{
    while (XCheckTypedEvent(g_dpy, type, event.get_ptr()));
}

void
x_wrapper::sync(bool discard)
{
    XSync(g_dpy, discard);
}


event_t&
x_wrapper::event_t::send(long mask, window_t win)
{
    XSendEvent(g_dpy, ((win.get() == None) ? val.xany.window : win.get()),
        False, mask, &val);
    return *this;
}
