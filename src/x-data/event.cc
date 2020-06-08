#include "event.hh"

using namespace x_data;


int l_substructure_level = 0;
long l_prev_root_mask = 0;

void
x_data::enable_substructure_events()
{
    if (--l_substructure_level != 0)
        return;

    if (!(l_prev_root_mask & SubstructureNotifyMask))
        return;

    XSelectInput(g_dpy, g_root, l_prev_root_mask | SubstructureNotifyMask);
    XFlush(g_dpy);
}

void
x_data::disable_substructure_events()
{
    if (++l_substructure_level != 1)
        return;

    if (!(l_prev_root_mask & SubstructureNotifyMask))
        return;

    XSelectInput(g_dpy, g_root, l_prev_root_mask & ~SubstructureNotifyMask);
    XFlush(g_dpy);
}

void
x_data::next_event(event_t& event)
{
    XNextEvent(g_dpy, event.get_ptr());
}

bool
x_data::typed_event(event_t& event, int type)
{
    return XCheckTypedEvent(g_dpy, type, event.get_ptr());
}

void
x_data::last_typed_event(event_t& event, int type)
{
    while (typed_event(event, type));
}

void
x_data::sync(bool discard)
{
    XSync(g_dpy, discard);
}

int
x_data::pending()
{
    return XPending(g_dpy);
}

event_t&
x_data::event_t::send(long mask, window_t win)
{
    XSendEvent(g_dpy, ((win.get() == None) ? val.xany.window : win.get()),
        False, mask, &val);
    return *this;
}
