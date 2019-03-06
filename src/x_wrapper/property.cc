#include "property.hh"

using namespace x_wrapper;


void
x_wrapper::remove_property(Window win, const ::std::string& name)
{
    XDeleteProperty(g_dpy, win, get_atom(name));
}
