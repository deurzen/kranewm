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
    XClearArea(g_dpy, win, pos.x, pos.y - font_dim.h, line_width, font_dim.h, False);
}

void
x_data::graphicscontext_t::clear(pos_t pos)
{
    XClearArea(g_dpy, win, pos.x, pos.y - font_dim.h, line_width, font_dim.h, False);
}

void
x_data::graphicscontext_t::clear_window()
{
    XClearWindow(g_dpy, win);
}

void
x_data::graphicscontext_t::draw_string(pos_t pos, const ::std::string& text)
{
    x_data::graphicscontext_t::pos = pos;
    XDrawString(g_dpy, win, gc, pos.x, pos.y, text.c_str(), text.size());
}
