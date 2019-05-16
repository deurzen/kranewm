#include "request.hh"


void
x_data::propagate_configure_request(event_t event, unsigned flags)
{
    XWindowChanges wc;
    wc.x = event.get().xconfigurerequest.x;
    wc.y = event.get().xconfigurerequest.y;
    wc.width = event.get().xconfigurerequest.width;
    wc.height = event.get().xconfigurerequest.height;
    wc.border_width = event.get().xconfigurerequest.border_width;
    wc.sibling = event.get().xconfigurerequest.above;
    wc.stack_mode = event.get().xconfigurerequest.detail;

    unsigned flag = event.get().xconfigurerequest.value_mask;
    if (flags)
        flag &= flags;

    XConfigureWindow(g_dpy, event.get().xconfigurerequest.window, flag, &wc);
}

void
x_data::propagate_circulate_request(event_t event)
{
    int dir = (event.get().xcirculaterequest.place == PlaceOnTop) ?  RaiseLowest : LowerHighest;
    XCirculateSubwindows(g_dpy, event.get().xcirculaterequest.window, dir);
}
