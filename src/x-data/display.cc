#include "display.hh"
#include "window.hh"

extern "C" {
#include <X11/Xlib.h>
}

x_data::display_t x_data::g_dpy(nullptr);
x_data::window_t x_data::g_root(XDefaultRootWindow(g_dpy));
int x_data::g_connection = XConnectionNumber(g_dpy);
