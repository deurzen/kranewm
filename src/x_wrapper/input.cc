#include "input.hh"


void
x_wrapper::select_input(window_t win, long mask)
{
    XSelectInput(g_dpy, win, mask);
}
