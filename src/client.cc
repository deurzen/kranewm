#include "client.hh"
#include "decoration.hh"

void
client_t::move(Pos new_pos)
{
    frame.move(new_pos);
    pos = new_pos;
}

void
client_t::resize(Size new_size)
{
    if (new_size.h > BORDER_HEIGHT)
        win.resize({new_size.w, new_size.h - BORDER_HEIGHT});
    frame.resize(new_size);

    size = new_size;
}

void
client_t::map()
{
    win.map();
    frame.map();
}

void
client_t::unmap()
{
    frame.unmap();
    win.unmap();
}

void
client_t::focus()
{

}

void
client_t::unfocus(bool)
{

}

void
client_t::center()
{

}
