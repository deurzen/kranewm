#include "graphics.hh"

void
x_data::graphicscontext_t::set_foreground(unsigned long color)
{
    XSetForeground(g_dpy, gc, color);
}

void
x_data::graphicscontext_t::set_background(unsigned long color)
{
    XSetBackground(g_dpy, gc, color);
}

void
x_data::graphicscontext_t::clear()
{
    XClearWindow(g_dpy, win);
}

void
x_data::graphicscontext_t::draw_string(pos_t pos, const ::std::string& text)
{
    XDrawString(g_dpy, win, gc, pos.x, pos.y, text.c_str(), text.size());
}
