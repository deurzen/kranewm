#include "client.hh"
#include "decoration.hh"
#include "x-wrapper/event.hh"
#include "x-wrapper/attributes.hh"
#include "x-wrapper/hints.hh"
#include "x-wrapper/input.hh"

client_ptr_t
create_client(x_wrapper::window_t win, rule_t& rule)
{
    auto sizehints = x_wrapper::get_sizehints(win);
    if (!sizehints.get().flags)
        sizehints.get().flags = PSize;

    dim_t base{}, inc{}, max{}, min{};
    range_t<float> aspect{};

    if (sizehints.get().flags & PBaseSize)
        base = {sizehints.get().base_width, sizehints.get().base_height};
    else if (sizehints.get().flags & PMinSize)
        base = {sizehints.get().min_width, sizehints.get().min_height};

    if (sizehints.get().flags & PResizeInc)
        inc = {sizehints.get().width_inc, sizehints.get().height_inc};

    if (sizehints.get().flags & PMaxSize)
        max = {sizehints.get().max_width, sizehints.get().max_height};

    if (sizehints.get().flags & PMinSize)
        min = {sizehints.get().min_width, sizehints.get().min_height};
    else if (sizehints.get().flags & PBaseSize)
        min = {sizehints.get().base_width, sizehints.get().base_height};

    if (sizehints.get().flags & PAspect)
        aspect = {static_cast<float>(sizehints.get().min_aspect.y) / sizehints.get().min_aspect.x,
            static_cast<float>(sizehints.get().max_aspect.y) / sizehints.get().max_aspect.x};

    sizeconstraints_t sizeconstraints(base, inc, max, min, aspect);

    auto win_attrs = x_wrapper::get_attributes(win);
    pos_t pos = win_attrs;
    dim_t dim = win_attrs;
    sizeconstraints.apply(pos, dim);

    x_wrapper::window_t frame = x_wrapper::create_window();
    x_wrapper::select_input(win, REG_WIN_SELECTION);
    x_wrapper::select_input(frame, REG_FRAME_SELECTION);

    frame.set_background_color(REG_COLOR);
    win.reparent({0, BORDER_HEIGHT}, frame);

    rule.fullscreen = win.is_of_state("FULLSCREEN");
    rule.floating |= rule.fullscreen
        || sizeconstraints.is_fixed()
        || win.is_of_type("DIALOG")
        || win.is_of_type("UTILITY")
        || win.is_of_type("SPLASH");

    client_ptr_t client = new client_t(win, frame, sizeconstraints, rule);
    client->resize({dim.w, dim.h + BORDER_HEIGHT});
    client->move({pos.x, pos.y - BORDER_HEIGHT});

    return client;
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

bool
client_t::redeem_expect(clientexpect occurred)
{
    if (occurred == expect) {
        switch (expect) {
        case MAP:      win.set_state(NormalState);
        case ICONIFY:  win.set_state(IconicState);
        case WITHDRAW: win.set_state(WithdrawnState);
        default: break;
        }

        expect = NO_EFFECT;
        return true;
    }

    return false;
}

client_t&
client_t::move(pos_t new_pos)
{
    frame.move(new_pos);
    pos = new_pos;
    update_offset(this);
    return *this;
}

client_t&
client_t::resize(dim_t new_dim)
{
    if (new_dim.h > BORDER_HEIGHT)
        win.resize({new_dim.w, new_dim.h - BORDER_HEIGHT});
    frame.resize(new_dim);
    dim = new_dim;
    update_offset(this);
    return *this;
}

client_t&
client_t::map()
{
    win.map();
    frame.map();
    return *this;
}

client_t&
client_t::unmap()
{
    frame.unmap();
    return *this;
}

client_t&
client_t::map_children()
{
    for (auto& child : children) {
        child->expect = MAP;
        child->map();
    }
    return *this;
}


client_t&
client_t::unmap_children()
{
    for (auto& child : children) {
        child->expect = WITHDRAW;
        child->unmap();
    }
    return *this;
}

client_t&
client_t::center()
{
    auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
    move({root_attrs.w() / 2 - dim.w / 2, root_attrs.h() / 2 - dim.h / 2});
    return *this;
}
