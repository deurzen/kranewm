#include "display.hh"
#include "window.hh"

x_wrapper::display_t x_wrapper::g_dpy(nullptr);
x_wrapper::window_t x_wrapper::g_root(XDefaultRootWindow(g_dpy));
