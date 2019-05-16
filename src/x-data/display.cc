#include "display.hh"
#include "window.hh"

x_data::display_t x_data::g_dpy(nullptr);
x_data::window_t x_data::g_root(XDefaultRootWindow(g_dpy));
