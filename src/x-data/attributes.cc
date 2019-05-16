#include "attributes.hh"
#include "display.hh"
#include "window.hh"

using namespace x_data;


attributes_t
x_data::get_attributes(window_t& win)
{
    XWindowAttributes wa;
    XGetWindowAttributes(g_dpy, win, &wa);
    return wa;
}
