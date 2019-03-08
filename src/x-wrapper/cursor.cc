#include "cursor.hh"
#include "window.hh"

void
x_wrapper::set_cursor(window_t win)
{
    XSetWindowAttributes wa;
    wa.cursor = XCreateFontCursor(g_dpy, XC_left_ptr);
    XChangeWindowAttributes(g_dpy, (XDefaultRootWindow(g_dpy)), CWCursor, &wa);
}
