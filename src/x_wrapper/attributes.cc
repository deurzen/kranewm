#include "attributes.hh"
#include "display.hh"
#include "window.hh"

using namespace x_wrapper;


attributes_t
x_wrapper::get_attributes(window_t& win)
{
    XWindowAttributes wa;
    XGetWindowAttributes(g_dpy, win, &wa);
    return wa;
}
