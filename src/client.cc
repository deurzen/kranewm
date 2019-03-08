#include "client.hh"
#include "decoration.hh"
#include "x_wrapper/event.hh"
#include "x_wrapper/attributes.hh"

client_ptr_t
create_client(x_wrapper::window_t win, bool)
{

}

void
update_offset(client_ptr_t client)
{
    auto frame_attrs = x_wrapper::get_attributes(client->frame);
    auto win_attrs = x_wrapper::get_attributes(client->win);

    x_wrapper::event_t event;
    event.get().type = ConfigureNotify;
    event.get().xconfigure.send_event = True;
    event.get().xconfigure.display = x_wrapper::g_dpy;
    event.get().xconfigure.event = client->win;
    event.get().xconfigure.window = client->win;
    event.get().xconfigure.x = frame_attrs.x();
    event.get().xconfigure.y = frame_attrs.y() + BORDER_HEIGHT;
    event.get().xconfigure.width = win_attrs.w();
    event.get().xconfigure.height = win_attrs.h();
    event.get().xconfigure.border_width = 0;
    event.get().xconfigure.above = None;
    event.get().xconfigure.override_redirect = True;

    event.send(StructureNotifyMask);
}

void
client_t::disown_child(client_ptr_t)
{

}

void
client_t::move(Pos new_pos)
{
    frame.move(new_pos);
    pos = new_pos;
    update_offset(this);
}

void
client_t::resize(Size new_size)
{
    if (new_size.h > BORDER_HEIGHT)
        win.resize({new_size.w, new_size.h - BORDER_HEIGHT});
    frame.resize(new_size);
    size = new_size;
    update_offset(this);
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
